
// -----------------------------------------------------------------------
#include "Game/Gameplay/Entity.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Math/Ray.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Gameplay/Camera/OrbitCamera.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Game/Gameplay/EntityDefinition.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Game/Gameplay/Map/Map.hpp"
#include "Game/Gameplay/Input/ReplayController.hpp"
#include "Engine/Renderer/SpriteAnimationDefinition.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/RandomNumberGenerator.hpp"
#include "Game/Gameplay/RTSCommand.hpp"
#include "Game/Gameplay/RTSTask.hpp"
#include "Engine/Renderer/Prop.hpp"
#include "Engine/Renderer/Model.hpp"
#include "Game/Framework/App.hpp"
#include "Engine/Job/Jobs.hpp"
#include "Game/Framework/Jobs/PathJob.hpp"

// -----------------------------------------------------------------------
Entity::Entity(Game* theGame, std::string entityType)
{
	m_directions.push_back(Map::SOUTH);
	m_directions.push_back(Map::SOUTHWEST);
	m_directions.push_back(Map::WEST);
	m_directions.push_back(Map::NORTHWEST);
	m_directions.push_back(Map::NORTH);
	m_directions.push_back(Map::NORTHEAST);
	m_directions.push_back(Map::EAST);
	m_directions.push_back(Map::SOUTHEAST);

	m_theGame = theGame;
	m_theMap = m_theGame->GetGameMap();

	// Set the GameHandle;
	uint16_t slot = m_theMap->GetFreeEntityIndex();
	uint16_t cyclicId = m_theMap->GetNextCyclicId();
	GameHandle handle = GameHandle(cyclicId, slot);
	m_gameHandle = handle;

	// Get the Entity Definition;
	std::map<std::string, EntityDefinition*>::iterator it = EntityDefinition::s_entityDefinitions.find(entityType);
	if(it == EntityDefinition::s_entityDefinitions.end())
	{
		// Bad things happen;
	}
	m_entityDefinition = it->second;

	// Set Stats;
	m_speed				= m_entityDefinition->m_speed;
	m_attack			= m_entityDefinition->m_attack;
	m_attackSpeed		= m_entityDefinition->m_attackSpeed;
	m_height			= m_entityDefinition->m_height;
	m_physicsRadius		= m_entityDefinition->m_physicsRadius;
	m_maxHealth			= m_entityDefinition->m_health;
	m_previousHealth	= m_entityDefinition->m_health;
	m_health			= m_entityDefinition->m_health;
	m_range				= m_entityDefinition->m_range;
	m_isResource		= m_entityDefinition->m_resource;

	if(m_entityDefinition->m_prop)
	{
		m_prop = new Prop(*m_entityDefinition->m_prop);
	}
	

	// Animations;
	if(entityType == "Peon")
	{
		m_theMap->LoadPeonAnimations(this, entityType);
	}
	else if(entityType == "Warrior")
	{
		m_theMap->LoadWarriorAnimations(this, entityType);
	}
	else if(entityType == "Goblin")
	{
		m_theMap->LoadGoblinAnimations(this, entityType);
	}

	m_currentAnimationState = ANIMATION_IDLE;
	m_previousAnimationState = ANIMATION_IDLE;
	m_currentAnimationDirection = DIRECTION_SOUTH;
	m_previousAnimationDirection = DIRECTION_SOUTH;	
}

// -----------------------------------------------------------------------
Entity::~Entity()
{
	delete m_prop;
	m_prop = nullptr;
}

// -----------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	if(m_justConstructed)
	{
		m_justConstructedTimer += deltaSeconds;
		if(m_justConstructedTimer >= m_justConstructedTimerThreshold)
		{
			m_justConstructed = false;
		}
	}

	if(m_health <= 0.0f && !m_isDead)
	{
		if(m_isResource)
		{
			m_isDead = true;
			m_isSelectable = false;
			m_tasks.clear();
			
			std::map< std::string, EntityDefinition* >::iterator resource = EntityDefinition::s_entityDefinitions.find("pinewhole");
			if(m_entityDefinition == resource->second)
			{
				DieCommand* dieCommand = new DieCommand();
				dieCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
				dieCommand->m_commandType = 4;
				dieCommand->m_unit = GetGameHandle();
				dieCommand->m_theMap = m_theGame->GetGameMap();
				dieCommand->m_instantDeath = true;
				m_theGame->EnQueueCommand(dieCommand);
				g_theReplayController->RecordCommand(dieCommand);
				
				CreateCommand* createCommand = new CreateCommand();
				createCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
				createCommand->m_theGame = g_theApp->m_theGame;
				createCommand->m_commandType = 10;
				createCommand->m_team = 99;
				createCommand->m_theMap = g_theApp->m_theGame->GetGameMap();
				createCommand->m_createPosition = m_position;
				g_theApp->m_theGame->EnQueueCommand(createCommand);
				g_theReplayController->RecordCommand(createCommand);
			}

			std::map< std::string, EntityDefinition* >::iterator resource2 = EntityDefinition::s_entityDefinitions.find("pinebark");
			if(m_entityDefinition == resource2->second)
			{
				DieCommand* dieCommand = new DieCommand();
				dieCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
				dieCommand->m_commandType = 4;
				dieCommand->m_unit = GetGameHandle();
				dieCommand->m_theMap = m_theGame->GetGameMap();
				dieCommand->m_instantDeath = true;
				m_theGame->EnQueueCommand(dieCommand);
				g_theReplayController->RecordCommand(dieCommand);
				
				CreateCommand* createCommand = new CreateCommand();
				createCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
				createCommand->m_theGame = g_theApp->m_theGame;
				createCommand->m_commandType = 11;
				createCommand->m_team = 99;
				createCommand->m_theMap = g_theApp->m_theGame->GetGameMap();
				createCommand->m_createPosition = m_position;
				g_theApp->m_theGame->EnQueueCommand(createCommand);
				g_theReplayController->RecordCommand(createCommand);
			}

			std::map< std::string, EntityDefinition* >::iterator resource3 = EntityDefinition::s_entityDefinitions.find("pinestump");
			if(m_entityDefinition == resource3->second)
			{
				DieCommand* dieCommand = new DieCommand();
				dieCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
				dieCommand->m_commandType = 4;
				dieCommand->m_unit = GetGameHandle();
				dieCommand->m_theMap = m_theGame->GetGameMap();
				dieCommand->m_deathTimerThreshold = 1.0f;
				m_theGame->EnQueueCommand(dieCommand);
				g_theReplayController->RecordCommand(dieCommand);
			}
		}
		else
		{
			m_isDead = true;
			m_isSelectable = false;
			m_tasks.clear();

			DieCommand* dieCommand = new DieCommand();
			dieCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
			dieCommand->m_commandType = 4;
			dieCommand->m_unit = GetGameHandle();
			dieCommand->m_theMap = m_theGame->GetGameMap();
			m_theGame->EnQueueCommand(dieCommand);
			g_theReplayController->RecordCommand(dieCommand);
		}
	}

	if(m_prop)
	{
		//m_prop->m_model->m_modelMatrix.SetT(m_position);

		IntVec2 tile = IntVec2((int)floor(m_position.x), (int)floor(m_position.y));
		m_prop->m_model->m_modelMatrix.SetT(Vec3((float)tile.x, (float)tile.y, m_position.z));
		m_position = m_prop->m_model->m_modelMatrix.GetT();
		return;
	}
	else
	{
		UpdateAnimation(deltaSeconds);

		

		// Capsule Updates;
		Matrix4x4 rotationMatrix;
		Matrix4x4 modelMatrix;

		rotationMatrix = rotationMatrix.MakeXRotationDegrees(90.0f);
		modelMatrix = modelMatrix.Transform(rotationMatrix);

		m_capsuleModel = modelMatrix;
		m_capsuleModel.SetT(Vec3(m_position));
	}
}

// -----------------------------------------------------------------------
void Entity::UpdateAnimation( float deltaSeconds )
{
	m_animationTimer += deltaSeconds;

	SpriteAnimationDefinition* spriteAnimationDefinition = m_animationSet[m_currentAnimationState][m_currentAnimationDirection];
	m_currentSpriteDefinition = spriteAnimationDefinition->GetSpriteDefinitionAtTime(m_animationTimer);

	if(m_currentAnimationState == ANIMATION_ATTACK)
	{
		int animationFrame = spriteAnimationDefinition->GetAnimationFrameAtTime(m_animationTimer);
		if(animationFrame == 3 && !m_soundPlayed)
		{
			int num = g_theRandomNumberGenerator->GetRandomIntInRange(0, 3);
			ChannelGroupID sfxGroup = g_theAudioSystem->CreateOrGetChannelGroup("SFX");

			if(m_entityToAttack)
			{

				SoundID testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/hit00.wav" );
				if(num == 1)
				{
					testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/hit01.wav" );
				}
				else if(num == 2)
				{
					testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/hit02.wav" );
				}
				else if(num == 3)
				{
					testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/hit04.wav" );
				}

				g_theAudioSystem->Set3DListener(m_theGame->GetGameCamera());
				g_theAudioSystem->Play3DSound( testSound, sfxGroup, m_position );
				m_soundPlayed = true;
				m_entityToAttack->m_health -= m_attack;
			}
			else if(m_entityToGather)
			{
				m_entityToGather->m_health -= m_attack;
				if(m_entityToGather->m_entityDefinition->m_resourceType == RESOURCETYPE_WOOD)
				{
					SoundID testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/wood_hit00.wav" );
					if(num == 1)
					{
						testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/wood_hit00.wav" );
					}
					else if(num == 2)
					{
						testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/wood_hit01.wav" );
					}
					else if(num == 3)
					{
						testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/wood_hit02.wav" );
					}

					g_theAudioSystem->Set3DListener(m_theGame->GetGameCamera());
					g_theAudioSystem->Play3DSound( testSound, sfxGroup, m_position );
					m_soundPlayed = true;
					m_woodAmount++;
				}
				if(m_entityToGather->m_entityDefinition->m_resourceType == RESOURCETYPE_MINERAL)
				{
					SoundID testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/mineral_hit00.wav" );
					if(num == 1)
					{
						testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/mineral_hit00.wav" );
					}
					else if(num == 2)
					{
						testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/mineral_hit00.wav" );
					}
					else if(num == 3)
					{
						testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/mineral_hit01.wav" );
					}

					g_theAudioSystem->Set3DListener(m_theGame->GetGameCamera());
					g_theAudioSystem->Play3DSound( testSound, sfxGroup, m_position );
					m_soundPlayed = true;
					m_mineralAmount++;
				}
			}
			else if(m_entityToRepair)
			{
				SoundID testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/wood_hit00.wav" );
				if(num == 1)
				{
					testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/wood_hit00.wav" );
				}
				else if(num == 2)
				{
					testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/wood_hit01.wav" );
				}
				else if(num == 3)
				{
					testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/wood_hit02.wav" );
				}

				g_theAudioSystem->Set3DListener(m_theGame->GetGameCamera());
				g_theAudioSystem->Play3DSound( testSound, sfxGroup, m_position );
				m_soundPlayed = true;
				float buildTime = m_entityToRepair->m_entityDefinition->m_buildtime;
				float deltaSecondPercentage = deltaSeconds * buildTime;
				float healthPerDeltaSecond = m_entityToRepair->m_maxHealth * deltaSecondPercentage;
				m_entityToRepair->m_health += healthPerDeltaSecond;
			}
		}
		else if(animationFrame != 3)
		{
			m_soundPlayed = false;
		}
	}
}

void Entity::RenderProp()
{
	if(!m_prop)
	{
		
		return;
	}

	m_prop->Render();
}

// -----------------------------------------------------------------------
void Entity::Render(std::vector<Vertex_PCU>* entityVertsPeon, std::vector<Vertex_PCU>* entityVertsWarrior, std::vector<Vertex_PCU>* entityVertsGoblin, std::vector<Vertex_PCU>* entityUIVerts, std::vector<Vertex_PCU>* entitySelectVerts)
{
	if(!m_isValid)
	{
		return;
	}

	if(m_isResource)
	{
		//DrawResourceHealthBars(entityUIVerts);
		DrawSelectionCircles(entitySelectVerts);
		return;
	}

	DrawUnitHealthBars(entityUIVerts);
	if(m_isTraining)
	{
		DrawUnitTrainBars(entityUIVerts);
	}
	DrawSelectionCircles(entitySelectVerts);

	if(m_prop)
	{
		return;
	}

	g_theRenderer->BindModelMatrix(Matrix4x4::IDENTITY);

	if(m_entityDefinition->m_entityType == "Warrior")
	{
		DrawBillboardedSprite(entityVertsWarrior, m_capsuleModel.GetT(), &m_currentSpriteDefinition, m_theGame->GetGameCamera());
	}
	else if(m_entityDefinition->m_entityType == "Peon")
	{
		DrawBillboardedSprite(entityVertsPeon, m_capsuleModel.GetT(), &m_currentSpriteDefinition, m_theGame->GetGameCamera());
	}
	else if(m_entityDefinition->m_entityType == "Goblin")
	{
		DrawBillboardedSprite(entityVertsGoblin, m_capsuleModel.GetT(), &m_currentSpriteDefinition, m_theGame->GetGameCamera());
	}
}

// -----------------------------------------------------------------------
void Entity::DrawUnitHealthBars(std::vector<Vertex_PCU>* entityUIVerts)
{
	if(m_isDead)
	{
		return;
	}
	
	Vec3 position = Vec3(0.0f, 0.0f, 0.0f);
	if(m_prop)
	{
		position = m_prop->m_model->m_modelMatrix.GetT();
	}
	else
	{
		position = m_capsuleModel.GetT();
	}

	OrbitCamera* camera = m_theGame->GetGameCamera();

	// Black;
	Vec3 blackCorners[4];

	float width = 0.5f;
	float height = 0.1f;
	Vec2 pivot = Vec2(0.5f, -2.0f);

	Vec3 right = camera->GetRight();
	Vec3 up = camera->GetUp();

	blackCorners[0] = position + height * up;
	blackCorners[1] = position + height * up + width * right;
	blackCorners[2] = position;
	blackCorners[3] = position + width * right;

	Vec2 localOffset = (pivot * Vec2(width, height));
	localOffset.x *= -1.0f;
	localOffset.y *= -1.0f;
	Vec3 worldOffset = localOffset.x * right + localOffset.y * up;

	for(uint i = 0; i < 4; i++)
	{
		blackCorners[i] += worldOffset;
	}

	
	AddVertsForAABB3D(*entityUIVerts, blackCorners, Rgba::BLACK);

	// Red;
	Vec3 redCorners[4];

	if(m_previousHealth > m_health * 5.0f)
	{
		m_previousHealth = m_health;
	}

	float healthRatio = m_previousHealth / m_maxHealth;
	width = 0.5f * healthRatio;

	redCorners[0] = position + height * up;
	redCorners[1] = position + height * up + width * right;
	redCorners[2] = position;
	redCorners[3] = position + width * right;

	for(uint i = 0; i < 4; i++)
	{
		redCorners[i] += worldOffset;
	}

	
	if(m_previousHealth > m_health)
	{
		m_previousHealth -= 0.5f;
	}
	else if((m_previousHealth - m_health) < 1.0f)
	{
		m_previousHealth = m_health;
	}

	AddVertsForAABB3D(*entityUIVerts, redCorners, Rgba::RED);

	// Green;
	Vec3 greenCorners[4];

	healthRatio = m_health / m_maxHealth;
	width = 0.5f * healthRatio;

	greenCorners[0] = position + height * up;
	greenCorners[1] = position + height * up + width * right;
	greenCorners[2] = position;
	greenCorners[3] = position + width * right;

	for(uint i = 0; i < 4; i++)
	{
		greenCorners[i] += worldOffset;
	}

	Rgba healthColor = Rgba::GREEN;
	if(healthRatio < 0.8f && healthRatio > 0.3f)
	{
		healthColor = Rgba::YELLOW;
	}
	else if(healthRatio <= 0.3f)
	{
		healthColor = Rgba::RED;
	}

	AddVertsForAABB3D(*entityUIVerts, greenCorners, healthColor);
}

void Entity::DrawUnitTrainBars( std::vector<Vertex_PCU>* entityUIVerts )
{
	if(m_isDead)
	{
		return;
	}

	Vec3 position = Vec3(0.0f, 0.0f, 0.0f);
	if(m_prop)
	{
		position = m_prop->m_model->m_modelMatrix.GetT();
	}
	else
	{
		position = m_capsuleModel.GetT();
	}

	OrbitCamera* camera = m_theGame->GetGameCamera();

	// Black;
	Vec3 blackCorners[4];

	float width = 0.5f;
	float height = 0.1f;
	Vec2 pivot = Vec2(0.5f, -3.0f);

	Vec3 right = camera->GetRight();
	Vec3 up = camera->GetUp();

	blackCorners[0] = position + height * up;
	blackCorners[1] = position + height * up + width * right;
	blackCorners[2] = position;
	blackCorners[3] = position + width * right;

	Vec2 localOffset = (pivot * Vec2(width, height));
	localOffset.x *= -1.0f;
	localOffset.y *= -1.0f;
	Vec3 worldOffset = localOffset.x * right + localOffset.y * up;

	for(uint i = 0; i < 4; i++)
	{
		blackCorners[i] += worldOffset;
	}


	AddVertsForAABB3D(*entityUIVerts, blackCorners, Rgba::BLACK);

	// Red;
	Vec3 blueCorners[4];

	float timeRatio = m_buildTimer / m_maxBuildTime;
	width = 0.5f * timeRatio;

	blueCorners[0] = position + height * up;
	blueCorners[1] = position + height * up + width * right;
	blueCorners[2] = position;
	blueCorners[3] = position + width * right;

	for(uint i = 0; i < 4; i++)
	{
		blueCorners[i] += worldOffset;
	}

	AddVertsForAABB3D(*entityUIVerts, blueCorners, Rgba::BLUE);
}

void Entity::DrawResourceHealthBars(std::vector<Vertex_PCU>* entityUIVerts)
{
	if(m_isDead)
	{
		return;
	}

	Vec3 position = m_prop->m_model->m_modelMatrix.GetT();
	OrbitCamera* camera = m_theGame->GetGameCamera();
	g_theRenderer->BindModelMatrix(Matrix4x4::IDENTITY);

	g_theRenderer->BindTextureView(0u, nullptr);
	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");

	// Black;
	Vec3 blackCorners[4];

	float width = 0.5f;
	float height = 0.1f;
	Vec2 pivot = Vec2(0.5f, -2.0f);

	Vec3 right = camera->GetRight();
	Vec3 up = camera->GetUp();

	blackCorners[0] = position + height * up;
	blackCorners[1] = position + height * up + width * right;
	blackCorners[2] = position;
	blackCorners[3] = position + width * right;

	Vec2 localOffset = (pivot * Vec2(width, height));
	localOffset.x *= -1.0f;
	localOffset.y *= -1.0f;
	Vec3 worldOffset = localOffset.x * right + localOffset.y * up;

	for(uint i = 0; i < 4; i++)
	{
		blackCorners[i] += worldOffset;
	}

	AddVertsForAABB3D(*entityUIVerts, blackCorners, Rgba::BLACK);

	// Red;
	Vec3 redCorners[4];

	float healthRatio = m_previousHealth / m_maxHealth;
	width = 0.5f * healthRatio;

	redCorners[0] = position + height * up;
	redCorners[1] = position + height * up + width * right;
	redCorners[2] = position;
	redCorners[3] = position + width * right;

	for(uint i = 0; i < 4; i++)
	{
		redCorners[i] += worldOffset;
	}

	if(m_previousHealth > m_health)
	{
		m_previousHealth -= 0.5f;
	}
	else if((m_previousHealth - m_health) < 1.0f)
	{
		m_previousHealth = m_health;
	}

	AddVertsForAABB3D(*entityUIVerts, redCorners, Rgba::RED);

	// Blue;
	Vec3 blueCorners[4];

	healthRatio = m_health / m_maxHealth;
	width = 0.5f * healthRatio;

	blueCorners[0] = position + height * up;
	blueCorners[1] = position + height * up + width * right;
	blueCorners[2] = position;
	blueCorners[3] = position + width * right;

	for(uint i = 0; i < 4; i++)
	{
		blueCorners[i] += worldOffset;
	}

	Rgba healthColor = Rgba(0.39f, 0.53f, 0.9f, 1.0f);

	AddVertsForAABB3D(*entityUIVerts, blueCorners, healthColor);
}

// -----------------------------------------------------------------------
void Entity::DrawSelectionCircles(std::vector<Vertex_PCU>* entityUIVerts)
{
	if(m_isDead)
	{
		return;
	}

	

	g_theRenderer->BindTextureView(0u, nullptr);
	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
	g_theRenderer->BindModelMatrix(Matrix4x4::IDENTITY);

	if(m_prop)
	{
		if(m_isSelected)
		{
			AddVertsForRing2D(*entityUIVerts, Vec2(m_position.x, m_position.y), m_physicsRadius * 1.2f, 0.03f, m_teamColor);
		}

		if(m_isHovered)
		{
			AddVertsForRing2D(*entityUIVerts, Vec2(m_position.x, m_position.y), m_physicsRadius * 0.9f, 0.03f, m_teamColor);
		}
	}
	else
	{
		if(m_isSelected)
		{
			AddVertsForRing2D(*entityUIVerts, Vec2(m_position.x - m_physicsRadius * 0.8f, m_position.y - m_physicsRadius), m_physicsRadius * 1.2f, 0.03f, m_teamColor);
		}

		if(m_isHovered)
		{
			AddVertsForRing2D(*entityUIVerts, Vec2(m_position.x - m_physicsRadius * 0.8f, m_position.y - m_physicsRadius), m_physicsRadius * 0.9f, 0.03f, m_teamColor);
		}
	}
	
}

// -----------------------------------------------------------------------
void Entity::Destroy()
{
	m_isDestoyed = true;
}

// -----------------------------------------------------------------------
void Entity::SetSelected( bool selected )
{
	m_isSelected = selected;
}

// -----------------------------------------------------------------------
void Entity::DrawBillboardedSprite(std::vector<Vertex_PCU>* entityVerts, Vec3 position, SpriteDefinition* sprite, OrbitCamera* camera )
{
	Vec3 corners[4];

	float width = 1.0f;
	float height = 1.0f;
	Vec2 pivot = Vec2(0.5f, 0.5f);

	Vec3 right = camera->GetRight();
	Vec3 up = camera->GetUp();
	//Vec3 up = Vec3(0.0f, 0.0f, -1.0f);

	corners[0] = position + height * up;
	corners[1] = position + height * up + width * right;
	corners[2] = position;
	corners[3] = position + width * right;

	Vec2 localOffset = (pivot * Vec2(width, height));
	localOffset.x *= -1.0f;
	localOffset.y *= -1.0f;
	Vec3 worldOffset = localOffset.x * right + localOffset.y * up;

	for(uint i = 0; i < 4; i++)
	{
		corners[i] += worldOffset;
	}

	sprite->GetUVs(m_currentSpriteAnimationBottomLeftUV, m_currentSpriteAnimationToptUV);

	AddVertsForAABB3D(*entityVerts, corners, m_teamColor, m_currentSpriteAnimationBottomLeftUV, m_currentSpriteAnimationToptUV);
	
}

// -----------------------------------------------------------------------
bool Entity::IsDestroyed()
{
	return m_isDestoyed;
}

// -----------------------------------------------------------------------
bool Entity::IsSelectable()
{
	return m_isSelectable;
}

// -----------------------------------------------------------------------
bool Entity::IsSelected()
{
	return m_isSelected;
}

// -----------------------------------------------------------------------
bool Entity::IsHovered()
{
	return m_isHovered;
}

// -----------------------------------------------------------------------
void Entity::SetHovered( bool hovered )
{
	m_isHovered = hovered;
}

// -----------------------------------------------------------------------
AnimationState Entity::GetAnimationState()
{
	return m_currentAnimationState;
}

// -----------------------------------------------------------------------
AnimationDirection Entity::GetAnimationDirection()
{
	return m_currentAnimationDirection;
}

// -----------------------------------------------------------------------
void Entity::SetAnimationState( AnimationState newState )
{
	m_animationTimer = m_currentAnimationState != m_previousAnimationState ? 0.0f : m_animationTimer;

	m_previousAnimationState = m_currentAnimationState;
	m_currentAnimationState = newState;
}

// -----------------------------------------------------------------------
void Entity::SetAnimationDirection( AnimationDirection animationDirection )
{
	m_previousAnimationDirection = m_currentAnimationDirection;
	m_currentAnimationDirection = animationDirection;
}

// -----------------------------------------------------------------------
void Entity::SetPosition( Vec3 position )
{
	m_position = position;
}

// -----------------------------------------------------------------------
void Entity::SetTargetPosition( Vec3 targetPosition )
{
	m_targetPosition = targetPosition;
}

// -----------------------------------------------------------------------
Vec3 Entity::GetPosition()
{
	return m_position;
}

// -----------------------------------------------------------------------
void Entity::PushOutOfOtherEntities()
{
	if(m_entityDefinition->m_speed == 0.0f)
	{
		return;
	}

	std::vector<Entity*> entities = m_theGame->GetGameMap()->GetAllValidEntities();
	for(Entity* entity: entities)
	{
		if(entity && this != entity && !this->m_isDead && !entity->m_isDead)
		{
			Vec2 myPosition = Vec2(m_position.x, m_position.y);
			Vec2 otherPosition = Vec2(entity->m_position.x, entity->m_position.y);

			bool overLapOtherEntity = DoDiscsOverlap(myPosition, m_physicsRadius, otherPosition, entity->m_physicsRadius);
			if(overLapOtherEntity)
			{
				PushDiscsOutOfDiscs(myPosition, m_physicsRadius, otherPosition, entity->m_physicsRadius);

				m_position = Vec3(myPosition, m_position.z);
				if(!entity->m_prop)
				{
					entity->m_position = Vec3(otherPosition, entity->m_position.z);
				}
				
			}
		}
	}
}

// -----------------------------------------------------------------------
void Entity::ProcessTasks(float deltaSeconds)
{
	if(m_tasks.size() == 0)
	{
		SetAnimationState(ANIMATION_IDLE);
		SetAnimationDirection(m_currentAnimationDirection);
		return;
	}
	
	RTSTask* currentTask = m_tasks.front();

	switch(currentTask->m_taskStatus)
	{
		case TASKSTATUS_START:
		{
			currentTask->Start(deltaSeconds);

			break;
		}
		case TASKSTATUS_DO:
		{
			currentTask->DoTask(deltaSeconds);

			break;
		}
		case TASKSTATUS_END:
		{
			delete currentTask;
			currentTask = nullptr;

			m_tasks.erase(m_tasks.begin());

			if(m_isValid)
			{
				ProcessTasks(deltaSeconds);
			}			

			break;
		}
	}
}

// -----------------------------------------------------------------------
bool Entity::Raycast( float* out, Ray3* ray )
{
	Capsule3 entityCapsule;
	entityCapsule.start = m_position + Vec3(0.0f, 0.0f, -m_height);
	entityCapsule.end = m_position;
	entityCapsule.radius = m_physicsRadius;
	
	uint numHits = 0;
	numHits = ::Raycast(out, ray, entityCapsule);

	return numHits;
}

// -----------------------------------------------------------------------
GameHandle Entity::GetGameHandle()
{
	return m_gameHandle;
}

// -----------------------------------------------------------------------
void Entity::AddTask(RTSTask* task)
{
	m_tasks.push_back(task);
}

// -----------------------------------------------------------------------
void Entity::ClearTasks()
{
	for(RTSTask* task: m_tasks)
	{
		delete task;
		task = nullptr;
	}
	m_tasks.clear();
}

// -----------------------------------------------------------------------
// Team
// -----------------------------------------------------------------------
void Entity::SetCurrentTeam( int teamID )
{
	m_team = teamID;
}

// -----------------------------------------------------------------------
int Entity::GetCurrentTeam()
{
	return m_team;
}

// -----------------------------------------------------------------------
bool Entity::IsOnTeam( int teamID )
{
	return m_team == teamID;
}

// -----------------------------------------------------------------------
// Command
// -----------------------------------------------------------------------
void Entity::ProcessRightClickCommand( RightClickCommand* rightClickCommand )
{
	if(rightClickCommand->m_rightClickPosition != Vec3(0.0f, 0.0f, 0.0f))
	{
		if(rightClickCommand->m_rightClickEntity == GameHandle::INVALID)
		{
			ClearTasks();
			AssignSelfMoveTask(rightClickCommand);
		}
		else
		{
			Entity* otherEntity = m_theMap->FindEntity(rightClickCommand->m_rightClickEntity);
			if(otherEntity)
			{
				if(GetCurrentTeam() == otherEntity->GetCurrentTeam())
				{
					if(otherEntity->m_entityDefinition->m_canBeRepaired && otherEntity->m_health != otherEntity->m_maxHealth)
					{
						AssignSelfRepairTask(rightClickCommand);
					}
					else
					{
						if(otherEntity->m_prop)
						{
							AssignSelfMoveTask(rightClickCommand);
						}
						else
						{
							AssignSelfFollowTask(rightClickCommand);
						}
					}
				}
				else if(GetCurrentTeam() != otherEntity->GetCurrentTeam() && otherEntity->m_isResource)
				{
					AssignSelfGatherTask(rightClickCommand);
				}
				else if(GetCurrentTeam() != otherEntity->GetCurrentTeam() && !otherEntity->m_isResource)
				{
					AssignSelfAttackTask(rightClickCommand);
				}
			}
		}
	}	
}

// -----------------------------------------------------------------------
void Entity::AssignSelfMoveTask( RightClickCommand* rightClickCommand )
{
	std::map< std::string, RTSTaskInfo* >::iterator found = m_entityDefinition->m_availableTasks.find("move");
	if(found != m_entityDefinition->m_availableTasks.end())
	{
		RTSTaskInfo* rtsTaskInfo = found->second;
		MoveTask* moveTask = static_cast<MoveTask*>(rtsTaskInfo->rtsTask->Clone());
		moveTask->m_taskStatus = TASKSTATUS_START;
		moveTask->m_unit = rightClickCommand->m_unit;
		moveTask->m_theMap = rightClickCommand->m_theMap;
		moveTask->m_targetPosition = rightClickCommand->m_rightClickPosition;
		moveTask->m_targetEntity = rightClickCommand->m_rightClickEntity;

		AddTask(moveTask);
	}
}

// -----------------------------------------------------------------------
void Entity::AssignSelfAttackTask( RightClickCommand* rightClickCommand )
{
	std::map< std::string, RTSTaskInfo* >::iterator found = m_entityDefinition->m_availableTasks.find("attack");
	if(found != m_entityDefinition->m_availableTasks.end())
	{
		RTSTaskInfo* rtsTaskInfo = found->second;
		AttackTask* attackTask = static_cast<AttackTask*>(rtsTaskInfo->rtsTask->Clone());
		attackTask->m_taskStatus = TASKSTATUS_START;
		attackTask->m_unit = rightClickCommand->m_unit;
		attackTask->m_theMap = rightClickCommand->m_theMap;
		attackTask->m_entityToAttack = rightClickCommand->m_rightClickEntity;

		AddTask(attackTask);
	}
	else
	{
		AssignSelfMoveTask(rightClickCommand);
	}
}

// -----------------------------------------------------------------------
void Entity::AssignSelfFollowTask( RightClickCommand* rightClickCommand )
{
	std::map< std::string, RTSTaskInfo* >::iterator found = m_entityDefinition->m_availableTasks.find("follow");
	if(found != m_entityDefinition->m_availableTasks.end())
	{
		RTSTaskInfo* rtsTaskInfo = found->second;
		FollowTask* followTask = static_cast<FollowTask*>(rtsTaskInfo->rtsTask->Clone());
		followTask->m_taskStatus = TASKSTATUS_START;
		followTask->m_unit = rightClickCommand->m_unit;
		followTask->m_theMap = rightClickCommand->m_theMap;
		followTask->m_entityToFollow = rightClickCommand->m_rightClickEntity;

		AddTask(followTask);
	}
	else
	{
		AssignSelfMoveTask(rightClickCommand);
	}
}

void Entity::AssignSelfGatherTask( RightClickCommand* rightClickCommand )
{
	std::map< std::string, RTSTaskInfo* >::iterator found = m_entityDefinition->m_availableTasks.find("gather");
	if(found != m_entityDefinition->m_availableTasks.end())
	{
		RTSTaskInfo* rtsTaskInfo = found->second;
		GatherTask* gatherTask = static_cast<GatherTask*>(rtsTaskInfo->rtsTask->Clone());
		gatherTask->m_taskStatus = TASKSTATUS_START;
		gatherTask->m_unit = rightClickCommand->m_unit;
		gatherTask->m_theMap = rightClickCommand->m_theMap;
		gatherTask->m_entityToGather = rightClickCommand->m_rightClickEntity;

		AddTask(gatherTask);
	}
	else
	{
		AssignSelfMoveTask(rightClickCommand);
	}
}

void Entity::AssignSelfRepairTask( RightClickCommand* rightClickCommand )
{
	std::map< std::string, RTSTaskInfo* >::iterator found = m_entityDefinition->m_availableTasks.find("repair");
	if(found != m_entityDefinition->m_availableTasks.end())
	{
		RTSTaskInfo* rtsTaskInfo = found->second;
		RepairTask* repairTask = static_cast<RepairTask*>(rtsTaskInfo->rtsTask->Clone());
		repairTask->m_taskStatus = TASKSTATUS_START;
		repairTask->m_unit = rightClickCommand->m_unit;
		repairTask->m_theMap = rightClickCommand->m_theMap;
		repairTask->m_entityToRepair = rightClickCommand->m_rightClickEntity;

		AddTask(repairTask);
	}
	else
	{
		AssignSelfMoveTask(rightClickCommand);
	}
}

Entity* Entity::FindNearestTownHall()
{
	Entity* closestTownHall = nullptr;
	std::vector<Entity*> townHalls;

	for(auto& entity: m_theMap->GetAllValidEntities())
	{
		if(entity->m_entityDefinition->m_entityType == "TownHall" && entity->m_buildingFinishedConstruction)
		{
			townHalls.push_back(entity);
		}
	}

	if(townHalls.size() > 0)
	{
		closestTownHall = townHalls.front();
		float shortestDistance = GetDistance(m_position, closestTownHall->m_position);

		if(townHalls.size() > 1)
		{
			for(auto& townHall: townHalls)
			{
				float distance = GetDistance(m_position, townHall->m_position);
				if(distance < shortestDistance)
				{
					shortestDistance = distance;
					closestTownHall = townHall;
				}
			}
		}
	}

	return closestTownHall;
}

Entity* Entity::FindNearestTownHallOnTeam( int team)
{
	Entity* closestTownHall = nullptr;
	std::vector<Entity*> townHalls;

	for(auto& entity: m_theMap->GetAllValidEntities())
	{
		if(entity->m_entityDefinition->m_entityType == "TownHall" && entity->m_buildingFinishedConstruction && entity->m_team == team)
		{
			townHalls.push_back(entity);
		}
	}

	if(townHalls.size() > 0)
	{
		closestTownHall = townHalls.front();
		float shortestDistance = GetDistance(m_position, closestTownHall->m_position);

		if(townHalls.size() > 1)
		{
			for(auto& townHall: townHalls)
			{
				float distance = GetDistance(m_position, townHall->m_position);
				if(distance < shortestDistance)
				{
					shortestDistance = distance;
					closestTownHall = townHall;
				}
			}
		}
	}

	return closestTownHall;
}

Entity* Entity::FindNearestResourceHubOnTeam( int team)
{
	Entity* closestResourceHub = nullptr;
	std::vector<Entity*> resourceHubs;

	for(auto& entity: m_theMap->GetAllValidEntities())
	{
		if((entity->m_entityDefinition->m_entityType == "TownHall" || entity->m_entityDefinition->m_entityType == "GoblinHut") && entity->m_team == team)
		{
			resourceHubs.push_back(entity);
		}
	}

	if(resourceHubs.size() > 0)
	{
		closestResourceHub = resourceHubs.front();
		float shortestDistance = GetDistance(m_position, closestResourceHub->m_position);

		if(resourceHubs.size() > 1)
		{
			for(auto& resourceHub: resourceHubs)
			{
				float distance = GetDistance(m_position, resourceHub->m_position);
				if(distance < shortestDistance)
				{
					shortestDistance = distance;
					closestResourceHub = resourceHub;
				}
			}
		}
	}

	return closestResourceHub;
}

Entity* Entity::FindNearestDamagedBuilding()
{
	Entity* closestBuilding = nullptr;
	std::vector<Entity*> buildings;

	for(auto& entity: m_theMap->GetAllValidEntities())
	{
		if(entity->m_entityDefinition->m_canBeRepaired && entity->m_justConstructed)
		{
			buildings.push_back(entity);
		}
	}

	if(buildings.size() > 0)
	{
		closestBuilding = buildings.front();
		float shortestDistance = GetDistance(m_position, closestBuilding->m_position);

		if(buildings.size() > 1)
		{
			for(auto& building: buildings)
			{
				float distance = GetDistance(m_position, building->m_position);
				if(distance < shortestDistance)
				{
					shortestDistance = distance;
					closestBuilding = building;
				}
			}
		}
	}

	return closestBuilding;
}

Entity* Entity::FindNearestDamagedBuildingOnTeam(int team)
{
	Entity* closestBuilding = nullptr;
	std::vector<Entity*> buildings;

	for(auto& entity: m_theMap->GetAllValidEntities())
	{
		if(entity->m_entityDefinition->m_canBeRepaired && entity->m_justConstructed && entity->m_team == team)
		{
			buildings.push_back(entity);
		}
	}

	if(buildings.size() > 0)
	{
		closestBuilding = buildings.front();
		float shortestDistance = GetDistance(m_position, closestBuilding->m_position);

		if(buildings.size() > 1)
		{
			for(auto& building: buildings)
			{
				float distance = GetDistance(m_position, building->m_position);
				if(distance < shortestDistance)
				{
					shortestDistance = distance;
					closestBuilding = building;
				}
			}
		}
	}

	return closestBuilding;
}

Entity* Entity::FindNearestTree()
{
	Entity* closestTree = nullptr;
	std::vector<Entity*> trees;

	for(auto& entity: m_theMap->GetAllValidEntities())
	{
		if(entity->m_entityDefinition->m_entityType == "pinewhole"
			|| entity->m_entityDefinition->m_entityType == "pinebark"
			|| entity->m_entityDefinition->m_entityType == "pinestump")
		{
			trees.push_back(entity);
		}
	}

	if(trees.size() > 0)
	{
		closestTree = trees.front();
		float shortestDistance = GetDistance(m_position, closestTree->m_position);

		if(trees.size() > 1)
		{
			for(auto& tree: trees)
			{
				float distance = GetDistance(m_position, tree->m_position);
				if(distance < shortestDistance)
				{
					shortestDistance = distance;
					closestTree = tree;
				}
			}
		}
	}

	return closestTree;
}

// -----------------------------------------------------------------------
Entity* Entity::FindNearestEntityUnitOnOtherTeam( int myTeam )
{
	Entity* closestEntity = nullptr;
	std::vector<Entity*> entities;
	std::vector<Entity*> validEntities = m_theMap->GetAllValidUnitsEntities();
	for(Entity* entity: validEntities)
	{
		if(entity->m_team != myTeam)
		{
			entities.push_back(entity);
		}
	}

	if(entities.size() > 0)
	{
		closestEntity = entities.front();
		float shortestDistance = GetDistance(m_position, closestEntity->m_position);

		if(entities.size() > 1)
		{
			for(auto& e: entities)
			{
				float distance = GetDistance(m_position, e->m_position);
				if(distance < shortestDistance)
				{
					shortestDistance = distance;
					closestEntity = e;
				}
			}
		}
	}

	return closestEntity;
}

// -----------------------------------------------------------------------
// Hashing
// -----------------------------------------------------------------------
unsigned int Entity::GenerateHash( unsigned int hashedFrame )
{
	hashedFrame;
	return 0;
}

void Entity::ApplyPath(PathJob* job)
{
	m_pathCreation = job->m_pathCreation;
	m_waitingOnPathJob = false;
}
