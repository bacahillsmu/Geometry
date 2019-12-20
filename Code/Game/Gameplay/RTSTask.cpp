#include "Game/Gameplay/RTSTask.hpp"
#include "Game/Gameplay/Entity.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Gameplay/Camera/OrbitCamera.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Game/Gameplay/Input/ReplayController.hpp"
#include "Game/Gameplay/Input/PlayerController.hpp"
#include "Game/Gameplay/EntityDefinition.hpp"
#include "Game/Gameplay/RTSCommand.hpp"
#include "Game/Framework/Jobs/PathJob.hpp"

// -----------------------------------------------------------------------
// Move Task
// -----------------------------------------------------------------------
void MoveTask::Start(float deltaSeconds)
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		m_originalPosition = entity->m_position;
		delete entity->m_pathCreation;
		entity->m_pathCreation = nullptr;
		entity->SetTargetPosition(m_targetPosition);
		entity->SetAnimationState(ANIMATION_WALK);
	}

	DoTask(deltaSeconds);
}

// -----------------------------------------------------------------------
void MoveTask::DoTask(float deltaSeconds)
{
	m_taskStatus = TASKSTATUS_DO;

	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		// A way out if the entity continuously tries to get to target position but cannot;
		CalculateAndUpdateStuckTimer(entity, deltaSeconds);

		// Figure out if we are moving to a single point, or to a building with multiple ends;
		std::vector<IntVec2> endTiles;
		PopulateEndTiles(&endTiles);		

		// Move along path to end points;
		CheckAndCalculatePath(entity, endTiles);
		
		SetTargetPositionOnPath(entity);
		MoveToTargetPosition(deltaSeconds, entity);

	}	
}

// -----------------------------------------------------------------------
void MoveTask::End(Entity* entity)
{
	m_taskStatus = TASKSTATUS_END;

	delete entity->m_pathCreation;
	entity->m_pathCreation = nullptr;

	entity->SetAnimationState(ANIMATION_IDLE);
	entity->SetAnimationDirection(entity->m_previousAnimationDirection);
}

// BEFORE DOING THE PATHJOB
// -----------------------------------------------------------------------
// void MoveTask::CheckAndCalculatePath(Entity* entity, std::vector<IntVec2>& endTiles)
// {
// 	// If I am trying to move without a path...
// 	if (!entity->m_pathCreation || entity->m_pathCreation->m_pathCreationTimeStamp < m_theMap->m_occupancyMapChangeTimeStamp)
// 	{
// 		m_originalPosition = entity->m_position;
// 		delete entity->m_pathCreation;
// 		entity->m_pathCreation = nullptr;
// 
// 		IntVec2 startTile = m_theMap->GetTileCoordFromPosition(m_originalPosition);
// 
// 		// ... Create a path;
// 		entity->m_pathCreation = m_theMap->m_pather.CreatePath(startTile, endTiles, m_theMap->m_tileDimensions);
// 		entity->m_pathCreation->m_path->pop_back();
// 		std::reverse(entity->m_pathCreation->m_path->begin(), entity->m_pathCreation->m_path->end());
// 
// 		entity->m_pathPositionDestinationIndex = 0;
// 	}
// }

// -----------------------------------------------------------------------
void MoveTask::CheckAndCalculatePath(Entity* entity, std::vector<IntVec2>& endTiles)
{
	// If I am trying to move without a path...
	if(!entity->m_pathCreation || entity->m_pathCreation->m_pathCreationTimeStamp < m_theMap->m_occupancyMapChangeTimeStamp)
	{
		if(!entity->m_waitingOnPathJob)
		{
			entity->m_waitingOnPathJob = true;
			PathJob* pathJob = new PathJob();
			
			m_originalPosition = entity->m_position;
			delete entity->m_pathCreation;
			entity->m_pathCreation = nullptr;

			//pathJob->m_theMap = m_theMap;
			pathJob->m_tileDimensions = m_theMap->m_tileDimensions;

#pragma region Pather
			pathJob->m_pather = new Pather();
			pathJob->m_pather->Init(pathJob->m_tileDimensions, 1.0f);
			IntVec2 tile = m_theMap->GetTileCoordFromPosition(entity->m_position);
			IntVec2 offset = entity->m_entityDefinition->m_offset;
			tile.x -= (int)offset.x;
			tile.y -= (int)offset.y;

			for (int y = 0; y < entity->m_entityDefinition->m_occupancySize.y; y++)
			{
				for (int x = 0; x < entity->m_entityDefinition->m_occupancySize.x; x++)
				{
					IntVec2 occupiedTile = IntVec2(0, 0);
					occupiedTile.x = tile.x + x;
					occupiedTile.y = tile.y + y;
					int tileIndex = GetIndexFromCoord(occupiedTile, pathJob->m_tileDimensions);

					m_theMap->SetOccupiedTileIndexStatus(tileIndex, 1);
					pathJob->m_pather->SetCostTile(tileIndex, 10000.0f);
				}
			}
#pragma endregion

			pathJob->m_startTile = m_theMap->GetTileCoordFromPosition(m_originalPosition);
			pathJob->m_endTiles = endTiles;

			pathJob->SetFinishCallback([=](Job* job)
				{ 
					entity->ApplyPath((PathJob*)job);
				});

			g_theJobSystem->Run(pathJob);

			entity->m_pathPositionDestinationIndex = 0;
		}
	}
}

// -----------------------------------------------------------------------
void MoveTask::SetTargetPositionOnPath(Entity* entity)
{
	if(!entity->m_pathCreation)
	{
		return;
	}

	Path* path = entity->m_pathCreation->m_path;
	
	IntVec2 currentTile = m_theMap->GetTileCoordFromPosition(entity->m_position);
	IntVec2 finalTile = path->back();
	IntVec2 aggressiveTile;
	int aggressiveTileDestinationindex = (int)path->size();
	
	if(currentTile == finalTile)
	{
		m_targetDestination = entity->m_targetPosition;
	}
	else
	{
		// Try to bump the destinationTile
		float raycast = 0.0f;
		while(raycast != 1.0f)
		{
			aggressiveTileDestinationindex--;
			if(aggressiveTileDestinationindex < 0)
			{
				m_targetDestination = m_lastBestTargetDestination;
				entity->m_debug_tile = m_targetDestination;
				break;
			}
			aggressiveTile = (*path)[aggressiveTileDestinationindex];
			raycast = m_theMap->GridRaycast(Vec2(entity->m_position.x, entity->m_position.y), Vec2((float)aggressiveTile.x, (float)aggressiveTile.y));
			m_targetDestination = Vec3((float)aggressiveTile.x, (float)aggressiveTile.y, m_targetPosition.z);
			entity->m_debug_tile = m_targetDestination;
		}

		m_lastBestTargetDestination = m_targetDestination;
	}
}

// -----------------------------------------------------------------------
void MoveTask::PopulateEndTiles( std::vector<IntVec2>* endTiles )
{
	if(m_targetEntity != GameHandle::INVALID)
	{
		Entity* targetEntity = m_theMap->FindEntity(m_targetEntity);

		IntVec2 tile = m_theMap->GetTileCoordFromPosition(targetEntity->m_position);
		IntVec2 offset = targetEntity->m_entityDefinition->m_offset;
		tile.x -= (int)offset.x;
		tile.y -= (int)offset.y;

		for(int y = 0; y < targetEntity->m_entityDefinition->m_occupancySize.y; y++)
		{
			for(int x = 0; x < targetEntity->m_entityDefinition->m_occupancySize.x; x++)
			{
				IntVec2 occupiedTile = IntVec2(0, 0);
				occupiedTile.x = tile.x + x;
				occupiedTile.y = tile.y + y;
				endTiles->push_back(occupiedTile);
			}
		}
	}
	else
	{
		IntVec2 endTile = m_theMap->GetTileCoordFromPosition(m_targetPosition);
		endTiles->push_back(endTile);
	}
}

// -----------------------------------------------------------------------
void MoveTask::MoveToTargetPosition(float deltaSeconds, Entity* entity)
{
	Vec3 displacment = m_targetDestination - entity->m_position;
	SetEntityCameraDirection(entity, m_targetDestination);

	if( !entity->m_waitingOnPathJob && ( displacment.GetLength() <= entity->m_speed * deltaSeconds) )
	{
		entity->m_position = m_targetDestination;

		End(entity);
		return;
	}
	else
	{
		entity->m_position += entity->m_facingDirection * entity->m_speed * deltaSeconds;
	}
}

// -----------------------------------------------------------------------
RTSTask* MoveTask::Clone()
{
	return new MoveTask(*this);
}

// -----------------------------------------------------------------------
// Follow Task
// -----------------------------------------------------------------------
void FollowTask::Start( float deltaSeconds )
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		Entity* entityToFollow = m_theMap->FindEntity(m_entityToFollow);
		if(entityToFollow)
		{
			m_originalPosition = entity->m_position;
			delete entity->m_pathCreation;
			entity->m_pathCreation = nullptr;
			
			entity->SetAnimationState(ANIMATION_WALK);

			DoTask(deltaSeconds);
		}
		else
		{
			End(entity);
			return;
		}		
	}	
}

// -----------------------------------------------------------------------
void FollowTask::DoTask( float deltaSeconds )
{
	m_taskStatus = TASKSTATUS_DO;

	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		Entity* entityToFollow = m_theMap->FindEntity(m_entityToFollow);
		if(entityToFollow)
		{
			entity->SetTargetPosition(entityToFollow->m_position);

			CalculateAndUpdateStuckTimer(entity, deltaSeconds);

			std::vector<IntVec2> endTiles;
			PopulateEndTiles(&endTiles);

			// Move along path to end points;
			CheckAndCalculatePath(entity, endTiles);
			SetTargetPositionOnPath(entity);
			MoveToTargetPosition(deltaSeconds, entity);

			m_entityToFollowLastPosition = entityToFollow->m_position;
		}
		else
		{
			End(entity);
			return;
		}
	}
}

// -----------------------------------------------------------------------
void FollowTask::End( Entity* entity )
{
	m_taskStatus = TASKSTATUS_END;

	delete entity->m_pathCreation;
	entity->m_pathCreation = nullptr;

	entity->SetAnimationState(ANIMATION_IDLE);
	entity->SetAnimationDirection(entity->m_previousAnimationDirection);
}

// -----------------------------------------------------------------------
void FollowTask::PopulateEndTiles( std::vector<IntVec2>* endTiles )
{
	if(m_entityToFollow != GameHandle::INVALID)
	{
		Entity* targetEntity = m_theMap->FindEntity(m_entityToFollow);

		if(targetEntity->m_prop)
		{
			IntVec2 tile = m_theMap->GetTileCoordFromPosition(targetEntity->m_position);
			IntVec2 offset = targetEntity->m_entityDefinition->m_offset;
			tile.x -= (int)offset.x;
			tile.y -= (int)offset.y;

			for(int y = 0; y < targetEntity->m_entityDefinition->m_occupancySize.y; y++)
			{
				for(int x = 0; x < targetEntity->m_entityDefinition->m_occupancySize.x; x++)
				{
					IntVec2 occupiedTile = IntVec2(0, 0);
					occupiedTile.x = tile.x + x;
					occupiedTile.y = tile.y + y;
					endTiles->push_back(occupiedTile);
				}
			}
		}
		else
		{
			IntVec2 endTile = m_theMap->GetTileCoordFromPosition(targetEntity->m_position);
			endTiles->push_back(endTile);
		}
	}
}

// -----------------------------------------------------------------------
void FollowTask::CheckAndCalculatePath( Entity* entity, std::vector<IntVec2>& endTiles )
{
	Entity* entityToFollow = m_theMap->FindEntity(m_entityToFollow);

	// If I am trying to move without a path...
	if(!entity->m_pathCreation || entity->m_pathCreation->m_pathCreationTimeStamp < m_theMap->m_occupancyMapChangeTimeStamp || entityToFollow->m_position != m_entityToFollowLastPosition)
	{
		m_originalPosition = entity->m_position;
		delete entity->m_pathCreation;
		entity->m_pathCreation = nullptr;

		IntVec2 startTile = m_theMap->GetTileCoordFromPosition(m_originalPosition);

		// ... Create a path;
		entity->m_pathCreation = m_theMap->m_pather.CreatePath(startTile, endTiles, m_theMap->m_tileDimensions);
		entity->m_pathCreation->m_path->pop_back();
		std::reverse(entity->m_pathCreation->m_path->begin(), entity->m_pathCreation->m_path->end());

		entity->m_pathPositionDestinationIndex = 0;
	}
}

// -----------------------------------------------------------------------
void FollowTask::SetTargetPositionOnPath( Entity* entity )
{
	Path* path = entity->m_pathCreation->m_path;

	IntVec2 currentTile = m_theMap->GetTileCoordFromPosition(entity->m_position);
	IntVec2 finalTile = path->back();
	IntVec2 aggressiveTile;
	int aggressiveTileDestinationindex = (int)path->size();

	if(currentTile == finalTile)
	{
		m_targetDestination = entity->m_targetPosition;
	}
	else
	{
		// Try to bump the destinationTile
		float raycast = 0.0f;
		while(raycast != 1.0f)
		{
			aggressiveTileDestinationindex--;
			if(aggressiveTileDestinationindex < 0)
			{
				m_targetDestination = m_lastBestTargetDestination;
				entity->m_debug_tile = m_targetDestination;
				break;
			}
			aggressiveTile = (*path)[aggressiveTileDestinationindex];
			raycast = m_theMap->GridRaycast(Vec2(entity->m_position.x, entity->m_position.y), Vec2((float)aggressiveTile.x, (float)aggressiveTile.y));
			m_targetDestination = Vec3((float)aggressiveTile.x, (float)aggressiveTile.y, entity->m_position.z);
			entity->m_debug_tile = m_targetDestination;
		}

		m_lastBestTargetDestination = m_targetDestination;
	}
}

// -----------------------------------------------------------------------
void FollowTask::MoveToTargetPosition( float deltaSeconds, Entity* entity )
{
	Entity* entityToFollow = m_theMap->FindEntity(m_entityToFollow);

	SetEntityCameraDirection(entity, m_targetDestination);

	float distance = GetDistance(entity->m_position, entityToFollow->m_position);
	if( distance <= ((entity->m_physicsRadius + entityToFollow->m_physicsRadius + 0.1f)) )
	{
		entity->SetAnimationState(ANIMATION_IDLE);
		entity->SetAnimationDirection(entity->m_previousAnimationDirection);
		m_stuckTimer = 0.0f;
	}
	else
	{
		entity->SetAnimationState(ANIMATION_WALK);
		entity->m_position += entity->m_facingDirection * entity->m_speed * deltaSeconds;
	}
}

// -----------------------------------------------------------------------
RTSTask* FollowTask::Clone()
{
	return new FollowTask(*this);
}

// -----------------------------------------------------------------------
// Attack Task
// -----------------------------------------------------------------------
void AttackTask::Start( float deltaSeconds )
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		Entity* entityToAttack = m_theMap->FindEntity(m_entityToAttack);
		if(entityToAttack)
		{
			m_originalPosition = entity->m_position;
			delete entity->m_pathCreation;
			entity->m_pathCreation = nullptr;

			entity->SetAnimationState(ANIMATION_WALK);

			DoTask(deltaSeconds);
		}
		else
		{
			End(entity);
			return;
		}		
	}
}

// -----------------------------------------------------------------------
void AttackTask::DoTask( float deltaSeconds )
{
	m_taskStatus = TASKSTATUS_DO;

	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		Entity* entityToAttack = m_theMap->FindEntity(m_entityToAttack);
		entityToAttack = entityToAttack->m_isDead ? nullptr : entityToAttack;
		entity->m_entityToAttack = entityToAttack;
		if(entityToAttack)
		{
			entity->SetTargetPosition(entityToAttack->m_position);

			CalculateAndUpdateStuckTimer(entity, deltaSeconds);

			std::vector<IntVec2> endTiles;
			PopulateEndTiles(&endTiles);

			// Move along path to end points;
			CheckAndCalculatePath(entity, endTiles);
			SetTargetPositionOnPath(entity);
			MoveToTargetPosition(deltaSeconds, entity);

			m_entityToAttackLastPosition = entityToAttack->m_position;
		}
		else
		{
			End(entity);
			return;
		}
	}
}

// -----------------------------------------------------------------------
void AttackTask::End( Entity* entity )
{
	m_taskStatus = TASKSTATUS_END;

	delete entity->m_pathCreation;
	entity->m_pathCreation = nullptr;

	entity->SetAnimationState(ANIMATION_IDLE);
	entity->SetAnimationDirection(entity->m_previousAnimationDirection);
}

// -----------------------------------------------------------------------
void AttackTask::PopulateEndTiles( std::vector<IntVec2>* endTiles )
{
	if(m_entityToAttack != GameHandle::INVALID)
	{
		Entity* targetEntity = m_theMap->FindEntity(m_entityToAttack);

		if(targetEntity->m_prop)
		{
			IntVec2 tile = m_theMap->GetTileCoordFromPosition(targetEntity->m_position);
			IntVec2 offset = targetEntity->m_entityDefinition->m_offset;
			tile.x -= (int)offset.x;
			tile.y -= (int)offset.y;

			for(int y = 0; y < targetEntity->m_entityDefinition->m_occupancySize.y; y++)
			{
				for(int x = 0; x < targetEntity->m_entityDefinition->m_occupancySize.x; x++)
				{
					IntVec2 occupiedTile = IntVec2(0, 0);
					occupiedTile.x = tile.x + x;
					occupiedTile.y = tile.y + y;
					endTiles->push_back(occupiedTile);
				}
			}
		}
		else
		{
			IntVec2 endTile = m_theMap->GetTileCoordFromPosition(targetEntity->m_position);
			endTiles->push_back(endTile);
		}
	}
}

// -----------------------------------------------------------------------
void AttackTask::CheckAndCalculatePath( Entity* entity, std::vector<IntVec2>& endTiles )
{
	Entity* entityToAttack = m_theMap->FindEntity(m_entityToAttack);

	// If I am trying to move without a path...
	if(!entity->m_pathCreation || entity->m_pathCreation->m_pathCreationTimeStamp < m_theMap->m_occupancyMapChangeTimeStamp || entityToAttack->m_position != m_entityToAttackLastPosition)
	{
		m_originalPosition = entity->m_position;
		delete entity->m_pathCreation;
		entity->m_pathCreation = nullptr;

		IntVec2 startTile = m_theMap->GetTileCoordFromPosition(m_originalPosition);

		// ... Create a path;
		entity->m_pathCreation = m_theMap->m_pather.CreatePath(startTile, endTiles, m_theMap->m_tileDimensions);
		entity->m_pathCreation->m_path->pop_back();
		std::reverse(entity->m_pathCreation->m_path->begin(), entity->m_pathCreation->m_path->end());

		entity->m_pathPositionDestinationIndex = 0;
	}
}

// -----------------------------------------------------------------------
void AttackTask::SetTargetPositionOnPath( Entity* entity )
{
	Path* path = entity->m_pathCreation->m_path;

	IntVec2 currentTile = m_theMap->GetTileCoordFromPosition(entity->m_position);
	IntVec2 finalTile = path->back();
	IntVec2 aggressiveTile;
	int aggressiveTileDestinationindex = (int)path->size();

	if(currentTile == finalTile)
	{
		m_targetDestination = entity->m_targetPosition;
	}
	else
	{
		// Try to bump the destinationTile
		float raycast = 0.0f;
		while(raycast != 1.0f)
		{
			aggressiveTileDestinationindex--;
			if(aggressiveTileDestinationindex < 0)
			{
				m_targetDestination = m_lastBestTargetDestination;
				entity->m_debug_tile = m_targetDestination;
				break;
			}
			aggressiveTile = (*path)[aggressiveTileDestinationindex];
			raycast = m_theMap->GridRaycast(Vec2(entity->m_position.x, entity->m_position.y), Vec2((float)aggressiveTile.x, (float)aggressiveTile.y));
			m_targetDestination = Vec3((float)aggressiveTile.x, (float)aggressiveTile.y, entity->m_position.z);
			entity->m_debug_tile = m_targetDestination;
		}

		m_lastBestTargetDestination = m_targetDestination;
	}
}

// -----------------------------------------------------------------------
void AttackTask::MoveToTargetPosition( float deltaSeconds, Entity* entity )
{
	Entity* entityToAttack = m_theMap->FindEntity(m_entityToAttack);

	SetEntityCameraDirection(entity, m_targetDestination);

	float distance = GetDistance(entity->m_position, entityToAttack->m_position);
	if( distance <= (entity->m_physicsRadius + entityToAttack->m_physicsRadius + entity->m_range) )
	{
		entity->SetAnimationState(ANIMATION_ATTACK);
		entity->SetAnimationDirection(entity->m_previousAnimationDirection);
		m_stuckTimer = 0.0f;
	}
	else
	{
		entity->SetAnimationState(ANIMATION_WALK);
		entity->m_position += entity->m_facingDirection * entity->m_speed * deltaSeconds;
	}
}

// -----------------------------------------------------------------------
RTSTask* AttackTask::Clone()
{
	return new AttackTask(*this);
}

// -----------------------------------------------------------------------
// Gather Task
// -----------------------------------------------------------------------
void GatherTask::Start( float deltaSeconds )
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		Entity* entityToGather = m_theMap->FindEntity(m_entityToGather);
		if(entityToGather)
		{
			m_originalPosition = entity->m_position;
			delete entity->m_pathCreation;
			entity->m_pathCreation = nullptr;

			entity->SetTargetPosition(entityToGather->m_position);
			entity->SetAnimationState(ANIMATION_WALK);

			DoTask(deltaSeconds);
		}
		else
		{
			End(entity);
			return;
		}		
	}
}

// -----------------------------------------------------------------------
void GatherTask::DoTask( float deltaSeconds )
{
	m_taskStatus = TASKSTATUS_DO;
	
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		CalculateAndUpdateStuckTimer(entity, deltaSeconds);

		// Return gathered Wood;
		if(entity->m_woodAmount >= entity->m_maxWoodAmount)
		{
			if(!m_returningWood)
			{
				delete entity->m_pathCreation;
				entity->m_pathCreation = nullptr;
				m_returningWood = true;
			}

			Entity* townHall = entity->FindNearestResourceHubOnTeam(entity->GetCurrentTeam());
			if(townHall)
			{
				entity->SetTargetPosition(townHall->m_position);

				std::vector<IntVec2> endTiles;
				PopulateEndTiles(&endTiles, townHall->GetGameHandle());

				CheckAndCalculatePath(entity, endTiles);
				SetTargetPositionOnPath(entity);
				MoveToTargetPosition(deltaSeconds, entity, townHall, true);
			}
		}
		// Go to EntityToGather;
		else
		{
			Entity* entityToGather = m_theMap->FindEntity(m_entityToGather);
			if(entityToGather && !entityToGather->m_isDead)
			{
				entity->m_entityToGather = entityToGather;
				entity->SetTargetPosition(entityToGather->m_position);
				
				std::vector<IntVec2> endTiles;
				PopulateEndTiles(&endTiles, entityToGather->GetGameHandle());

				CheckAndCalculatePath(entity, endTiles);
				SetTargetPositionOnPath(entity);
				MoveToTargetPosition(deltaSeconds, entity, entityToGather);
			}
			else
			{
				// Find another tree to gather from?
				Entity* newTree = entity->FindNearestTree();
				if(newTree)
				{
					m_entityToGather = newTree->GetGameHandle();
					entity->SetTargetPosition(newTree->m_position);
				}
				else
				{
					if(m_taskAttempts > m_taskAttemptThreshold)
					{
						End(entity);
						return;
					}
					else
					{
						m_taskAttempts++;
					}					
				}				
			}
		}
	}
}

// -----------------------------------------------------------------------
void GatherTask::End( Entity* entity )
{
	m_taskStatus = TASKSTATUS_END;
	entity->m_entityToGather = nullptr;

	delete entity->m_pathCreation;
	entity->m_pathCreation = nullptr;

	entity->SetAnimationState(ANIMATION_IDLE);
	entity->SetAnimationDirection(entity->m_previousAnimationDirection);
}

// -----------------------------------------------------------------------
void GatherTask::PopulateEndTiles( std::vector<IntVec2>* endTiles, GameHandle destinationEntity )
{
	if(destinationEntity != GameHandle::INVALID)
	{
		Entity* targetEntity = m_theMap->FindEntity(destinationEntity);

		IntVec2 tile = m_theMap->GetTileCoordFromPosition(targetEntity->m_position);
		IntVec2 offset = targetEntity->m_entityDefinition->m_offset;
		tile.x -= (int)offset.x;
		tile.y -= (int)offset.y;

		for(int y = 0; y < targetEntity->m_entityDefinition->m_occupancySize.y; y++)
		{
			for(int x = 0; x < targetEntity->m_entityDefinition->m_occupancySize.x; x++)
			{
				IntVec2 occupiedTile = IntVec2(0, 0);
				occupiedTile.x = tile.x + x;
				occupiedTile.y = tile.y + y;
				endTiles->push_back(occupiedTile);
			}
		}
	}
}

// -----------------------------------------------------------------------
void GatherTask::CheckAndCalculatePath( Entity* entity, std::vector<IntVec2>& endTiles )
{
	// If I am trying to move without a path...
	if(!entity->m_pathCreation || entity->m_pathCreation->m_pathCreationTimeStamp < m_theMap->m_occupancyMapChangeTimeStamp)
	{
		m_originalPosition = entity->m_position;
		delete entity->m_pathCreation;
		entity->m_pathCreation = nullptr;

		IntVec2 startTile = m_theMap->GetTileCoordFromPosition(m_originalPosition);

		// ... Create a path;
		entity->m_pathCreation = m_theMap->m_pather.CreatePath(startTile, endTiles, m_theMap->m_tileDimensions);
		entity->m_pathCreation->m_path->pop_back();
		std::reverse(entity->m_pathCreation->m_path->begin(), entity->m_pathCreation->m_path->end());

		entity->m_pathPositionDestinationIndex = 0;
	}
}

void GatherTask::SetTargetPositionOnPath( Entity* entity )
{
	Path* path = entity->m_pathCreation->m_path;

	IntVec2 currentTile = m_theMap->GetTileCoordFromPosition(entity->m_position);
	IntVec2 finalTile = path->back();
	IntVec2 aggressiveTile;
	int aggressiveTileDestinationindex = (int)path->size();

	if(currentTile == finalTile)
	{
		m_targetDestination = entity->m_targetPosition;
	}
	else
	{
		// Try to bump the destinationTile
		float raycast = 0.0f;
		while(raycast != 1.0f)
		{
			aggressiveTileDestinationindex--;
			if(aggressiveTileDestinationindex < 0)
			{
				m_targetDestination = m_lastBestTargetDestination;
				entity->m_debug_tile = m_targetDestination;
				break;
			}
			aggressiveTile = (*path)[aggressiveTileDestinationindex];
			raycast = m_theMap->GridRaycast(Vec2(entity->m_position.x, entity->m_position.y), Vec2((float)aggressiveTile.x, (float)aggressiveTile.y));
			m_targetDestination = Vec3((float)aggressiveTile.x, (float)aggressiveTile.y, entity->m_position.z);
			entity->m_debug_tile = m_targetDestination;
		}

		m_lastBestTargetDestination = m_targetDestination;
	}
}

// -----------------------------------------------------------------------
void GatherTask::MoveToTargetPosition( float deltaSeconds, Entity* entity, Entity* targetEntity, bool dropoffResource /*= false*/ )
{
	SetEntityCameraDirection(entity, m_targetDestination);

	float distance = GetDistance(entity->m_position, targetEntity->m_targetPosition);
	if( distance <= (entity->m_physicsRadius + targetEntity->m_physicsRadius + (entity->m_range * 3.0f)) )
	{
		entity->SetAnimationState(ANIMATION_ATTACK);
		entity->SetAnimationDirection(entity->m_previousAnimationDirection);
		m_stuckTimer = 0.0f;

		if(dropoffResource)
		{
			m_theMap->AddWoodToTeam(entity->m_woodAmount, entity->m_team);
			entity->m_woodAmount = 0;
			delete entity->m_pathCreation;
			entity->m_pathCreation = nullptr;
			m_returningWood = false;
		}
	}
	else
	{
		entity->SetAnimationState(ANIMATION_WALK);
		entity->m_position += entity->m_facingDirection * entity->m_speed * deltaSeconds;
	}
}

// -----------------------------------------------------------------------
RTSTask* GatherTask::Clone()
{
	return new GatherTask(*this);
}

// -----------------------------------------------------------------------
// BuildTask
// -----------------------------------------------------------------------
void BuildTask::Start( float deltaSeconds )
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		m_originalPosition = entity->m_position;
		delete entity->m_pathCreation;
		entity->m_pathCreation = nullptr;

		entity->SetTargetPosition(m_buildPosition);
		entity->SetAnimationState(ANIMATION_WALK);

		DoTask(deltaSeconds);		
	}
}

// -----------------------------------------------------------------------
void BuildTask::DoTask( float deltaSeconds )
{
	m_taskStatus = TASKSTATUS_DO;

	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{

		// A way out if the entity continuously tries to get to target position but cannot;
		CalculateAndUpdateStuckTimer(entity, deltaSeconds);

		// Figure out if we are moving to a single point, or to a building with multiple ends;
		std::vector<IntVec2> endTiles;
		PopulateEndTiles(&endTiles);		

		// Move along path to end points;
		CheckAndCalculatePath(entity, endTiles);
		SetTargetPositionOnPath(entity);
		MoveToTargetPosition(deltaSeconds, entity);
	}
}

// -----------------------------------------------------------------------
void BuildTask::End( Entity* entity )
{
	m_taskStatus = TASKSTATUS_END;

	delete entity->m_pathCreation;
	entity->m_pathCreation = nullptr;

	entity->SetAnimationState(ANIMATION_IDLE);
	entity->SetAnimationDirection(entity->m_previousAnimationDirection);
}

// -----------------------------------------------------------------------
void BuildTask::PopulateEndTiles( std::vector<IntVec2>* endTiles )
{
	IntVec2 endTile = m_theMap->GetTileCoordFromPosition(m_buildPosition);
	endTiles->push_back(endTile);
}

// -----------------------------------------------------------------------
void BuildTask::CheckAndCalculatePath( Entity* entity, std::vector<IntVec2>& endTiles )
{
	// If I am trying to move without a path...
	if(!entity->m_pathCreation || entity->m_pathCreation->m_pathCreationTimeStamp < m_theMap->m_occupancyMapChangeTimeStamp)
	{
		m_originalPosition = entity->m_position;
		delete entity->m_pathCreation;
		entity->m_pathCreation = nullptr;

		IntVec2 startTile = m_theMap->GetTileCoordFromPosition(m_originalPosition);

		// ... Create a path;
		entity->m_pathCreation = m_theMap->m_pather.CreatePath(startTile, endTiles, m_theMap->m_tileDimensions);
		entity->m_pathCreation->m_path->pop_back();
		std::reverse(entity->m_pathCreation->m_path->begin(), entity->m_pathCreation->m_path->end());

		entity->m_pathPositionDestinationIndex = 0;
	}
}

// -----------------------------------------------------------------------
void BuildTask::SetTargetPositionOnPath( Entity* entity )
{
	Path* path = entity->m_pathCreation->m_path;

	IntVec2 currentTile = m_theMap->GetTileCoordFromPosition(entity->m_position);
	IntVec2 finalTile = path->back();
	IntVec2 aggressiveTile;
	int aggressiveTileDestinationindex = (int)path->size();

	if(currentTile == finalTile)
	{
		m_targetDestination = entity->m_targetPosition;
	}
	else
	{
		// Try to bump the destinationTile
		float raycast = 0.0f;
		while(raycast != 1.0f)
		{
			aggressiveTileDestinationindex--;
			if(aggressiveTileDestinationindex < 0)
			{
				m_targetDestination = m_lastBestTargetDestination;
				entity->m_debug_tile = m_targetDestination;
				break;
			}
			aggressiveTile = (*path)[aggressiveTileDestinationindex];
			raycast = m_theMap->GridRaycast(Vec2(entity->m_position.x, entity->m_position.y), Vec2((float)aggressiveTile.x, (float)aggressiveTile.y));
			m_targetDestination = Vec3((float)aggressiveTile.x, (float)aggressiveTile.y, m_buildPosition.z);
			entity->m_debug_tile = m_targetDestination;
		}

		m_lastBestTargetDestination = m_targetDestination;
	}
}

// -----------------------------------------------------------------------
void BuildTask::MoveToTargetPosition( float deltaSeconds, Entity* entity )
{
	SetEntityCameraDirection(entity, m_targetDestination);

	float distance = GetDistance(entity->m_position, m_buildPosition);
	if(distance < (entity->m_physicsRadius + m_entityDefToBuild->m_physicsRadius + entity->m_range))
	{
		if(!m_buildingCreated)
		{
			Entity* building = new Entity(g_theApp->m_theGame, m_entityDefToBuild->m_entityType.c_str());
			building->m_position		= m_buildPosition;
			building->m_targetPosition	= m_buildPosition;
			building->m_health			= 0.1f * building->m_maxHealth;
			building->m_justConstructed = true;
			building->m_team			= m_team;
			building->m_teamColor		= m_team == 0 ? Rgba(0.68f, 0.85f, 0.9f, 1.0f) : Rgba(1.0f, 0.58f, 0.47f, 1.0f);
			m_theMap->DelayAddEntity(building);

			m_buildingCreated = true;
			g_thePlayerController->m_placedPlaceHolderProp = nullptr;
			g_thePlayerController->m_placedPlaceHolderDefinition = nullptr;
			entity->SetAnimationState(ANIMATION_IDLE);
		}

		entity->SetAnimationDirection(entity->m_previousAnimationDirection);

		// Delay then repair;
		Entity* building = entity->FindNearestDamagedBuildingOnTeam(entity->GetCurrentTeam());
		if(building)
		{
			std::map< std::string, RTSTaskInfo* >::iterator found = entity->m_entityDefinition->m_availableTasks.find("repair");
			if(found != entity->m_entityDefinition->m_availableTasks.end())
			{
				RTSTaskInfo* rtsTaskInfo = found->second;
				RepairTask* repairTask = static_cast<RepairTask*>(rtsTaskInfo->rtsTask->Clone());
				repairTask->m_taskStatus = TASKSTATUS_START;
				repairTask->m_unit = m_unit;
				repairTask->m_theMap = m_theMap;
				repairTask->m_entityToRepair = building->GetGameHandle();

				entity->AddTask(repairTask);
			}
		}
		else
		{
			m_taskAttempts++;
		}

		if(m_taskAttempts > m_taskAttemptThreshold)
		{
			End(entity);
			return;
		}

	}
	else
	{
		entity->SetAnimationState(ANIMATION_WALK);
		entity->m_position += entity->m_facingDirection * entity->m_speed * deltaSeconds;
	}
}

// -----------------------------------------------------------------------
RTSTask* BuildTask::Clone()
{
	return new BuildTask(*this);
}

// -----------------------------------------------------------------------
// Die Task
// -----------------------------------------------------------------------
void DieTask::Start( float deltaSeconds )
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		entity->SetAnimationState(ANIMATION_DIE);

		SoundID testSound = g_theAudioSystem->CreateOrGetSound( "Data/Audio/pain.wav" );
		ChannelGroupID sfxGroup = g_theAudioSystem->CreateOrGetChannelGroup("SFX");
		g_theAudioSystem->Set3DListener(entity->m_theGame->GetGameCamera());
		g_theAudioSystem->Play3DSound( testSound, sfxGroup, entity->m_position );
	}

	DoTask(deltaSeconds);
}

// -----------------------------------------------------------------------
void DieTask::DoTask( float deltaSeconds )
{
	m_taskStatus = TASKSTATUS_DO;

	Entity* entity = m_theMap->FindEntity(m_unit);

	if(m_instantDeath)
	{
		End(entity);
		return;
	}
	else
	{
		m_deathTimer += deltaSeconds;
		if(m_deathTimer > m_deathTimerThreshold)
		{
			End(entity);
			return;
		}
	}
}

// -----------------------------------------------------------------------
void DieTask::End( Entity* entity )
{
	UNUSED(entity);

	m_taskStatus = TASKSTATUS_END;
	entity->m_isValid = false;
}

// -----------------------------------------------------------------------
RTSTask* DieTask::Clone()
{
	return new DieTask(*this);
}

// -----------------------------------------------------------------------
void RTSTask::SetEntityCameraDirection( Entity* entity, Vec3 movingDirection )
{
	Vec3 displacment = movingDirection - entity->m_position;
	entity->m_facingDirection = displacment.GetNormalized();

	// Transform by the Camera's View Matrix
	Matrix4x4 viewMatrix = g_theApp->m_theGame->GetGameCamera()->GetViewMatrix();
	Vec3 dir = entity->m_facingDirection;
	dir = viewMatrix.TransformVector3D(dir);
	Vec2 ddir = Vec2(dir.x, dir.z);
	ddir.Normalize();

	uint bestIndex = 0;
	float bestValue = DotProductVec2(ddir, entity->m_directions[0]);
	float value;

	for(uint i = 1; i < entity->m_directions.size(); i++)
	{
		value = DotProductVec2(ddir, entity->m_directions[i]);
		if(value > bestValue)
		{
			bestIndex = i;
			bestValue = value;
		}
	}

	entity->SetAnimationDirection((AnimationDirection)bestIndex);
}

// -----------------------------------------------------------------------
void RTSTask::CalculateAndUpdateStuckTimer( Entity* entity, float deltaSeconds )
{
	Vec3 oldPosition = m_lastPosition;
	m_lastPosition = entity->m_position;

	float distance = GetDistance(oldPosition, m_lastPosition);
	float distanceThreshold = (entity->m_speed * deltaSeconds) / 2.0f;
	if(distance < distanceThreshold)
	{
		m_stuckTimer += deltaSeconds;
	}

	if(m_stuckTimer >= 3.0f)
	{
		End(entity);
		return;
	}
}

// -----------------------------------------------------------------------
// Repair Task
// -----------------------------------------------------------------------
void RepairTask::Start( float deltaSeconds )
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		Entity* entityToRepair = m_theMap->FindEntity(m_entityToRepair);
		if(entityToRepair)
		{
			entity->SetTargetPosition(entityToRepair->m_position);
			entity->SetAnimationState(ANIMATION_WALK);

			DoTask(deltaSeconds);
		}
		else
		{
			End(entity);
			return;
		}		
	}
}

// -----------------------------------------------------------------------
void RepairTask::DoTask( float deltaSeconds )
{
	m_taskStatus = TASKSTATUS_DO;

	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		Entity* entityToRepair = m_theMap->FindEntity(m_entityToRepair);
		if(entityToRepair)
		{
			if(entityToRepair->m_health >= entityToRepair->m_maxHealth)
			{
				entityToRepair->m_buildingFinishedConstruction = true;
				End(entity);
				return;
			}

			entity->m_entityToRepair = entityToRepair;
			entity->m_targetPosition = entityToRepair->m_position;
			SetEntityCameraDirection(entity, entity->m_targetPosition);

			// Check how far we are to entityToRepair
			float distance = GetDistance(entity->m_position, entityToRepair->m_position);
			if(distance < (entity->m_physicsRadius + entityToRepair->m_physicsRadius + entity->m_range * 2.0f))
			{
				//entityToRepair->m_health += healthPerDeltaSecond;
				entity->SetAnimationState(ANIMATION_ATTACK);
				entity->SetAnimationDirection(entity->m_previousAnimationDirection);
			}
			else
			{
				entity->SetAnimationState(ANIMATION_WALK);
				entity->m_position += entity->m_facingDirection * entity->m_speed * deltaSeconds;
			}
		}
	}
}

// -----------------------------------------------------------------------
void RepairTask::End( Entity* entity )
{
	m_taskStatus = TASKSTATUS_END;

	entity->m_entityToRepair = nullptr;
	entity->SetAnimationState(ANIMATION_IDLE);
	entity->SetAnimationDirection(entity->m_previousAnimationDirection);
}

// -----------------------------------------------------------------------
RTSTask* RepairTask::Clone()
{
	return new RepairTask(*this);
}

// -----------------------------------------------------------------------
// Train Task
// -----------------------------------------------------------------------
void TrainTask::Start( float deltaSeconds )
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		int currentWood = m_theMap->GetWoodForTeam(m_team);
		int currentMinerals = m_theMap->GetMineralForTeam(m_team);
		int currentSupply = m_theMap->GetSupplyForTeam(m_team);
		int maxSupply = m_theMap->GetMaxSupplyForTeam(m_team);

		int woodCost = m_entityDefinitionToTrain->m_woodCost;
		int mineralCost = m_entityDefinitionToTrain->m_mineralCost;
		int supplyCost = (int)m_entityDefinitionToTrain->m_supply;

		if((woodCost > currentWood) || (mineralCost > currentMinerals) || ((currentSupply + supplyCost) > maxSupply))
		{
			End(entity);
			return;
		}
		else
		{
			m_theMap->AddWoodToTeam(-woodCost, m_team);
			m_theMap->AddMineralToTeam(-mineralCost, m_team);
			m_theMap->AddSupplyToTeam(supplyCost, m_team);
			
			DoTask(deltaSeconds);
		}
	}
}

// -----------------------------------------------------------------------
void TrainTask::DoTask( float deltaSeconds )
{
	m_taskStatus = TASKSTATUS_DO;

	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		m_trainTimer += deltaSeconds;
		entity->m_isTraining = true;
		entity->m_buildTimer = m_trainTimer;
		entity->m_maxBuildTime = m_entityDefinitionToTrain->m_buildtime;

		if(m_trainTimer >= m_entityDefinitionToTrain->m_buildtime)
		{
			if(m_entityDefinitionToTrain->m_entityType == "Peon")
			{
				Entity* peon = new Entity(g_theApp->m_theGame, "Peon");
				peon->m_position		= Vec3(entity->m_position.x, entity->m_position.y - 1.1f, entity->m_position.z);
				peon->m_targetPosition	= Vec3(entity->m_position.x, entity->m_position.y - 1.1f, entity->m_position.z);
				peon->m_health			= peon->m_maxHealth;
				peon->m_justConstructed	= false;
				peon->m_team			= m_team;
				peon->m_teamColor		= m_team == 0 ? Rgba(0.68f, 0.85f, 0.9f, 1.0f) : Rgba(1.0f, 0.58f, 0.47f, 1.0f);
				m_theMap->DelayAddEntity(peon);
			}
			else if(m_entityDefinitionToTrain->m_entityType == "Warrior")
			{
				Entity* warrior = new Entity(g_theApp->m_theGame, "Warrior");
				warrior->m_position			= Vec3(entity->m_position.x, entity->m_position.y - 1.1f, entity->m_position.z);
				warrior->m_targetPosition	= Vec3(entity->m_position.x, entity->m_position.y - 1.1f, entity->m_position.z);
				warrior->m_health			= warrior->m_maxHealth;
				warrior->m_justConstructed	= false;
				warrior->m_team				= m_team;
				warrior->m_teamColor		= m_team == 0 ? Rgba(0.68f, 0.85f, 0.9f, 1.0f) : Rgba(1.0f, 0.58f, 0.47f, 1.0f);
				m_theMap->DelayAddEntity(warrior);
			}
			else if(m_entityDefinitionToTrain->m_entityType == "Goblin")
			{
				Entity* goblin = new Entity(g_theApp->m_theGame, "Goblin");
				goblin->m_position			= Vec3(entity->m_position.x, entity->m_position.y - 0.7f, entity->m_position.z);
				goblin->m_targetPosition	= Vec3(entity->m_position.x, entity->m_position.y - 0.7f, entity->m_position.z);
				goblin->m_health			= goblin->m_maxHealth;
				goblin->m_justConstructed	= false;
				goblin->m_team				= m_team;
				goblin->m_teamColor			= m_team == 0 ? Rgba(0.68f, 0.85f, 0.9f, 1.0f) : Rgba(1.0f, 0.58f, 0.47f, 1.0f);
				m_theMap->DelayAddEntity(goblin);
			}
			
			End(entity);
			return;
		}
	}
}

// -----------------------------------------------------------------------
void TrainTask::End( Entity* entity )
{
	entity->m_isTraining = false;
	entity->m_buildTimer = 0.0f;
	entity->m_maxBuildTime = 0.0f;

	m_taskStatus = TASKSTATUS_END;
}

// -----------------------------------------------------------------------
RTSTask* TrainTask::Clone()
{
	return new TrainTask(*this);
}
