#include "Game/Gameplay/Input/AIController.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Gameplay/Map/Map.hpp"
#include "Game/Gameplay/EntityDefinition.hpp"
#include "Game/Gameplay/RTSCommand.hpp"
#include "Game/Gameplay/Input/ReplayController.hpp"
#include "Game/Gameplay/Entity.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Gameplay/RTSTask.hpp"

AIController* g_theAIController = nullptr;



// -----------------------------------------------------------------------
AIController::AIController( Game* game )
{
	m_theGame = game;
}

// -----------------------------------------------------------------------
AIController::~AIController()
{

}

// -----------------------------------------------------------------------
void AIController::UpdateThoughts()
{
	m_theMap = m_theGame->GetGameMap();
	m_theMap->CalculateSupplyForTeam(m_aiTeam);

	if(!m_createdFirstTownCenter)
	{
		CreateFirstTownCenter();
	}

	LazyGoblinCheck();
	TrainGoblin();

	std::string type = "Goblin";
	std::vector<Entity*> goblins = m_theMap->GetEntitiesOfTypeOnTeam(type, m_aiTeam);
	if(goblins.size() >= 8 && !m_issuedAttack)
	{
		for(Entity* goblin: goblins)
		{
			goblin->m_aiControlling = false;
			goblin->m_tasks.clear();
		}
		GoblinsAttack();
		m_issuedAttack = true;
	}


	GoblinsGather();

	
	
}

// -----------------------------------------------------------------------
void AIController::LazyGoblinCheck()
{
	std::string type = "Goblin";
	std::vector<Entity*> goblins = m_theMap->GetEntitiesOfTypeOnTeam(type, m_aiTeam);
	if(goblins.size() > 0)
	{
		for(Entity* goblin: goblins)
		{
			if(goblin->m_tasks.size() == 0 && goblin->m_aiControlling)
			{
				goblin->m_aiControlling = false;
			}
		}
	}
}

// -----------------------------------------------------------------------
void AIController::TrainGoblin()
{
	std::vector<Entity*> goblinHuts = m_theMap->GetGoblinHutsOnTeam(m_aiTeam);
	if(goblinHuts.size() > 0)
	{
		for(Entity* goblinHut: goblinHuts)
		{
			std::map< std::string, RTSTaskInfo* >::iterator found = goblinHut->m_entityDefinition->m_availableTasks.find("traingoblin");
			if(found != goblinHut->m_entityDefinition->m_availableTasks.end())
			{
				std::map< std::string, EntityDefinition* >::iterator goblin = EntityDefinition::s_entityDefinitions.find("Goblin");
				if(goblin != EntityDefinition::s_entityDefinitions.end())
				{
					//EntityDefinition* entityDefintion = warrior->second;
					TrainCommand* trainCommand = new TrainCommand();
					trainCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
					trainCommand->m_commandType = TRAINGOBLIN;
					trainCommand->m_training = "traingoblin";
					trainCommand->m_entityToTrain = "Goblin";
					trainCommand->m_team = m_aiTeam;
					trainCommand->m_theGame = m_theGame;
					trainCommand->m_theMap = m_theGame->GetGameMap();
					trainCommand->m_unit = goblinHut->GetGameHandle();
					m_theGame->EnQueueCommand(trainCommand);
					g_theReplayController->RecordCommand(trainCommand);

					goblinHut->m_isTraining = true;
				}
			}
		}
	}
}

// -----------------------------------------------------------------------
void AIController::GoblinsGather()
{
	std::string type = "Goblin";
	std::vector<Entity*> goblins = m_theMap->GetEntitiesOfTypeOnTeam(type, m_aiTeam);
	if(goblins.size() > 0)
	{
		for(Entity* goblin: goblins)
		{
			if(!goblin->m_aiControlling)
			{
				Entity* tree = goblin->FindNearestTree();
				std::map< std::string, RTSTaskInfo* >::iterator found = goblin->m_entityDefinition->m_availableTasks.find("gather");
				if(found != goblin->m_entityDefinition->m_availableTasks.end())
				{
					RTSTaskInfo* rtsTaskInfo = found->second;
					GatherTask* gatherTask = static_cast<GatherTask*>(rtsTaskInfo->rtsTask->Clone());
					gatherTask->m_taskStatus = TASKSTATUS_START;
					gatherTask->m_unit = goblin->GetGameHandle();
					gatherTask->m_theMap = m_theMap;
					gatherTask->m_entityToGather = tree->GetGameHandle();

					goblin->AddTask(gatherTask);
					goblin->m_aiControlling = true;
				}
			}
		}
	}
}

// -----------------------------------------------------------------------
void AIController::GoblinsAttack()
{
	std::string type = "Goblin";
	std::vector<Entity*> goblins = m_theMap->GetEntitiesOfTypeOnTeam(type, m_aiTeam);
	if(goblins.size() > 0)
	{
		for(Entity* goblin: goblins)
		{
			if(!goblin->m_aiControlling)
			{
				Entity* entityToAttack = goblin->FindNearestEntityUnitOnOtherTeam(m_aiTeam);
				std::map< std::string, RTSTaskInfo* >::iterator found = goblin->m_entityDefinition->m_availableTasks.find("attack");
				if(found != goblin->m_entityDefinition->m_availableTasks.end())
				{
					RTSTaskInfo* rtsTaskInfo = found->second;
					AttackTask* attackTask = static_cast<AttackTask*>(rtsTaskInfo->rtsTask->Clone());
					attackTask->m_taskStatus = TASKSTATUS_START;
					attackTask->m_unit = goblin->GetGameHandle();
					attackTask->m_theMap = m_theMap;
					attackTask->m_entityToAttack = entityToAttack->GetGameHandle();

					goblin->AddTask(attackTask);
					goblin->m_aiControlling = true;
				}
			}
		}
	}
}

// -----------------------------------------------------------------------
void AIController::CreateFirstTownCenter()
{
	Vec3 position = Vec3(12.0f, 4.0f, 0.0f);
	IntVec2 tile = IntVec2((int)floor(position.x), (int)floor(position.y));

	std::map< std::string, EntityDefinition* >::iterator goblinHut = EntityDefinition::s_entityDefinitions.find("GoblinHut");
	EntityDefinition* goblinHutDefinition = goblinHut->second;

	IntVec2 offset = goblinHutDefinition->m_offset;
	tile.x -= (int)offset.x;
	tile.y -= (int)offset.y;

	int tileIndex = GetIndexFromCoord(tile, m_theMap->m_tileDimensions);
	int status = m_theMap->GetOccupiedTileIndexStatus(tileIndex);

	if(status == -1)
	{
		CreateCommand* createCommand = new CreateCommand();
		createCommand->m_commandFrameIssued = 15;
		createCommand->m_theGame = m_theGame;
		createCommand->m_team = m_aiTeam;
		createCommand->m_commandType = CREATEGOBLINHUT;
		createCommand->m_theMap = m_theGame->GetGameMap();
		createCommand->m_createPosition = position;
		m_theGame->EnQueueCommand(createCommand);
		g_theReplayController->RecordCommand(createCommand);
	}

	m_createdFirstTownCenter = true;
}
