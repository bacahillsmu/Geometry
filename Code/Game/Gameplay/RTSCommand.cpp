#include "Game/Framework/App.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Gameplay/RTSCommand.hpp"
#include "Game/Gameplay/RTSTask.hpp"
#include "Game/Gameplay/Entity.hpp"
#include "Game/Gameplay/EntityDefinition.hpp"

// -----------------------------------------------------------------------
// Create
// -----------------------------------------------------------------------
void CreateCommand::Execute()
{
	switch((CommandType)m_commandType)
	{
		case CREATEPEON:
		{
			Entity* peon = new Entity(g_theApp->m_theGame, "Peon");
			peon->m_position		= m_createPosition;
			peon->m_targetPosition	= m_createPosition;
			peon->m_health			= m_percentHealth * peon->m_maxHealth;
			peon->m_justConstructed	= m_justConstructed;
			peon->m_team			= m_team;
			peon->m_teamColor		= m_team == 0 ? Rgba(0.68f, 0.85f, 0.9f, 1.0f) : Rgba(1.0f, 0.58f, 0.47f, 1.0f);
			m_theMap->DelayAddEntity(peon);
			break;
		}
		case CREATEWARRIOR:
		{
			Entity* warrior = new Entity(g_theApp->m_theGame, "Warrior");
			warrior->m_position			= m_createPosition;
			warrior->m_targetPosition	= m_createPosition;
			warrior->m_health			= m_percentHealth * warrior->m_maxHealth;
			warrior->m_justConstructed	= m_justConstructed;
			warrior->m_team				= m_team;
			warrior->m_teamColor		= m_team == 0 ? Rgba(0.68f, 0.85f, 0.9f, 1.0f) : Rgba(1.0f, 0.58f, 0.47f, 1.0f);
			m_theMap->DelayAddEntity(warrior);
			break;
		}
		case CREATEGOBLIN:
		{
			Entity* goblin = new Entity(g_theApp->m_theGame, "Goblin");
			goblin->m_position			= m_createPosition;
			goblin->m_targetPosition	= m_createPosition;
			goblin->m_health			= m_percentHealth * goblin->m_maxHealth;
			goblin->m_justConstructed	= m_justConstructed;
			goblin->m_team				= m_team;
			goblin->m_teamColor			= m_team == 0 ? Rgba(0.68f, 0.85f, 0.9f, 1.0f) : Rgba(1.0f, 0.58f, 0.47f, 1.0f);
			m_theMap->DelayAddEntity(goblin);
			break;
		}

		case CREATEPINETREE:
		{
			Entity* pinewhole = new Entity(g_theApp->m_theGame, "pinewhole");
			pinewhole->m_position			= m_createPosition;
			pinewhole->m_targetPosition		= m_createPosition;
			pinewhole->m_health				= m_percentHealth * pinewhole->m_maxHealth;
			pinewhole->m_justConstructed	= m_justConstructed;
			pinewhole->m_team				= m_team;
			pinewhole->m_teamColor			= Rgba::WHITE;
			m_theMap->AddEntity(pinewhole);
			break;
		}

		case CREATEPINEBARK:
		{
			Entity* pinebark = new Entity(g_theApp->m_theGame, "pinebark");
			pinebark->m_position = m_createPosition;
			pinebark->m_targetPosition = m_createPosition;
			pinebark->m_health			= m_percentHealth * pinebark->m_maxHealth;
			pinebark->m_justConstructed	= m_justConstructed;
			pinebark->m_team = m_team;
			pinebark->m_teamColor = Rgba::WHITE;
			m_theMap->AddEntity(pinebark);
			break;
		}

		case CREATEPINESTUMP:
		{
			Entity* pinestump = new Entity(g_theApp->m_theGame, "pinestump");
			pinestump->m_position			= m_createPosition;
			pinestump->m_targetPosition		= m_createPosition;
			pinestump->m_health				= m_percentHealth * pinestump->m_maxHealth;
			pinestump->m_justConstructed	= m_justConstructed;
			pinestump->m_team				= m_team;
			pinestump->m_teamColor			= Rgba::WHITE;
			m_theMap->AddEntity(pinestump);
			break;
		}

		case CREATETOWNHALL:
		{
			Entity* townhall = new Entity(g_theApp->m_theGame, "TownHall");
			townhall->m_position		= m_createPosition;
			townhall->m_targetPosition	= m_createPosition;
			townhall->m_health			= m_percentHealth * townhall->m_maxHealth;
			townhall->m_justConstructed = m_justConstructed;
			townhall->m_team			= m_team;
			townhall->m_teamColor = m_team == 0 ? Rgba(0.68f, 0.85f, 0.9f, 1.0f) : Rgba(1.0f, 0.58f, 0.47f, 1.0f);
			m_theMap->DelayAddEntity(townhall);
			break;
		}

		case CREATEGOBLINHUT:
		{
			Entity* goblinHut = new Entity(g_theApp->m_theGame, "GoblinHut");
			goblinHut->m_position		= m_createPosition;
			goblinHut->m_targetPosition	= m_createPosition;
			goblinHut->m_health			= m_percentHealth * goblinHut->m_maxHealth;
			goblinHut->m_justConstructed = m_justConstructed;
			goblinHut->m_team			= m_team;
			goblinHut->m_teamColor = m_team == 0 ? Rgba(0.68f, 0.85f, 0.9f, 1.0f) : Rgba(1.0f, 0.58f, 0.47f, 1.0f);
			m_theMap->DelayAddEntity(goblinHut);
			break;
		}

		case CREATEFULLMINERAL:
		{
			Entity* full = new Entity(g_theApp->m_theGame, "fullmineral");
			full->m_position			= m_createPosition;
			full->m_targetPosition		= m_createPosition;
			full->m_health				= m_percentHealth * full->m_maxHealth;
			full->m_justConstructed		= m_justConstructed;
			full->m_team				= m_team;
			full->m_teamColor			= Rgba::WHITE;
			m_theMap->AddEntity(full);
			break;
		}

		case CREATEMIDDLEMINERAL:
		{
			Entity* middle = new Entity(g_theApp->m_theGame, "middlemineral");
			middle->m_position				= m_createPosition;
			middle->m_targetPosition		= m_createPosition;
			middle->m_health				= m_percentHealth * middle->m_maxHealth;
			middle->m_justConstructed		= m_justConstructed;
			middle->m_team					= m_team;
			middle->m_teamColor				= Rgba::WHITE;
			m_theMap->AddEntity(middle);
			break;
		}

		case CREATEEMPTYMINERAL:
		{
			Entity* emptyMineral = new Entity(g_theApp->m_theGame, "emptymineral");
			emptyMineral->m_position			= m_createPosition;
			emptyMineral->m_targetPosition		= m_createPosition;
			emptyMineral->m_health				= m_percentHealth * emptyMineral->m_maxHealth;
			emptyMineral->m_justConstructed		= m_justConstructed;
			emptyMineral->m_team				= m_team;
			emptyMineral->m_teamColor			= Rgba::WHITE;
			m_theMap->AddEntity(emptyMineral);
			break;
		}
	}	
}

// -----------------------------------------------------------------------
void CreateCommand::AppendDataToXMLElement( tinyxml2::XMLElement& element )
{
	element.SetAttribute("GameHandle", (int)m_unit.m_handle);
	element.SetAttribute("CommandType", m_commandType);
	element.SetAttribute("CommandFrameIssued", m_commandFrameIssued);

	std::string justConstructed = m_justConstructed ? "true" : "false";
	element.SetAttribute("JustConstructed", justConstructed.c_str());
	element.SetAttribute("PercentHealth", m_percentHealth);
	element.SetAttribute("Team", m_team);
	element.SetAttribute("CreatePosition", m_createPosition.GetAsString().c_str());
	
}

// -----------------------------------------------------------------------
// Die
// -----------------------------------------------------------------------
void DieCommand::Execute()
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		DieTask* dieTask = new DieTask();
		dieTask->m_taskStatus = TASKSTATUS_START;
		dieTask->m_unit = m_unit;
		dieTask->m_theMap = m_theMap;
		dieTask->m_instantDeath = m_instantDeath;
		dieTask->m_deathTimerThreshold = m_deathTimerThreshold;

		entity->AddTask(dynamic_cast<RTSTask*>(dieTask));
	}
}

// -----------------------------------------------------------------------
void DieCommand::AppendDataToXMLElement( tinyxml2::XMLElement& element )
{
	element.SetAttribute("GameHandle", (int)m_unit.m_handle);
	element.SetAttribute("CommandType", m_commandType);
	element.SetAttribute("CommandFrameIssued", m_commandFrameIssued);

	std::string instantDeath = m_instantDeath ? "true" : "false";
	element.SetAttribute("InstantDeath", instantDeath.c_str());
}

// -----------------------------------------------------------------------
// Right Click
// -----------------------------------------------------------------------
void RightClickCommand::Execute()
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		entity->ProcessRightClickCommand(this);
	}
}

// -----------------------------------------------------------------------
// Build
// -----------------------------------------------------------------------
void RightClickCommand::AppendDataToXMLElement( tinyxml2::XMLElement& element )
{
	element.SetAttribute("GameHandle", (int)m_unit.m_handle);
	element.SetAttribute("CommandType", m_commandType);
	element.SetAttribute("CommandFrameIssued", m_commandFrameIssued);

	element.SetAttribute("RightClickPosition", m_rightClickPosition.GetAsString().c_str());
	element.SetAttribute("RighClickedEntity", (int)m_rightClickEntity.m_handle);
}

// -----------------------------------------------------------------------
void BuildCommand::Execute()
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		std::string buildtask = "";
		switch ((CommandType)m_commandType)
		{
			case BUILDTOWNHALL:
			{
				buildtask = "buildtowncenter";
				break;
			}
			case BUILDHUT:
			{
				buildtask = "buildhut";
				break;
			}
			case BUILDTOWER:
			{
				buildtask = "buildtower";
				break;
			}
			case BUILDGOBLINHUT:
			{
				buildtask = "buildgoblintower";
				break;
			}
			case BUILDGOBLINTOWER:
			{
				buildtask = "buildgoblintower";
				break;
			}
		}
		std::map< std::string, RTSTaskInfo* >::iterator found = entity->m_entityDefinition->m_availableTasks.find(buildtask);
		if(found != entity->m_entityDefinition->m_availableTasks.end())
		{
			RTSTaskInfo* rtsTaskInfo = found->second;
			BuildTask* buildTask = static_cast<BuildTask*>(rtsTaskInfo->rtsTask->Clone());
			buildTask->m_taskStatus = TASKSTATUS_START;
			buildTask->m_unit = m_unit;
			buildTask->m_team = m_team;
			buildTask->m_buildPosition = m_buildPosition;
			buildTask->m_theMap = m_theMap;
			std::string toBuild = "";
			CommandType commandType = (CommandType)m_commandType;
			switch(commandType)
			{
				case BUILDTOWNHALL:
				{
					toBuild = "TownHall";
					break;
				}
				case BUILDHUT:
				{
					toBuild = "Hut";
					break;
				}
				case BUILDTOWER:
				{
					toBuild = "Tower";
					break;
				}
				case BUILDGOBLINHUT:
				{
					toBuild = "GoblinHut";
					break;
				}
				case BUILDGOBLINTOWER:
				{
					toBuild = "GoblinTower";
					break;
				}
			}
			std::map<std::string, EntityDefinition*>::iterator e = EntityDefinition::s_entityDefinitions.find(toBuild.c_str());
			buildTask->m_entityDefToBuild = e->second;

			entity->AddTask(dynamic_cast<RTSTask*>(buildTask));
		}
	}
}

// -----------------------------------------------------------------------
void BuildCommand::AppendDataToXMLElement( tinyxml2::XMLElement& element )
{
	element.SetAttribute("GameHandle", (int)m_unit.m_handle);
	element.SetAttribute("CommandType", m_commandType);
	element.SetAttribute("CommandFrameIssued", m_commandFrameIssued);

	element.SetAttribute("Team", m_team);
	element.SetAttribute("BuildPosition", m_buildPosition.GetAsString().c_str());
}

// -----------------------------------------------------------------------
// Attack
// -----------------------------------------------------------------------
void AttackCommand::Execute()
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		std::map< std::string, RTSTaskInfo* >::iterator found = entity->m_entityDefinition->m_availableTasks.find("attack");
		if(found != entity->m_entityDefinition->m_availableTasks.end())
		{
			RTSTaskInfo* rtsTaskInfo = found->second;
			AttackTask* attackTask = static_cast<AttackTask*>(rtsTaskInfo->rtsTask->Clone());
			attackTask->m_taskStatus = TASKSTATUS_START;
			attackTask->m_unit = m_unit;
			attackTask->m_theMap = m_theMap;
			attackTask->m_entityToAttack = m_entityToAttack;

			entity->AddTask(attackTask);
		}
	}
}

// -----------------------------------------------------------------------
void AttackCommand::AppendDataToXMLElement( tinyxml2::XMLElement& element )
{
	element.SetAttribute("GameHandle", (int)m_unit.m_handle);
	element.SetAttribute("CommandType", m_commandType);
	element.SetAttribute("CommandFrameIssued", m_commandFrameIssued);

	element.SetAttribute("EntityToAttack", (int)m_entityToAttack.m_handle);
}

// -----------------------------------------------------------------------
// Follow
// -----------------------------------------------------------------------
void FollowCommand::Execute()
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		std::map< std::string, RTSTaskInfo* >::iterator found = entity->m_entityDefinition->m_availableTasks.find("follow");
		if(found != entity->m_entityDefinition->m_availableTasks.end())
		{
			RTSTaskInfo* rtsTaskInfo = found->second;
			FollowTask* followTask = static_cast<FollowTask*>(rtsTaskInfo->rtsTask->Clone());
			followTask->m_taskStatus = TASKSTATUS_START;
			followTask->m_unit = m_unit;
			followTask->m_theMap = m_theMap;
			followTask->m_entityToFollow = m_entityToFollow;

			entity->AddTask(followTask);
		}
	}
}

// -----------------------------------------------------------------------
void FollowCommand::AppendDataToXMLElement( tinyxml2::XMLElement& element )
{
	element.SetAttribute("GameHandle", (int)m_unit.m_handle);
	element.SetAttribute("CommandType", m_commandType);
	element.SetAttribute("CommandFrameIssued", m_commandFrameIssued);

	element.SetAttribute("EntityToFollow", (int)m_entityToFollow.m_handle);
}

// -----------------------------------------------------------------------
// Move
// -----------------------------------------------------------------------
void MoveCommand::Execute()
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		std::map< std::string, RTSTaskInfo* >::iterator found = entity->m_entityDefinition->m_availableTasks.find("move");
		if(found != entity->m_entityDefinition->m_availableTasks.end())
		{
			RTSTaskInfo* rtsTaskInfo = found->second;
			MoveTask* moveTask = static_cast<MoveTask*>(rtsTaskInfo->rtsTask->Clone());
			moveTask->m_taskStatus = TASKSTATUS_START;
			moveTask->m_unit = m_unit;
			moveTask->m_theMap = m_theMap;
			moveTask->m_targetPosition = m_movePosition;

			entity->AddTask(moveTask);
		}
	}
}

// -----------------------------------------------------------------------
void MoveCommand::AppendDataToXMLElement( tinyxml2::XMLElement& element )
{
	element.SetAttribute("GameHandle", (int)m_unit.m_handle);
	element.SetAttribute("CommandType", m_commandType);
	element.SetAttribute("CommandFrameIssued", m_commandFrameIssued);

	element.SetAttribute("MovePosition", m_movePosition.GetAsString().c_str());
}

// -----------------------------------------------------------------------
// Gather
// -----------------------------------------------------------------------
void GatherCommand::Execute()
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		std::map< std::string, RTSTaskInfo* >::iterator found = entity->m_entityDefinition->m_availableTasks.find("gather");
		if(found != entity->m_entityDefinition->m_availableTasks.end())
		{
			RTSTaskInfo* rtsTaskInfo = found->second;
			GatherTask* gatherTask = static_cast<GatherTask*>(rtsTaskInfo->rtsTask->Clone());
			gatherTask->m_taskStatus = TASKSTATUS_START;
			gatherTask->m_unit = m_unit;
			gatherTask->m_theMap = m_theMap;
			gatherTask->m_entityToGather = m_entityToGather;

			entity->AddTask(gatherTask);
		}
	}
}

// -----------------------------------------------------------------------
void GatherCommand::AppendDataToXMLElement( tinyxml2::XMLElement& element )
{
	element.SetAttribute("GameHandle", (int)m_unit.m_handle);
	element.SetAttribute("CommandType", m_commandType);
	element.SetAttribute("CommandFrameIssued", m_commandFrameIssued);

	element.SetAttribute("EntityToGather", (int)m_entityToGather.m_handle);
}

// -----------------------------------------------------------------------
// Repair
// -----------------------------------------------------------------------
void RepairCommand::Execute()
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		std::map< std::string, RTSTaskInfo* >::iterator found = entity->m_entityDefinition->m_availableTasks.find("repair");
		if(found != entity->m_entityDefinition->m_availableTasks.end())
		{
			RTSTaskInfo* rtsTaskInfo = found->second;
			RepairTask* repairTask = static_cast<RepairTask*>(rtsTaskInfo->rtsTask->Clone());
			repairTask->m_taskStatus = TASKSTATUS_START;
			repairTask->m_unit = m_unit;
			repairTask->m_theMap = m_theMap;
			repairTask->m_entityToRepair = m_entityToRepair;

			entity->AddTask(repairTask);
		}
	}
}

// -----------------------------------------------------------------------
void RepairCommand::AppendDataToXMLElement( tinyxml2::XMLElement& element )
{
	element.SetAttribute("GameHandle", (int)m_unit.m_handle);
	element.SetAttribute("CommandType", m_commandType);
	element.SetAttribute("CommandFrameIssued", m_commandFrameIssued);

	element.SetAttribute("EntityToRepair", (int)m_entityToRepair.m_handle);
}

// -----------------------------------------------------------------------
// Train
// -----------------------------------------------------------------------
void TrainCommand::Execute()
{
	Entity* entity = m_theMap->FindEntity(m_unit);
	if(entity)
	{
		std::map< std::string, RTSTaskInfo* >::iterator found = entity->m_entityDefinition->m_availableTasks.find(m_training);
		if(found != entity->m_entityDefinition->m_availableTasks.end())
		{
			std::map< std::string, EntityDefinition* >::iterator found2 = EntityDefinition::s_entityDefinitions.find(m_entityToTrain);;
			EntityDefinition* entityDefinition = found2->second;
			RTSTaskInfo* rtsTaskInfo = found->second;
			TrainTask* trainTask = static_cast<TrainTask*>(rtsTaskInfo->rtsTask->Clone());
			trainTask->m_taskStatus = TASKSTATUS_START;
			trainTask->m_unit = m_unit;
			trainTask->m_theMap = m_theMap;
			trainTask->m_team = m_team;
			trainTask->m_entityDefinitionToTrain = entityDefinition;

			entity->AddTask(trainTask);
		}
	}
}

// -----------------------------------------------------------------------
void TrainCommand::AppendDataToXMLElement( tinyxml2::XMLElement& element )
{
	element.SetAttribute("GameHandle", (int)m_unit.m_handle);
	element.SetAttribute("CommandType", m_commandType);
	element.SetAttribute("CommandFrameIssued", m_commandFrameIssued);

	element.SetAttribute("Team", m_team);
	element.SetAttribute("Training", m_training.c_str());
	element.SetAttribute("EntityToTrain", m_entityToTrain.c_str());
}
