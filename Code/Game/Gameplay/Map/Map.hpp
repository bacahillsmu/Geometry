#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/VertexLit.hpp"
#include "Game/Gameplay/Pathing/Pathing.hpp"

#include <stdlib.h>
#include <vector>
#include <map>
#include <string>

class Game;
class GPUMesh;
class Material;
class Entity;
class EntityDefinition;
class GameHandle;
struct Vec3;
struct Ray3;
class SpriteSheet;
class Prop;

struct AABB2;

struct MapTile
{
	// empty for now; 
	int m_placeholder; 
};

class Map
{

public:

	Map(Game* game);
	~Map();

	bool Load( const std::string& filename, int x, int y );
	bool Create( int tileWidth, int tileHeight );
	void GenerateTerrainMesh();

	void Update(float deltaSeconds);
	void Render();
	void RenderScreenSelectedUnits(AABB2 screenDimensions, std::vector<Vertex_PCU>* verts);
	void RenderScreenSelectedTasks(AABB2 screenDimensions, std::vector<Vertex_PCU>* verts);
	void RenderScreenResourceInfo(AABB2 screenDimensions, std::vector<Vertex_PCU>* verts);
	void RenderTerrain( Material* material = nullptr );
	void CleanupEntities();
	void EndFrame();

	// Loading;
	void LoadPlaySprites();
	void LoadUnitsAndBuildingsFromXML();
	void LoadResourcesFromXML();

	// Map Tile Info;
	AABB2 GetXYBounds() const;
	void SetOccupiedTileIndexStatus(int tileIndex, int status);
	int GetOccupiedTileIndexStatus(int tileIndex);
	void UnoccupyAllTiles();
	void CheckForOccupancyMapChanges();
	void ResetPatherCosts();
	IntVec2 GetTileCoordFromPosition(Vec3 position);
	bool IsTileOccupied(IntVec2 tile);

	// Entity;
	std::vector<Entity*> GetAllValidEntities();
	std::vector<Entity*> GetAllValidBuildingEntities();
	std::vector<Entity*> GetAllValidUnitsEntities();
	Entity* FindEntity(GameHandle handle);
	Entity* GetFirstValidEntity();
	Entity* GetFirstSelectedEntity();
	void AddEntity(Entity* entity);
	void DelayAddEntity(Entity* entity);
	void CheckDelayEntity();
	void UnhoverAllEntities();
	void UnSelectAllEntities();
	bool CanAffordEntity(EntityDefinition* entityDefinition);
	void PayCostEntity(EntityDefinition* entityDefinition);
	std::vector<Entity*> GetGoblinHutsOnTeam(int team);
	std::vector<Entity*> GetEntitiesOfTypeOnTeam(std::string& type, int team);

	// Pick;
	Entity* RaycastEntity(float* hitTime, Ray3* ray, float maxDistance = INFINITY);
	bool RaycastTerrain(float* hitTime, Ray3* ray, float maxDistance = INFINITY);

	void PurgeDestroyedEntities();

	// GameHandle;
	uint16_t GetFreeEntityIndex();	// Return a free entity slot;
	uint16_t GetNextCyclicId();		// Gets the next hi-word to use, skipping 0;

	// Load Animations;
	void LoadPeonAnimations(Entity* entity, std::string& entityType);
	void LoadWarriorAnimations(Entity* entity, std::string& entityType);
	void LoadGoblinAnimations(Entity* entity, std::string& entityType);
	
	// Map Resource calculations;
	void CalculateSupplyForTeam(int team);
	void AddWoodToTeam(int woodAmount, int team);
	void AddMineralToTeam(int mineralAmount, int team);
	void AddSupplyToTeam(int supplyAmount, int team);
	int GetWoodForTeam(int team);
	int GetMineralForTeam(int team);
	int GetSupplyForTeam(int team);
	int GetMaxSupplyForTeam(int team);

	// Hashing;
	unsigned int HashEntities();

	// Raycast;
	float GridRaycast(Vec2 start, Vec2 end);


public:

	Game* m_theGame = nullptr;

	// Resource Info;
	int m_currentSupplyTeam0 = 0;
	int m_maxSupplyTeam0 = 0;
	int m_currentWoodTeam0 = 300;
	int m_currentMineralTeam0 = 300;

	int m_currentSupplyTeam1 = 0;
	int m_maxSupplyTeam1 = 0;
	int m_currentWoodTeam1 = 300;
	int m_currentMineralTeam1 = 300;

	// Dimensions;
	IntVec2 m_tileDimensions = IntVec2(-1, -1);
	IntVec2 m_vertDimensions = IntVec2(-1, -1);	

	// Map Terrain Data;
	//std::vector<MapTile> m_mapTiles;
	int m_mapSize = 0;
	std::vector<Vertex_Lit> m_mapVerts;
	std::vector<int> m_mapTileOccupancy;
	std::vector<int> m_mapTileOccupancyCopy;

	//std::map<int, Vec2> m_tileCoords;
	//std::map<int, int> m_tileOccupancy;

	GPUMesh* m_terrainMesh = nullptr;
	Material* m_terrainMaterial = nullptr;
	Matrix4x4 m_terrainModel;

	// Entity Data;
	std::vector<Entity*> m_entities;
	std::vector<Entity*> m_delayEntities;
	uint16_t m_cyclicId = 0; // Used for generating the GameHandle

	std::map<std::string, SpriteSheet*> m_spriteSheets;

	// Pathing;
	Pather m_pather;
	double m_occupancyMapChangeTimeStamp = 0.0;


	static Vec2 SOUTH;
	static Vec2 SOUTHWEST;
	static Vec2 WEST;
	static Vec2 NORTHWEST;
	static Vec2 NORTH;
	static Vec2 NORTHEAST;
	static Vec2 EAST;
	static Vec2 SOUTHEAST;

	
};