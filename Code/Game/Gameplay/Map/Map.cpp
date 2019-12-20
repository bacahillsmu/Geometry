#include "Game/Gameplay/Map/Map.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/Gameplay/GameHandle.hpp"
#include "Game/Gameplay/Entity.hpp"
#include "Game/Gameplay/EntityDefinition.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Engine/Math/Ray.hpp"
#include "Engine/Math/Plane3.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Gameplay/Input/PlayerController.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Game/Gameplay/Entity.hpp"
#include "Game/Gameplay/EntityUnits/Peon.hpp"
#include "Game/Gameplay/EntityUnits/Warrior.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimationDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/BitMapFont.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Prop.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/CRC32.hpp"
#include "Game/Gameplay/RTSCommand.hpp"
#include "Game/Gameplay/Input/ReplayController.hpp"

// -----------------------------------------------------------------------
Map::Map(Game* game)
{
	m_theGame = game;
}

// -----------------------------------------------------------------------
Map::~Map()
{
	delete m_terrainMesh;
	m_terrainMesh = nullptr;

	for(int i = 0; i < m_entities.size(); i++)
	{
		delete m_entities[i];
		m_entities[i] = nullptr;
	}
	m_entities.clear();

	for (auto it = m_spriteSheets.cbegin(); it != m_spriteSheets.cend() /* not hoisted */; /* no increment */)
	{
		m_spriteSheets.erase(it++);
	}
}

// -----------------------------------------------------------------------
bool Map::Load( const std::string& filename, int x, int y )
{
	LoadPlaySprites();

	LoadUnitsAndBuildingsFromXML();
	LoadResourcesFromXML();

	// We will eventually stop using the tileWidth and tileHeight here and just pull this information from filename
	if(filename == "play")
	{
		m_terrainMaterial = g_theRenderer->GetOrCreateMaterial("Data/Materials/grass_terrain.mat");
	}

	if(filename == "edit")
	{
		m_terrainMaterial = g_theRenderer->GetOrCreateMaterial("Data/Materials/stone_terrain.mat");
	}
	
	bool mapCreated = Create( x, y );

	

	return mapCreated;
}

// -----------------------------------------------------------------------
bool Map::Create( int tileWidth, int tileHeight )
{
	m_terrainMesh = new GPUMesh( g_theRenderer );

	m_tileDimensions = IntVec2( tileWidth, tileHeight );
	m_vertDimensions = IntVec2( 2 * tileWidth + 1, 2 * tileHeight + 1 );

	// Make tiles;
	//m_mapTiles.resize(tileWidth * tileHeight);
	m_mapSize = tileWidth * tileHeight;
	m_mapTileOccupancy.resize(m_mapSize);
	int index = 0;
	for(int y = 0; y < m_tileDimensions.y; y++)
	{
		for(int x = 0; x < m_tileDimensions.x; x++)
		{
			Vec2 coord = Vec2((float)x, (float)y);
			m_mapTileOccupancy[index] = -1;
			index++;
		}
	}

	// We generated our tiles, now make terrain based off the tiles;
	GenerateTerrainMesh();

	// Put a ring of trees around the map;
	for(int y = 0; y < m_tileDimensions.y; y++)
	{
		for(int x = 0; x < m_tileDimensions.x; x++)
		{
			if(x == 0 || x == m_tileDimensions.x - 1 || y == 0 || y == m_tileDimensions.y - 1)
			{
				IntVec2 tile = IntVec2(x, y);
				std::map< std::string, EntityDefinition* >::iterator tree = EntityDefinition::s_entityDefinitions.find("pinewhole");
				EntityDefinition* treeDefinition = tree->second;

				IntVec2 offset = treeDefinition->m_offset;
				tile.x -= (int)offset.x;
				tile.y -= (int)offset.y;

				int tileIndex = GetIndexFromCoord(tile, m_tileDimensions);
				int status = GetOccupiedTileIndexStatus(tileIndex);

				if(status == -1)
				{
					CreateCommand* createCommand = new CreateCommand();
					createCommand->m_commandFrameIssued = 15;
					createCommand->m_theGame = m_theGame;
					createCommand->m_team = 99;
					createCommand->m_commandType = CREATEPINETREE;
					createCommand->m_theMap = m_theGame->GetGameMap();
					createCommand->m_createPosition = Vec3(float(x), float(y), 0.0f);
					m_theGame->EnQueueCommand(createCommand);
					g_theReplayController->RecordCommand(createCommand);
				}
			}
		}
	}

	return true;
}

// -----------------------------------------------------------------------
void Map::Update(float deltaSeconds)
{
	UnoccupyAllTiles();
	ResetPatherCosts();

	for(Entity* entity: m_entities)
	{
		if(entity && entity->m_prop)
		{
			IntVec2 tile = GetTileCoordFromPosition(entity->m_position);
			IntVec2 offset = entity->m_entityDefinition->m_offset;
			tile.x -= (int)offset.x;
			tile.y -= (int)offset.y;
			
			for(int y = 0; y < entity->m_entityDefinition->m_occupancySize.y; y++)
			{
				for(int x = 0; x < entity->m_entityDefinition->m_occupancySize.x; x++)
				{
					IntVec2 occupiedTile = IntVec2(0, 0);
					occupiedTile.x = tile.x + x;
					occupiedTile.y = tile.y + y;
					int tileIndex = GetIndexFromCoord(occupiedTile, m_tileDimensions);

					SetOccupiedTileIndexStatus(tileIndex, 1);
					m_pather.SetCostTile(tileIndex, 10000.0f);
				}
			}
		}
	}

	CheckForOccupancyMapChanges();

	CalculateSupplyForTeam(m_theGame->GetCurrentTeam());
	
	
	// Update each Entity;
	for(Entity* entity: m_entities)
	{
		if(entity)
		{
			entity->ProcessTasks(deltaSeconds);
			if(entity->m_isValid)
			{
				entity->PushOutOfOtherEntities();
				entity->Update(deltaSeconds);
			}
				
		}
	}
	
	g_thePlayerController->UpdatePlaceHolderProp();
	CheckDelayEntity();
}

// -----------------------------------------------------------------------
void Map::Render()
{
	// Render the Terrain;
	RenderTerrain( m_terrainMaterial );	

	// Render each Entity;
	std::vector<Vertex_PCU> entityVertsPeon;
	std::vector<Vertex_PCU> entityVertsWarrior;
	std::vector<Vertex_PCU> entityVertsGoblin;
	std::vector<Vertex_PCU> entityUIVerts;
	std::vector<Vertex_PCU> entitySelectVerts;
	for(Entity* entity: m_entities)
	{
		if(entity && entity->m_isValid)
		{
			entity->Render(&entityVertsPeon, &entityVertsWarrior, &entityVertsGoblin, &entityUIVerts, &entitySelectVerts);
		}		
	}

	

	if(entitySelectVerts.size() > 0)
	{
		g_theRenderer->BindModelMatrix(Matrix4x4::IDENTITY);
		g_theRenderer->BindTextureView(0u, nullptr);
		g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
		g_theRenderer->DrawVertexArray((int)entitySelectVerts.size(), &entitySelectVerts[0]);
	}

	if(entityVertsPeon.size() > 0)
	{
		g_theRenderer->BindModelMatrix(Matrix4x4::IDENTITY);
		g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Laborer_spriteshee_2k.png"));
		g_theRenderer->BindShader("Data/Shaders/sprite.shader");
		g_theRenderer->DrawVertexArray((int)entityVertsPeon.size(), &entityVertsPeon[0]);
	}

	if(entityVertsWarrior.size() > 0)
	{
// 		if(m_currentAnimationState == ANIMATION_ATTACK)
// 		{
// 			g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Warrior_spritesheet_attack.png"));
// 		}
// 		else
// 		{
// 			g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Warrior_spritesheet_moveDeath.png"));
// 		}
		g_theRenderer->BindModelMatrix(Matrix4x4::IDENTITY);
		g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Warrior_spritesheet_moveDeath.png"));
		g_theRenderer->BindShader("Data/Shaders/sprite.shader");
		g_theRenderer->DrawVertexArray((int)entityVertsWarrior.size(), &entityVertsWarrior[0]);
	}

	if(entityVertsGoblin.size() > 0)
	{
		// 		if(m_currentAnimationState == ANIMATION_ATTACK)
		// 		{
		// 			g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/goblin.attack.png"));
		// 		}
		// 		else
		// 		{
		// 			g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/goblin.walkdeath.png"));
		// 		}
		g_theRenderer->BindModelMatrix(Matrix4x4::IDENTITY);
		g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/goblin.walkdeath.png"));
		g_theRenderer->BindShader("Data/Shaders/sprite.shader");
		g_theRenderer->DrawVertexArray((int)entityVertsGoblin.size(), &entityVertsGoblin[0]);
	}

	for(Entity* entity: m_entities)
	{
		if(entity && entity->m_isValid)
		{
			entity->RenderProp();
		}		
	}
		
	if(entityUIVerts.size() > 0)
	{
		g_theRenderer->BindModelMatrix(Matrix4x4::IDENTITY);
		g_theRenderer->BindTextureView(0u, nullptr);
		g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
		g_theRenderer->DrawVertexArray((int)entityUIVerts.size(), &entityUIVerts[0]);
	}
	

	g_thePlayerController->RenderPlaceHolderProp(); 

	
}

// -----------------------------------------------------------------------
void Map::RenderScreenSelectedUnits(AABB2 screenDimensions, std::vector<Vertex_PCU>* textVerts)
{
	float positionRatio = 0.95f;
	BitMapFont* bitmapFont = g_theRenderer->CreateOrGetBitmapFont("SquirrelFixedFont");
	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");

	
	Vec2 printPosition = Vec2(screenDimensions.maxs.x * 0.05f, screenDimensions.maxs.y * positionRatio);
	float height = 12.0f;
	float width = 6.0f * height;
	printPosition.y -= height * 0.5f;
	printPosition.x -= width * 0.5f;

	bitmapFont->AddVertsForText2D(*textVerts, printPosition, height, "Current Team: %d\n", Rgba::YELLOW, m_theGame->GetCurrentTeam());
	positionRatio -= 0.02f;

	for(Entity* entity: m_entities)
	{
		if(entity && entity->m_isSelected)
		{
			// Show Name;
			std::string name = entity->m_entityDefinition->m_entityType;
			float health = entity->m_health;			

			printPosition = Vec2(screenDimensions.maxs.x * 0.05f, screenDimensions.maxs.y * positionRatio);
			printPosition.y -= height * 0.5f;
			printPosition.x -= width * 0.5f;

			bitmapFont->AddVertsForText2D(*textVerts, printPosition, height, "%s", Rgba::GREEN, name.c_str());
			printPosition.x += width * 1.4f;

			// Show Health;
			Rgba healthColor = Rgba::GREEN;
			if(health <= entity->m_entityDefinition->m_health * 0.8f && health > entity->m_entityDefinition->m_health * 0.3f)
			{
				healthColor = Rgba::YELLOW;
			}
			else if(health <= entity->m_entityDefinition->m_health * 0.3f)
			{
				healthColor = Rgba::RED;
			}

			bitmapFont->AddVertsForText2D(*textVerts, printPosition, height, "Health [%f]", healthColor, health);

			positionRatio -= 0.02f;

		}
	}
}

void Map::RenderScreenSelectedTasks( AABB2 screenDimensions, std::vector<Vertex_PCU>* textVerts )
{
	float positionRatio = 0.2f;
	BitMapFont* bitmapFont = g_theRenderer->CreateOrGetBitmapFont("SquirrelFixedFont");
	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");

	Vec2 printPosition = Vec2(screenDimensions.maxs.x * 0.8f, screenDimensions.maxs.y * positionRatio);
	float height = 12.0f;
	float width = 6.0f * height;
	printPosition.y -= height * 0.5f;
	printPosition.x -= width * 0.5f;

	Entity* selectedEntity = GetFirstSelectedEntity();
	if(selectedEntity)
	{
		std::string name = selectedEntity->m_entityDefinition->m_entityType;
		bitmapFont->AddVertsForText2D(*textVerts, printPosition, height, "[%s] Current Tasks:\n", Rgba::WHITE, name.c_str());
		positionRatio -= 0.02f;

		std::map<std::string, RTSTaskInfo*>::iterator taskIterator;
		for(taskIterator = selectedEntity->m_entityDefinition->m_availableTasks.begin(); taskIterator != selectedEntity->m_entityDefinition->m_availableTasks.end(); taskIterator++)
		{
			RTSTaskInfo* rtsTaskInfo = taskIterator->second;
			std::string label = rtsTaskInfo->hotkeyLabel;

			printPosition = Vec2(screenDimensions.maxs.x * 0.8f, screenDimensions.maxs.y * positionRatio);
			printPosition.y -= height * 0.5f;
			printPosition.x -= width * 0.5f;

			bitmapFont->AddVertsForText2D(*textVerts, printPosition, height, "%s", Rgba::YELLOW, label.c_str());
			printPosition.x += width * 1.4f;

			positionRatio -= 0.02f;
		}
	}
}

// -----------------------------------------------------------------------
void Map::RenderScreenResourceInfo( AABB2 screenDimensions, std::vector<Vertex_PCU>* textVerts )
{
	int team = m_theGame->GetCurrentTeam();
	float positionRatio = 0.95f;
	BitMapFont* bitmapFont = g_theRenderer->CreateOrGetBitmapFont("SquirrelFixedFont");
	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");

	float height = 12.0f;
	float width = 6.0f * height;

	int currentMineral = 0;
	int currentWood = 0;
	int currentSupply = 0;
	int currentMaxSupply = 0;

	if(team == 0)
	{
		currentMineral = m_currentMineralTeam0;
		currentWood = m_currentWoodTeam0;
		currentSupply = m_currentSupplyTeam0;
		currentMaxSupply = m_maxSupplyTeam0;
	}
	else if(team == 1)
	{
		currentMineral = m_currentMineralTeam1;
		currentWood = m_currentWoodTeam1;
		currentSupply = m_currentSupplyTeam1;
		currentMaxSupply = m_maxSupplyTeam1;
	}

	Vec2 printPosition = Vec2(screenDimensions.maxs.x * 0.9f, screenDimensions.maxs.y * positionRatio);
	printPosition.y -= height * 0.5f;
	printPosition.x -= width * 0.5f;
	bitmapFont->AddVertsForText2D(*textVerts, printPosition, height, "Minerals: %d", Rgba::WHITE, currentMineral);
	
	printPosition = Vec2(screenDimensions.maxs.x * 0.8f, screenDimensions.maxs.y * positionRatio);
	printPosition.y -= height * 0.5f;
	printPosition.x -= width * 0.5f;
	bitmapFont->AddVertsForText2D(*textVerts, printPosition, height, "Wood: %d", Rgba::WHITE, currentWood);

	printPosition = Vec2(screenDimensions.maxs.x * 0.7f, screenDimensions.maxs.y * positionRatio);
	printPosition.y -= height * 0.5f;
	printPosition.x -= width * 0.5f;
	bitmapFont->AddVertsForText2D(*textVerts, printPosition, height, "Supply: %d/%d", Rgba::WHITE, currentSupply, currentMaxSupply);

	printPosition = Vec2(screenDimensions.maxs.x * 0.5f, screenDimensions.maxs.y * positionRatio);
	printPosition.y -= height * 0.5f;
	printPosition.x -= width * 0.5f;
	bitmapFont->AddVertsForText2D(*textVerts, printPosition, height, "MeFrame: %d", Rgba::WHITE, m_theGame->m_myCommandFrameComplete);

	printPosition = Vec2(screenDimensions.maxs.x * 0.3f, screenDimensions.maxs.y * positionRatio);
	printPosition.y -= height * 0.5f;
	printPosition.x -= width * 0.5f;
	bitmapFont->AddVertsForText2D(*textVerts, printPosition, height, "ThemFrame: %d", Rgba::WHITE, m_theGame->m_themCommandFrameComplete);
}

// -----------------------------------------------------------------------
void Map::RenderTerrain( Material* material /*= nullptr */ )
{
	m_terrainModel.SetT( Vec3( 0.0f, 0.0f, 0.0f ) );	
	g_theRenderer->BindModelMatrix( m_terrainModel );
	g_theRenderer->BindMaterial( material );
	g_theRenderer->DrawMesh( m_terrainMesh );

	g_theRenderer->BindShader(g_theRenderer->GetOrCreateShader( "Data/Shaders/default_unlit_devconsole.shader" ));
	g_theRenderer->BindTextureViewWithSampler(0u, nullptr);
	std::vector<Vertex_PCU> quad;

	for(int tileIndex = 0; tileIndex < m_mapSize; tileIndex++)
	{
		int occupyStatus = m_mapTileOccupancy[tileIndex];
		// Unoccupied
		if(occupyStatus == 0)
		{
			Vec2 coord = Vec2(GetCoordFromIndex(tileIndex, m_tileDimensions));
			
			AABB2 tile = AABB2::MakeFromMinsMaxs( Vec2( coord.x - 0.48f, coord.y - 0.48f ), Vec2( coord.x + 0.48f, coord.y + 0.48f ) );
			AddVertsForAABB2D(quad, tile, Rgba(0.0f, 1.0f, 0.0f, 0.5f));
		}
		// Trying to place a building but something is in the way;
		else if(occupyStatus == 2)
		{
			Vec2 coord = Vec2(GetCoordFromIndex(tileIndex, m_tileDimensions));

			AABB2 tile = AABB2::MakeFromMinsMaxs( Vec2( coord.x - 0.48f, coord.y - 0.48f ), Vec2( coord.x + 0.48f, coord.y + 0.48f ) );
			AddVertsForAABB2D(quad, tile, Rgba(1.0f, 0.0f, 0.0f, 0.5f));
		}
		// Occupied by a prop;
// 		else if(occupyStatus == 1)
// 		{
// 			Vec2 coord = Vec2(GetCoordFromIndex(tileIndex, m_tileDimensions));
// 
// 			AABB2 tile = AABB2::MakeFromMinsMaxs( Vec2( coord.x - 0.48f, coord.y - 0.48f ), Vec2( coord.x + 0.48f, coord.y + 0.48f ) );
// 			AddVertsForAABB2D(quad, tile, Rgba(1.0f, 0.0f, 0.0f, 0.5f));
// 		}
	}
	if(g_thePlayerController->m_selectedEntities.size() == 1)
	{
		Entity* entity = FindEntity(g_thePlayerController->m_selectedEntities.front());
		if(entity && entity->m_pathCreation)
		{
			Path path = *entity->m_pathCreation->m_path;
			while(path.size() > 0)
			{
				IntVec2 icoord = path.back();
				Vec2 coord = Vec2((float)icoord.x, (float)icoord.y);
				AABB2 tile = AABB2::MakeFromMinsMaxs( Vec2( coord.x - 0.48f, coord.y - 0.48f ), Vec2( coord.x + 0.48f, coord.y + 0.48f ) );
				AddVertsForAABB2D(quad, tile, Rgba(1.0f, 0.0f, 0.0f, 0.5f));
				path.pop_back();
			}
			Vec2 coord = Vec2(entity->m_debug_tile.x, entity->m_debug_tile.y);
			AABB2 tile = AABB2::MakeFromMinsMaxs( Vec2( coord.x - 0.48f, coord.y - 0.48f ), Vec2( coord.x + 0.48f, coord.y + 0.48f ) );
			AddVertsForAABB2D(quad, tile, Rgba(0.0f, 0.0f, 1.0f, 0.5f));
		}
	}

	if(quad.size() > 0)
	{
		for(auto& q: quad)
		{
			q.position.z -= 0.0f;
		}
		g_theRenderer->DrawVertexArray( quad );
	}
}

// -----------------------------------------------------------------------
void Map::EndFrame()
{
	CleanupEntities();
}

// -----------------------------------------------------------------------
void Map::CleanupEntities()
{
	for(int entityIndex = 0; entityIndex < m_entities.size(); entityIndex++)
	{
		Entity* entity = m_entities[entityIndex];
		if(entity && !entity->m_isValid)
		{
			delete entity;
			entity = nullptr;
			m_entities[entityIndex] = nullptr;
		}
	}
}

// -----------------------------------------------------------------------
void Map::LoadPlaySprites()
{
	TextureView* spriteTexture = g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Laborer_spriteshee_2k.png");
	SpriteSheet* spriteSheet = new SpriteSheet(spriteTexture, IntVec2(16, 16));
	m_spriteSheets["Peon"] = spriteSheet;

	spriteTexture = g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Warrior_spritesheet_attack.png");
	spriteSheet = new SpriteSheet(spriteTexture, IntVec2(8, 8));
	m_spriteSheets["WarriorAttack"] = spriteSheet;

	spriteTexture = g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Warrior_spritesheet_moveDeath.png");
	spriteSheet = new SpriteSheet(spriteTexture, IntVec2(8, 8));
	m_spriteSheets["WarriorMoveDeath"] = spriteSheet;

	spriteTexture = g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/goblin.attack.png");
	spriteSheet = new SpriteSheet(spriteTexture, IntVec2(8, 8));
	m_spriteSheets["GoblinAttack"] = spriteSheet;

	spriteTexture = g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/goblin.walkdeath.png");
	spriteSheet = new SpriteSheet(spriteTexture, IntVec2(8, 8));
	m_spriteSheets["GoblinMoveDeath"] = spriteSheet;
}

// -----------------------------------------------------------------------
void Map::LoadUnitsAndBuildingsFromXML()
{
	EntityDefinition::LoadUnitsFromXML("Data/XML/Humans.xml");
}

void Map::LoadResourcesFromXML()
{
	EntityDefinition::LoadResourcesFromXML("Data/XML/Resources.xml");
}

// -----------------------------------------------------------------------
AABB2 Map::GetXYBounds() const
{
	float minX = -0.5f;
	float maxX = m_tileDimensions.x - 0.5f;
	float minY = -0.5f;
	float maxY = m_tileDimensions.y - 0.5f;

	return AABB2::MakeFromMinsMaxs( Vec2( minX, minY ), Vec2( maxX, maxY ) );
}

// -----------------------------------------------------------------------
void Map::SetOccupiedTileIndexStatus( int tileIndex, int status )
{
	m_mapTileOccupancy[tileIndex] = status;
}

// -----------------------------------------------------------------------
int Map::GetOccupiedTileIndexStatus( int tileIndex )
{
	return m_mapTileOccupancy[tileIndex];
}

// -----------------------------------------------------------------------
void Map::UnoccupyAllTiles()
{
	m_mapTileOccupancyCopy = m_mapTileOccupancy;

	for(int tileIndex = 0; tileIndex < m_mapTileOccupancy.size(); tileIndex++)
	{
		m_mapTileOccupancy[tileIndex] = -1;
	}
}

// -----------------------------------------------------------------------
void Map::CheckForOccupancyMapChanges()
{
	for(int x = 0; x < m_mapTileOccupancy.size(); x++)
	{
		if(m_mapTileOccupancy[x] != m_mapTileOccupancyCopy[x])
		{
			m_occupancyMapChangeTimeStamp = GetCurrentTimeSeconds();
			return;
		}
	}
}

// -----------------------------------------------------------------------
void Map::ResetPatherCosts()
{
	m_pather.Init(m_tileDimensions, 1.0f);
}

// -----------------------------------------------------------------------
IntVec2 Map::GetTileCoordFromPosition( Vec3 position )
{
	if(position.x < 0.0f)
	{
		position.x = 0.0f;
	}
	if(position.y < 0.0f)
	{
		position.y = 0.0f;
	}

	// x
	float x = position.x;
	float wholex, fractionalx;
	fractionalx = std::modf(x, &wholex);

	if(fractionalx < 0.5f)
	{
		position.x = floor(position.x);
	}
	else
	{
		position.x = ceil(position.x);
	}

	// y
	float y = position.y;
	float wholey, fractionaly;
	fractionaly = std::modf(y, &wholey);

	if(fractionaly < 0.5f)
	{
		position.y = floor(position.y);
	}
	else
	{
		position.y = ceil(position.y);
	}
	

	return IntVec2((int)position.x, (int)position.y);
}

// -----------------------------------------------------------------------
bool Map::IsTileOccupied( IntVec2 tile )
{
	int tileIndex = GetIndexFromCoord(tile, m_tileDimensions);
	int occupied = m_mapTileOccupancy[tileIndex]; // 1 means occupied;
	if(occupied == 1)
	{
		return true;
	}

	return false;
}

// -----------------------------------------------------------------------
// Entity
// -----------------------------------------------------------------------
Entity* Map::FindEntity( GameHandle handle )
{
	if(handle == GameHandle::INVALID)
	{
		return nullptr;
	}
	
	uint slot = handle.GetIndexSlot();
	Entity* entity = m_entities[slot];

	if((entity) && entity->GetGameHandle() == handle)
	{
		return entity;
	}

	return nullptr;
}

// -----------------------------------------------------------------------
std::vector<Entity*> Map::GetAllValidEntities()
{
	std::vector<Entity*> validEntities;
	for(auto& entity: m_entities)
	{
		if(entity)
		{
			if(!entity->m_isDead && entity->m_isValid)
			{
				validEntities.push_back(entity);
			}
		}
	}

	return validEntities;
}

// -----------------------------------------------------------------------
std::vector<Entity*> Map::GetAllValidBuildingEntities()
{
	std::vector<Entity*> validEntities;
	for(auto& entity: m_entities)
	{
		if(entity)
		{
			if(!entity->m_isDead && entity->m_isValid && !entity->m_isResource && entity->m_prop)
			{
				validEntities.push_back(entity);
			}
		}
	}

	return validEntities;
}

// -----------------------------------------------------------------------
std::vector<Entity*> Map::GetAllValidUnitsEntities()
{
	std::vector<Entity*> validEntities;
	for(auto& entity: m_entities)
	{
		if(entity)
		{
			if(!entity->m_isDead && entity->m_isValid && !entity->m_isResource && !entity->m_prop)
			{
				validEntities.push_back(entity);
			}
		}
	}

	return validEntities;
}

// -----------------------------------------------------------------------
Entity* Map::GetFirstValidEntity()
{
	std::vector<Entity*> validEntities;
	for(auto& entity: m_entities)
	{
		if(entity)
		{
			if(!entity->m_isDead && entity->m_isValid)
			{
				validEntities.push_back(entity);
			}
		}
	}
	
	return validEntities.front();
}

// -----------------------------------------------------------------------
void Map::AddEntity( Entity* entity )
{
	int slot = entity->GetGameHandle().GetIndexSlot();
	if(slot >= m_entities.size())
	{
		m_entities.resize(slot + 1);
	}
	m_entities[slot] = entity;
}

// -----------------------------------------------------------------------
void Map::DelayAddEntity( Entity* entity )
{
	m_delayEntities.push_back(entity);
}

// -----------------------------------------------------------------------
void Map::CheckDelayEntity()
{
	if(m_delayEntities.size() > 0)
	{
		for(Entity* entity: m_delayEntities)
		{
			if(entity)
			{
				int slot = entity->GetGameHandle().GetIndexSlot();

				if(slot >= m_entities.size())
				{
					m_entities.resize(slot + 1);
				}

				m_entities[slot] = entity;
			}
		}
	}
	
	m_delayEntities.clear();
}

// -----------------------------------------------------------------------
void Map::UnSelectAllEntities()
{
	for(Entity* entity: m_entities)
	{
		if(entity)
		{
			entity->SetSelected(false);
		}		
	}
}

// -----------------------------------------------------------------------
Entity* Map::GetFirstSelectedEntity()
{
	for(Entity* entity: m_entities)
	{
		if(entity && entity->m_isSelected)
		{
			return entity;
		}		
	}

	return nullptr;
}

bool Map::CanAffordEntity( EntityDefinition* entityDefinition )
{
	int woodCost = entityDefinition->m_woodCost;
	int mineralCost = entityDefinition->m_mineralCost;
	int team = m_theGame->GetCurrentTeam();

	if(team == 0)
	{
		if(woodCost > m_currentWoodTeam0 || mineralCost > m_currentMineralTeam0)
		{
			return false;
		}
	}
	else if(team == 1)
	{
		if(woodCost > m_currentWoodTeam1 || mineralCost > m_currentMineralTeam1)
		{
			return false;
		}
	}

	return true;
}

// -----------------------------------------------------------------------
void Map::PayCostEntity( EntityDefinition* entityDefinition )
{
	int woodCost = entityDefinition->m_woodCost;
	int mineralCost = entityDefinition->m_mineralCost;
	int team = m_theGame->GetCurrentTeam();

	if(team == 0)
	{
		m_currentWoodTeam0 -= woodCost;
		m_currentMineralTeam0 -= mineralCost;
	}
	else if(team == 1)
	{
		m_currentWoodTeam1 -= woodCost;
		m_currentMineralTeam1 -= mineralCost;
	}
}

// -----------------------------------------------------------------------
std::vector<Entity*> Map::GetGoblinHutsOnTeam( int team )
{
	std::vector<Entity*> goblinHuts;
	std::vector<Entity*> validEntities;

	validEntities = GetAllValidBuildingEntities();
	for(auto& entity: validEntities)
	{
		if(entity->m_entityDefinition->m_entityType == "GoblinHut" && entity->m_team == team)
		{
			goblinHuts.push_back(entity);
		}
	}

	return goblinHuts;
}

// -----------------------------------------------------------------------
std::vector<Entity*> Map::GetEntitiesOfTypeOnTeam( std::string& type, int team )
{
	std::vector<Entity*> entityTypes;
	std::vector<Entity*> validEntities;

	validEntities = GetAllValidUnitsEntities();
	for(auto& entity: validEntities)
	{
		if(entity->m_entityDefinition->m_entityType == type && entity->m_team == team)
		{
			entityTypes.push_back(entity);
		}
	}

	return entityTypes;
}

// -----------------------------------------------------------------------
void Map::UnhoverAllEntities()
{
	for(Entity* entity: m_entities)
	{
		if(entity)
		{
			entity->SetHovered(false);
		}		
	}
}

// -----------------------------------------------------------------------
// Pick
// -----------------------------------------------------------------------
Entity* Map::RaycastEntity( float* hitTime, Ray3* ray, float maxDistance /*= INFINITY*/ )
{
	Entity* bestEntity = nullptr;
	float bestTime = INFINITY;	

	for(Entity* entity: m_entities)
	{
		if(entity)
		{
			float time[2];
			if(entity->IsSelectable() && entity->Raycast(time, ray))
			{
				float smaller = time[0] > time[1] ? time[1] : time[0];

				if(smaller >= 0.0f && smaller <= maxDistance && smaller < bestTime)
				{
					bestEntity = entity;
					bestTime = smaller;
				}
			}
		}
	}

	*hitTime = bestTime;
	return bestEntity;
}

// -----------------------------------------------------------------------
bool Map::RaycastTerrain( float* hitTime, Ray3* ray, float maxDistance /*= INFINITY*/ )
{
	UNUSED(maxDistance);

	bool terrainHit = false;

	float time = 0.0f;
	Plane3 terrain = Plane3::AtPosition(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));
	
	terrainHit = Raycast(&time, ray, terrain);	

	if(!terrainHit)
	{
		return false;
	}

	*hitTime = time;
	Vec3 spotHit = ray->PointAtTime(time);

	// Inside dimensions;
	if(spotHit.x >= -0.5f && spotHit.y >= -0.5f && spotHit.x <= m_tileDimensions.x - 0.5f && spotHit.y <= m_tileDimensions.y - 0.5f)
	{
		return true;
	}
	// Outside dimensions;
	else
	{
		return false;
	}	
}

// -----------------------------------------------------------------------
void Map::PurgeDestroyedEntities()
{
	for(Entity* entity: m_entities)
	{
		if(entity)
		{
			if(entity->IsDestroyed())
			{
				delete entity;
				entity = nullptr;
			}
		}
	}
}

uint16_t Map::GetFreeEntityIndex()
{
	uint16_t i = 0;
	for(Entity* entity: m_entities)
	{
		if(!entity)
		{
			return i;
		}

		i++;
	}

	//m_entities.push_back(nullptr);
	return i;
}

uint16_t Map::GetNextCyclicId()
{
	if(m_cyclicId == 0)
	{
		m_cyclicId++;
	}

	return m_cyclicId++;
}

void Map::LoadPeonAnimations(Entity* entity, std::string& entityType)
{
	UNUSED(entityType);

	std::map<std::string, SpriteSheet*>::iterator mapPair = m_spriteSheets.find("Peon");
	SpriteSheet* spriteSheet = mapPair->second;

	float speed = 1.0f / entity->m_speed;
	float attackSpeed = 1.0f / entity->m_attackSpeed;

	// Idle;
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_SOUTH]		=  new SpriteAnimationDefinition(*spriteSheet, 8, 8, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_SOUTHWEST] =  new SpriteAnimationDefinition(*spriteSheet, 24, 24, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_WEST]		=  new SpriteAnimationDefinition(*spriteSheet, 40, 40, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_NORTHWEST] =  new SpriteAnimationDefinition(*spriteSheet, 56, 56, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_NORTH]		=  new SpriteAnimationDefinition(*spriteSheet, 72, 72, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_NORTHEAST] =  new SpriteAnimationDefinition(*spriteSheet, 88, 88, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_EAST]		=  new SpriteAnimationDefinition(*spriteSheet, 104, 104, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_SOUTHEAST] =  new SpriteAnimationDefinition(*spriteSheet, 120, 120, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);

	// Walking;
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_SOUTH]		= new SpriteAnimationDefinition(*spriteSheet, 0, 6, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_SOUTHWEST] = new SpriteAnimationDefinition(*spriteSheet, 16, 22, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_WEST]		= new SpriteAnimationDefinition(*spriteSheet, 32, 38, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_NORTHWEST] = new SpriteAnimationDefinition(*spriteSheet, 48, 54, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_NORTH]		= new SpriteAnimationDefinition(*spriteSheet, 64, 70, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_NORTHEAST] = new SpriteAnimationDefinition(*spriteSheet, 80, 86, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_EAST]		= new SpriteAnimationDefinition(*spriteSheet, 96, 102, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_SOUTHEAST] = new SpriteAnimationDefinition(*spriteSheet, 112, 118, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);

	// Attacking;
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_SOUTH]		= new SpriteAnimationDefinition(*spriteSheet, 8, 14, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_SOUTHWEST]	= new SpriteAnimationDefinition(*spriteSheet, 24, 30, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_WEST]		= new SpriteAnimationDefinition(*spriteSheet, 40, 46, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_NORTHWEST]	= new SpriteAnimationDefinition(*spriteSheet, 56, 62, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_NORTH]		= new SpriteAnimationDefinition(*spriteSheet, 72, 78, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_NORTHEAST]	= new SpriteAnimationDefinition(*spriteSheet, 88, 94, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_EAST]		= new SpriteAnimationDefinition(*spriteSheet, 104, 110, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_SOUTHEAST]	= new SpriteAnimationDefinition(*spriteSheet, 120, 126, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);

	// Die;
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_SOUTH]		= new SpriteAnimationDefinition(*spriteSheet, 128, 134, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_SOUTHWEST]	= new SpriteAnimationDefinition(*spriteSheet, 144, 150, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_WEST]		= new SpriteAnimationDefinition(*spriteSheet, 160, 166, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_NORTHWEST]	= new SpriteAnimationDefinition(*spriteSheet, 176, 182, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_NORTH]		= new SpriteAnimationDefinition(*spriteSheet, 192, 198, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_NORTHEAST]	= new SpriteAnimationDefinition(*spriteSheet, 208, 214, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_EAST]		= new SpriteAnimationDefinition(*spriteSheet, 224, 230, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_SOUTHEAST]	= new SpriteAnimationDefinition(*spriteSheet, 240, 246, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);

}

void Map::LoadWarriorAnimations( Entity* entity, std::string& entityType )
{
	UNUSED(entityType);

	std::map<std::string, SpriteSheet*>::iterator mapPairMoveDeath = m_spriteSheets.find("WarriorMoveDeath");
	SpriteSheet* spriteSheetMoveDeath = mapPairMoveDeath->second;

	float speed = 1.0f / entity->m_speed;
	float attackSpeed = 1.0f / entity->m_attackSpeed;

	// Idle;
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_SOUTH]		=  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 0, 0, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_SOUTHWEST] =  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 8, 8, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_WEST]		=  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 16, 16, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_NORTHWEST] =  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 24, 24, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_NORTH]		=  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 32, 32, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_NORTHEAST] =  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 40, 40, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_EAST]		=  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 48, 48, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_SOUTHEAST] =  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 56, 56, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);

	// Walking;
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_SOUTH]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 0, 5, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_SOUTHWEST] = new SpriteAnimationDefinition(*spriteSheetMoveDeath, 8, 13, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_WEST]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 16, 21, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_NORTHWEST] = new SpriteAnimationDefinition(*spriteSheetMoveDeath, 24, 29, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_NORTH]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 32, 37, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_NORTHEAST] = new SpriteAnimationDefinition(*spriteSheetMoveDeath, 40, 45, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_EAST]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 48, 53, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_SOUTHEAST] = new SpriteAnimationDefinition(*spriteSheetMoveDeath, 56, 61, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);

	
	// Die;
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_SOUTH]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 6, 7, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_SOUTHWEST]	= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 14, 15, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_WEST]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 22, 23, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_NORTHWEST]	= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 30, 31, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_NORTH]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 38, 39, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_NORTHEAST]	= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 46, 47, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_EAST]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 54, 55, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_SOUTHEAST]	= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 62, 63, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);

	std::map<std::string, SpriteSheet*>::iterator mapPairAttack = m_spriteSheets.find("WarriorAttack");
	SpriteSheet* spriteSheetAttack = mapPairAttack->second;

	// Attacking;
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_SOUTH]		= new SpriteAnimationDefinition(*spriteSheetAttack, 0, 5, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_SOUTHWEST]	= new SpriteAnimationDefinition(*spriteSheetAttack, 8, 13, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_WEST]		= new SpriteAnimationDefinition(*spriteSheetAttack, 16, 21, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_NORTHWEST]	= new SpriteAnimationDefinition(*spriteSheetAttack, 24, 29, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_NORTH]		= new SpriteAnimationDefinition(*spriteSheetAttack, 32, 37, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_NORTHEAST]	= new SpriteAnimationDefinition(*spriteSheetAttack, 40, 45, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_EAST]		= new SpriteAnimationDefinition(*spriteSheetAttack, 48, 53, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_SOUTHEAST]	= new SpriteAnimationDefinition(*spriteSheetAttack, 56, 61, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);

}

void Map::LoadGoblinAnimations( Entity* entity, std::string& entityType )
{
	UNUSED(entityType);

	std::map<std::string, SpriteSheet*>::iterator mapPairMoveDeath = m_spriteSheets.find("GoblinMoveDeath");
	SpriteSheet* spriteSheetMoveDeath = mapPairMoveDeath->second;

	float speed = 1.0f / entity->m_speed;
	float attackSpeed = 1.0f / entity->m_attackSpeed;

	// Idle;
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_SOUTH]		=  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 0, 0, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_SOUTHWEST] =  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 8, 8, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_WEST]		=  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 16, 16, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_NORTHWEST] =  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 24, 24, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_NORTH]		=  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 32, 32, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_NORTHEAST] =  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 40, 40, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_EAST]		=  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 48, 48, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_IDLE][DIRECTION_SOUTHEAST] =  new SpriteAnimationDefinition(*spriteSheetMoveDeath, 56, 56, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);

	// Walking;
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_SOUTH]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 0, 5, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_SOUTHWEST] = new SpriteAnimationDefinition(*spriteSheetMoveDeath, 8, 13, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_WEST]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 16, 21, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_NORTHWEST] = new SpriteAnimationDefinition(*spriteSheetMoveDeath, 24, 29, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_NORTH]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 32, 37, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_NORTHEAST] = new SpriteAnimationDefinition(*spriteSheetMoveDeath, 40, 45, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_EAST]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 48, 53, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_WALK][DIRECTION_SOUTHEAST] = new SpriteAnimationDefinition(*spriteSheetMoveDeath, 56, 61, speed, SPRITE_ANIMATION_PLAYBACK_LOOP);


	// Die;
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_SOUTH]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 6, 7, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_SOUTHWEST]	= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 14, 15, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_WEST]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 22, 23, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_NORTHWEST]	= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 30, 31, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_NORTH]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 38, 39, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_NORTHEAST]	= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 46, 47, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_EAST]		= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 54, 55, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);
	entity->m_animationSet[ANIMATION_DIE][DIRECTION_SOUTHEAST]	= new SpriteAnimationDefinition(*spriteSheetMoveDeath, 62, 63, speed, SPRITE_ANIMATION_PLAYBACK_ONCE);

	std::map<std::string, SpriteSheet*>::iterator mapPairAttack = m_spriteSheets.find("GoblinAttack");
	SpriteSheet* spriteSheetAttack = mapPairAttack->second;

	// Attacking;
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_SOUTH]		= new SpriteAnimationDefinition(*spriteSheetAttack, 0, 5, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_SOUTHWEST]	= new SpriteAnimationDefinition(*spriteSheetAttack, 8, 13, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_WEST]		= new SpriteAnimationDefinition(*spriteSheetAttack, 16, 21, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_NORTHWEST]	= new SpriteAnimationDefinition(*spriteSheetAttack, 24, 29, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_NORTH]		= new SpriteAnimationDefinition(*spriteSheetAttack, 32, 37, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_NORTHEAST]	= new SpriteAnimationDefinition(*spriteSheetAttack, 40, 45, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_EAST]		= new SpriteAnimationDefinition(*spriteSheetAttack, 48, 53, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);
	entity->m_animationSet[ANIMATION_ATTACK][DIRECTION_SOUTHEAST]	= new SpriteAnimationDefinition(*spriteSheetAttack, 56, 61, attackSpeed, SPRITE_ANIMATION_PLAYBACK_LOOP);

}



// -----------------------------------------------------------------------
// Map Resource
// -----------------------------------------------------------------------
void Map::CalculateSupplyForTeam(int team)
{
	if(team == 0)
	{
		m_maxSupplyTeam0 = 0;
		m_currentSupplyTeam0 = 0;

		for(auto& entity: m_entities)
		{
			if(entity && entity->m_team == team)
			{
				EntityDefinition* entityDefinition = entity->m_entityDefinition;
				if(entityDefinition->m_providesSupply)
				{
					m_maxSupplyTeam0 += (int)entityDefinition->m_supply;
					m_maxSupplyTeam0 = Clamp(m_maxSupplyTeam0, 0, 200);
				}

				if(!entityDefinition->m_providesSupply)
				{
					m_currentSupplyTeam0 += (int)entityDefinition->m_supply;
				}
			}
		}
	}
	else if(team == 1)
	{
		m_maxSupplyTeam1 = 0;
		m_currentSupplyTeam1 = 0;

		for(auto& entity: m_entities)
		{
			if(entity && entity->m_team == team)
			{
				EntityDefinition* entityDefinition = entity->m_entityDefinition;
				if(entityDefinition->m_providesSupply)
				{
					m_maxSupplyTeam1 += (int)entityDefinition->m_supply;
					m_maxSupplyTeam1 = Clamp(m_maxSupplyTeam1, 0, 200);
				}

				if(!entityDefinition->m_providesSupply)
				{
					m_currentSupplyTeam1 += (int)entityDefinition->m_supply;
				}
			}
		}
	}
	else
	{
		ERROR_AND_DIE("Asked to calculate Supply for Unknown Team.");
	}
}

// -----------------------------------------------------------------------
void Map::AddWoodToTeam( int woodAmount, int team )
{
	if(team == 0)
	{
		m_currentWoodTeam0 += woodAmount;
	}
	else if(team == 1)
	{
		m_currentWoodTeam1 += woodAmount;
	}
	else
	{
		ERROR_AND_DIE("Incorrect Team asked for Wood Add.");
	}
}

// -----------------------------------------------------------------------
void Map::AddMineralToTeam( int mineralAmount, int team )
{
	if(team == 0)
	{
		m_currentMineralTeam0 += mineralAmount;
	}
	else if(team == 1)
	{
		m_currentMineralTeam1 += mineralAmount;
	}
	else
	{
		ERROR_AND_DIE("Incorrect Team asked for Mineral Add.");
	}
}

// -----------------------------------------------------------------------
void Map::AddSupplyToTeam( int supplyAmount, int team )
{
	if(team == 0)
	{
		m_maxSupplyTeam0 += supplyAmount;
	}
	else if(team == 1)
	{
		m_maxSupplyTeam1 += supplyAmount;
	}
	else
	{
		ERROR_AND_DIE("Incorrect Team asked for Supply Add.");
	}
}

// -----------------------------------------------------------------------
int Map::GetWoodForTeam( int team )
{
	if(team == 0)
	{
		return m_currentWoodTeam0;
	}
	else if(team == 1)
	{
		return m_currentWoodTeam1;
	}
	else
	{
		ERROR_AND_DIE("Incorrect Team asked for Wood Amount.");
	}
}

// -----------------------------------------------------------------------
int Map::GetMineralForTeam( int team )
{
	if(team == 0)
	{
		return m_currentMineralTeam0;
	}
	else if(team == 1)
	{
		return m_currentMineralTeam1;
	}
	else
	{
		ERROR_AND_DIE("Incorrect Team asked for Mineral Amount.");
	}
}

// -----------------------------------------------------------------------
int Map::GetSupplyForTeam( int team )
{
	if(team == 0)
	{
		return m_currentSupplyTeam0;
	}
	else if(team == 1)
	{
		return m_currentSupplyTeam1;
	}
	else
	{
		ERROR_AND_DIE("Incorrect Team asked for Supply Amount.");
	}
}

// -----------------------------------------------------------------------
int Map::GetMaxSupplyForTeam( int team )
{
	if(team == 0)
	{
		return m_maxSupplyTeam0;
	}
	else if(team == 1)
	{
		return m_maxSupplyTeam1;
	}
	else
	{
		ERROR_AND_DIE("Incorrect Team asked for Max Supply Amount.");
	}
}

// -----------------------------------------------------------------------
// Hashing
// -----------------------------------------------------------------------
unsigned int Map::HashEntities()
{
	unsigned int hashedFrame = 0xffffffff;

	for(Entity* entity: m_entities)
	{
		hashedFrame = entity->GenerateHash(hashedFrame);
	}

	return hashedFrame;
}

// -----------------------------------------------------------------------
// Raycast
// -----------------------------------------------------------------------
float Map::GridRaycast( Vec2 start, Vec2 end )
{
	IntVec2 endTile = GetTileCoordFromPosition(Vec3(end.x, end.y, 0.0f));
	Vec2 direction = end - start;
	Vec2 sign = Sign(direction);
	IntVec2 intSign = IntVec2((int)sign.x, (int)sign.y);

	direction = AbsoluteValue(direction);
	Vec2 inverseDirection = Vec2(1.0f, 1.0f) / direction;

	Vec2 fractStart = Fract(start);
	Vec2 positiveDistance = Vec2(1.0f - fractStart.x, 1.0f - fractStart.y);
	Vec2 negativeDistance = fractStart;
	Vec2 distance = positiveDistance;

	if(sign.x < 0.0f)
	{
		distance.x = negativeDistance.x;
	}
	if(sign.y < 0.0f)
	{
		distance.y = negativeDistance.y;
	}

	IntVec2 tile = GetTileCoordFromPosition(Vec3(start.x, start.y, 0.0f));
	float timeMoved = 0.0f;
	while(timeMoved < 1.0f)
	{
		if(tile == endTile)
		{
			return 1.0f;
		}

		// Make sure that we do not Raycast out of bounds, then break when we have a bad tile index;
		if(!IsTileCoordInBounds(tile, m_tileDimensions))
		{
			return timeMoved;
		}

		int tileIndex = GetIndexFromCoord(tile, m_tileDimensions);
		int occupied = m_mapTileOccupancy[tileIndex]; // 1 means occupied;
		if(occupied == 1)
		{
			return timeMoved;
		}

		Vec2 time = distance * inverseDirection;
		if(time.x <= time.y)
		{
			// Move to right;
			tile += IntVec2(intSign.x, 0);
			distance.x = 1.0f;
			distance.y -= direction.y * time.x;
			timeMoved += time.x;
		}
		else
		{
			// Move up;
			tile += IntVec2(0, intSign.y);
			distance.x -= direction.x * time.y;
			distance.y = 1.0f;
			timeMoved += time.y;
		}
	}

	return 1.0f;
}

// -----------------------------------------------------------------------
// Terrain
// -----------------------------------------------------------------------
void Map::GenerateTerrainMesh()
{
	int maxY = m_vertDimensions.y - 1;
	for(int y = 0; y < m_vertDimensions.y; y++)
	{
		for(int x = 0; x < m_vertDimensions.x; x++)
		{
			Vertex_Lit vertexLit;

			// Position;
			float positionX = 0.5f * x - 0.5f;
			float positionY = 0.5f * y - 0.5f;
			vertexLit.position = Vec3( positionX, positionY, 0.0f );

			// Color;
			vertexLit.color = Rgba::WHITE;

			// UVTexCoords;
			float u = 0.5f * x;
			float v = 0.5f * maxY;
			
			vertexLit.uvTexCoords = Vec2( u, v );

			// Normal;
			vertexLit.normal = Vec3( 0.0f, 0.0f, -1.0f );

			// Tangent;
			vertexLit.tangent = Vec3( 1.0f, 0.0f, 0.0f );

			// BiTangent
			vertexLit.bitangent = Vec3( 0.0f, 1.0f, 0.0f );

			// Add vert to array;
			m_mapVerts.push_back( vertexLit );
		}

		maxY--;
	}

	// Add indices;
	/*

	36 ...
	18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35
	0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17

	*/
	std::vector<uint> indices;
	for(int y = 0; y < m_vertDimensions.y - 1; y++)
	{
		for(int x = 0; x < m_vertDimensions.x - 1; x++)
		{
			int bottomLeft = x + ( y * m_vertDimensions.x );
			int bottomRight = x + ( y * m_vertDimensions.x ) + 1;
			int topLeft = x + ( y * m_vertDimensions.x ) + m_vertDimensions.x;
			int topRight = x + ( y * m_vertDimensions.x ) + m_vertDimensions.x + 1;

			indices.push_back( bottomLeft );
			indices.push_back( bottomRight );
			indices.push_back( topRight );

			indices.push_back( bottomLeft );
			indices.push_back( topRight );
			indices.push_back( topLeft );
		}
	}

	m_terrainMesh->CreateFromVertices( m_mapVerts, indices );
}
// Vec2 Map::SOUTHEAST	= Vec2(0.0f, -1.0f);
// Vec2 Map::SOUTH		= Vec2(-0.777f, -0.777f);
// Vec2 Map::SOUTHWEST	= Vec2(-1.0f, 0.0f);
// Vec2 Map::WEST		= Vec2(-0.777f, 0.777f);
// Vec2 Map::NORTHWEST	= Vec2(0.0f, 1.0f);
// Vec2 Map::NORTH		= Vec2(0.777f, 0.777f);
// Vec2 Map::NORTHEAST	= Vec2(1.0f, 0.0f);
// Vec2 Map::EAST		= Vec2(0.777f, -0.777f);

Vec2 Map::SOUTHEAST	= Vec2(0.777f, -0.777f);



Vec2 Map::SOUTH		= Vec2(0.0f, -1.0f);
Vec2 Map::SOUTHWEST	= Vec2(-0.777f, -0.777f);
Vec2 Map::WEST		= Vec2(-1.0f, 0.0f);
Vec2 Map::NORTHWEST	= Vec2(-0.777f, 0.777f);
Vec2 Map::NORTH		= Vec2(0.0f, 1.0f);
Vec2 Map::NORTHEAST	= Vec2(0.777f, 0.777f);
Vec2 Map::EAST		= Vec2(1.0f, 0.0f);