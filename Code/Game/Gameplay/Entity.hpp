#pragma once
#include "Game/Gameplay/GameHandle.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Game/Gameplay/Pathing/Pathing.hpp"

#include <map>
#include <string>

class Game;
class GameHandle;
struct Ray3;
class GPUMesh;
class RTSCommand;
class RightClickCommand;
class RTSTask;
class TextureView;
class SpriteSheet;
class SpriteAnimationDefinition;
class OrbitCamera;
class EntityDefinition;
class Map;
class Prop;
class Job;
class PathJob;

enum AnimationState
{
	ANIMATION_IDLE,
	ANIMATION_WALK,
	ANIMATION_ATTACK,
	ANIMATION_DIE,

	ANIMATION_COUNT
};

enum AnimationDirection
{
	DIRECTION_SOUTH,
	DIRECTION_SOUTHWEST,
	DIRECTION_WEST,
	DIRECTION_NORTHWEST,
	DIRECTION_NORTH,
	DIRECTION_NORTHEAST,
	DIRECTION_EAST,
	DIRECTION_SOUTHEAST,

	DIRECTION_COUNT
};

class Entity
{
	
public:
	

	Entity(Game* theGame, std::string entityType);
	~Entity();

	void Update(float deltaSeconds);
	void UpdateAnimation(float deltaSeconds);
	void Render(std::vector<Vertex_PCU>* entityVertsPeon, std::vector<Vertex_PCU>* entityVertsWarrior, std::vector<Vertex_PCU>* entityVertsGoblin, std::vector<Vertex_PCU>* entityUIVerts, std::vector<Vertex_PCU>* entitySelectVerts);
	void RenderProp();

	// Drawing Sprite;
	void DrawUnitHealthBars(std::vector<Vertex_PCU>* entityUIVerts);
	void DrawUnitTrainBars(std::vector<Vertex_PCU>* entityUIVerts);
	void DrawResourceHealthBars(std::vector<Vertex_PCU>* entityUIVerts);
	void DrawSelectionCircles(std::vector<Vertex_PCU>* entitySelectVerts);
	void DrawBillboardedSprite(std::vector<Vertex_PCU>* entityVerts, Vec3 position, SpriteDefinition* sprite, OrbitCamera* camera);

	// Entity States;
	bool IsDestroyed();
	void Destroy();
	bool IsSelectable();
	bool IsSelected();
	void SetSelected( bool selected );
	bool IsHovered();
	void SetHovered( bool hovered );

	// Entity Animation States
	AnimationState GetAnimationState();
	AnimationDirection GetAnimationDirection();
	void SetAnimationState(AnimationState newState);
	void SetAnimationDirection(AnimationDirection animationDirection);

	// Moving;
	void SetPosition( Vec3 position );
	void SetTargetPosition( Vec3 targetPosition );
	Vec3 GetPosition();

	// Physics;
	void PushOutOfOtherEntities();
	
	// Tasks;
	void ProcessTasks(float deltaSeconds);

	// Raycast;
	bool Raycast( float* out, Ray3* ray);

	GameHandle GetGameHandle();

	void AddTask(RTSTask* task);
	void ClearTasks();

	// Team;
	void SetCurrentTeam(int teamID);
	int GetCurrentTeam();
	bool IsOnTeam(int teamID);

	// Command;
	void ProcessRightClickCommand(RightClickCommand* rightClickCommand);
	void AssignSelfMoveTask(RightClickCommand* rightClickCommand);
	void AssignSelfAttackTask(RightClickCommand* rightClickCommand);
	void AssignSelfFollowTask(RightClickCommand* rightClickCommand);
	void AssignSelfGatherTask(RightClickCommand* rightClickCommand);
	void AssignSelfRepairTask(RightClickCommand* rightClickCommand);

	// Helpers;
	Entity* FindNearestTownHall();
	Entity* FindNearestTownHallOnTeam(int team);
	Entity* FindNearestResourceHubOnTeam(int team);
	Entity* FindNearestDamagedBuilding();
	Entity* FindNearestDamagedBuildingOnTeam(int team);
	Entity* FindNearestTree();
	Entity* FindNearestEntityUnitOnOtherTeam(int myTeam);

	// Hashing
	unsigned int GenerateHash(unsigned int hashedFrame);
	void ApplyPath(PathJob* job);

public:

	//PathJob* m_pathJob = nullptr;
	bool m_waitingOnPathJob = false;
	
	bool m_soundPlayed = false;
	Entity* m_entityToAttack = nullptr;
	Entity* m_entityToGather = nullptr;
	Entity* m_entityToRepair = nullptr;

	//GameHandle m_entityToAttack;
	//GameHandle m_entityToGather;
	//GameHandle m_entityToRepair;


	float m_redHealthTimer = 1.0f;


	// States;
	Vec3 m_position;
	Vec3 m_targetPosition;
	int m_team				= 0;
	Rgba m_teamColor		= Rgba::MAGENTA;
	int m_woodAmount		= 0;
	int m_maxWoodAmount		= 10;
	int m_mineralAmount		= 0;
	int m_maxMineralAmount	= 10;
	bool m_isDestoyed		= false;	
	bool m_isSelected		= false;
	bool m_isHovered		= false;
	bool m_isDead			= false;
	bool m_isValid			= true;
	bool m_justConstructed	= false;
	bool m_buildingFinishedConstruction = false;
	float m_justConstructedTimerThreshold = 1.0f;
	float m_justConstructedTimer = 0.0f;

	// Stats;
	EntityDefinition* m_entityDefinition = nullptr;
	float m_maxHealth		= 0.0f;
	float m_previousHealth	= 0.0f;
	float m_health			= 0.0f;
	float m_supply			= 0.0f;
	float m_armor			= 0.0f;
	float m_attack			= 0.0f;
	float m_range			= 0.0f;
	float m_attackSpeed		= 0.0f;
	float m_speed			= 0.0f;
	float m_buildtime		= 0.0f;
	float m_selectable		= 0.0f;
	float m_physicsRadius	= 0.0f;
	float m_height			= 0.0f;
	bool m_isSelectable		= true;
	bool m_isResource		= false;
	
	// Game and Map knowledge;
	Game* m_theGame;
	Map* m_theMap;

	// Handle;
	GameHandle m_gameHandle;

	// Tasks;
	std::vector<RTSTask*> m_tasks;

	// Animations;
	Vec3 m_facingDirection = Vec3(-0.777f, -0.777f, 0.0f);
	SpriteAnimationDefinition* m_animationSet[ANIMATION_COUNT][DIRECTION_COUNT];
	std::vector<Vec2> m_directions;
	AnimationDirection m_previousAnimationDirection;
	AnimationDirection m_currentAnimationDirection;
	AnimationState m_currentAnimationState;
	AnimationState m_previousAnimationState;
	float m_animationTimer = 0.0f;

	// Sprite;
	SpriteDefinition m_currentSpriteDefinition;
	Vec2 m_currentSpriteAnimationBottomLeftUV = Vec2(0.0f, 0.0f);
	Vec2 m_currentSpriteAnimationToptUV = Vec2(0.0f, 0.0f);

	Matrix4x4 m_capsuleModel;

	Prop* m_prop = nullptr;

	bool m_isTraining = false;
	float m_buildTimer = 0.0f;
	float m_maxBuildTime = 0.0f;

	// Pathing;
	PathCreation* m_pathCreation = nullptr;
	int m_pathPositionDestinationIndex = 0;
	Vec3 m_debug_tile = Vec3(0.0f, 0.0f, 0.0f);

	bool m_aiControlling = false;

	


};


