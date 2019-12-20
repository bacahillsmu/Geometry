#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Game/Gameplay/GameHandle.hpp"
#include "Game/Gameplay/Map/Map.hpp"



enum TaskStatus
{
	TASKSTATUS_START,
	TASKSTATUS_DO,
	TASKSTATUS_END
};

class EntityDefinition;

// -----------------------------------------------------------------------
class RTSTask
{

public:

	virtual ~RTSTask() {}
	virtual void Start(float deltaSeconds) = 0;
	virtual void DoTask(float deltaSeconds) = 0;
	virtual void End(Entity* entity) = 0;

	virtual RTSTask* Clone() = 0;

	void SetEntityCameraDirection(Entity* entity, Vec3 movingDirection);
	void CalculateAndUpdateStuckTimer(Entity* entity, float deltaSeconds);
	
public:

	TaskStatus m_taskStatus;
	Map* m_theMap = nullptr;
	GameHandle m_unit;

	// Figuring out if we are stuck;
	float m_stuckTimer = 0.0f;
	Vec3 m_lastPosition = Vec3(0.0f, 0.0f, 0.0f);
	int m_taskAttempts = 0;
	int m_taskAttemptThreshold = 10;
};

// -----------------------------------------------------------------------
class MoveTask : public RTSTask
{

public:

	virtual void Start(float deltaSeconds) override;
	virtual void DoTask(float deltaSeconds) override;
	virtual void End(Entity* entity) override;

	void PopulateEndTiles( std::vector<IntVec2>* endTiles );
	void CheckAndCalculatePath(Entity* entity, std::vector<IntVec2>& endTiles);
	void SetTargetPositionOnPath(Entity* entity);
	void MoveToTargetPosition(float deltaSeconds, Entity* entity);

	virtual RTSTask* Clone() override;

public:

	
	Vec3 m_targetPosition = Vec3(0.0f, 0.0f, 0.0f);
	GameHandle m_targetEntity = GameHandle::INVALID;

	// Calculated;
	Vec3 m_originalPosition = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 m_targetDestination = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 m_lastBestTargetDestination = Vec3(0.0f, 0.0f, 0.0f);
	
};

// -----------------------------------------------------------------------
class FollowTask : public RTSTask
{

public:

	virtual void Start(float deltaSeconds) override;
	virtual void DoTask(float deltaSeconds) override;
	virtual void End(Entity* entity) override;

	void PopulateEndTiles( std::vector<IntVec2>* endTiles );
	void CheckAndCalculatePath(Entity* entity, std::vector<IntVec2>& endTiles);
	void SetTargetPositionOnPath(Entity* entity);
	void MoveToTargetPosition(float deltaSeconds, Entity* entity);

	virtual RTSTask* Clone() override;

public:

	GameHandle m_entityToFollow;

	// Calculated;
	Vec3 m_entityToFollowLastPosition = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 m_originalPosition = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 m_targetDestination = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 m_lastBestTargetDestination = Vec3(0.0f, 0.0f, 0.0f);

};

// -----------------------------------------------------------------------
class AttackTask : public RTSTask
{

public:

	virtual void Start(float deltaSeconds) override;
	virtual void DoTask(float deltaSeconds) override;
	virtual void End(Entity* entity) override;

	void PopulateEndTiles( std::vector<IntVec2>* endTiles );
	void CheckAndCalculatePath(Entity* entity, std::vector<IntVec2>& endTiles);
	void SetTargetPositionOnPath(Entity* entity);
	void MoveToTargetPosition(float deltaSeconds, Entity* entity);


	virtual RTSTask* Clone() override;

public:

	GameHandle m_entityToAttack;

	// Calculated;
	Vec3 m_entityToAttackLastPosition = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 m_originalPosition = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 m_targetDestination = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 m_lastBestTargetDestination = Vec3(0.0f, 0.0f, 0.0f);

};

// -----------------------------------------------------------------------
class GatherTask : public RTSTask
{
public:

	virtual void Start(float deltaSeconds) override;
	virtual void DoTask(float deltaSeconds) override;
	virtual void End(Entity* entity) override;

	void PopulateEndTiles( std::vector<IntVec2>* endTiles, GameHandle destinationEntity );
	void CheckAndCalculatePath(Entity* entity, std::vector<IntVec2>& endTiles);
	void SetTargetPositionOnPath(Entity* entity);
	void MoveToTargetPosition(float deltaSeconds, Entity* entity, Entity* targetEntity, bool dropoffResource = false);

	virtual RTSTask* Clone() override;

public:

	GameHandle m_entityToGather;

	// Calculated;
	//GameHandle m_targetEntity = GameHandle::INVALID;
	bool m_returningWood = false;
	Vec3 m_originalPosition = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 m_targetDestination = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 m_lastBestTargetDestination = Vec3(0.0f, 0.0f, 0.0f);

};

// -----------------------------------------------------------------------
class RepairTask : public RTSTask
{
public:

	virtual void Start(float deltaSeconds) override;
	virtual void DoTask(float deltaSeconds) override;
	virtual void End(Entity* entity) override;

	virtual RTSTask* Clone() override;

public:

	GameHandle m_entityToRepair;


};

// -----------------------------------------------------------------------
class BuildTask : public RTSTask
{
public:

	virtual void Start(float deltaSeconds) override;
	virtual void DoTask(float deltaSeconds) override;
	virtual void End(Entity* entity) override;

	void PopulateEndTiles( std::vector<IntVec2>* endTiles );
	void CheckAndCalculatePath(Entity* entity, std::vector<IntVec2>& endTiles);
	void SetTargetPositionOnPath(Entity* entity);
	void MoveToTargetPosition(float deltaSeconds, Entity* entity);

	virtual RTSTask* Clone() override;

public:

	float m_percentHealth	= 1.0f;
	int m_team				= 0;
	EntityDefinition* m_entityDefToBuild = nullptr;
	Vec3 m_buildPosition = Vec3(0.0f, 0.0f, 0.0f);
	bool m_buildingCreated = false;

	// Calculated
	Vec3 m_originalPosition = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 m_targetDestination = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 m_lastBestTargetDestination = Vec3(0.0f, 0.0f, 0.0f);


};

// -----------------------------------------------------------------------
class DieTask : public RTSTask
{
public:

	virtual void Start(float deltaSeconds) override;
	virtual void DoTask(float deltaSeconds) override;
	virtual void End(Entity* entity) override;

	virtual RTSTask* Clone() override;

public:

	float m_deathTimer = 0.0f;
	float m_deathTimerThreshold = 3.0f;
	bool m_instantDeath = false;


};

// -----------------------------------------------------------------------
class TrainTask : public RTSTask
{
public:

	virtual void Start(float deltaSeconds) override;
	virtual void DoTask(float deltaSeconds) override;
	virtual void End(Entity* entity) override;

	virtual RTSTask* Clone() override;

public:

	EntityDefinition* m_entityDefinitionToTrain = nullptr;
	int m_team				= 0;
	Vec3 m_createPosition	= Vec3(0.0f, 0.0f, 0.0f);
	float m_trainTimer = 0.0f;


};

