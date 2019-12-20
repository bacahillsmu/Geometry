#include "Game/Gameplay/Input/ReplayController.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Gameplay/RTSCommand.hpp"
#include "Game/Framework/App.hpp"


ReplayController* g_theReplayController = nullptr;



// -----------------------------------------------------------------------
ReplayController::ReplayController( Game* theGame )
{
	m_theGame = theGame;
}

// -----------------------------------------------------------------------
ReplayController::~ReplayController()
{
	for(ReplayInfo& replayInfo: m_replayInfo)
	{
		delete replayInfo.rtsCommand;
		replayInfo.rtsCommand = nullptr;
	}
}

// -----------------------------------------------------------------------
void ReplayController::RecordCommand( RTSCommand* rtsCommand )
{
	ReplayInfo replayInfo;
	replayInfo.commandFrame = g_theApp->m_commandFrame.GetCommandFrame();
	replayInfo.rtsCommand = rtsCommand;

	m_replayInfo.push_back(replayInfo);
}

// -----------------------------------------------------------------------
void ReplayController::LoadReplayFile( std::string filename )
{
	std::string loadName = "Data/Replays/" + filename;

	tinyxml2::XMLDocument xmlLoadDoc;
	xmlLoadDoc.LoadFile(loadName.c_str());

	XmlElement* rootElement = xmlLoadDoc.RootElement();
	XmlElement* commandElement = rootElement->FirstChildElement("Command");
	while(commandElement != nullptr)
	{
		ReplayInfo replayInfo;		
		replayInfo.commandFrame = ParseXmlAttribute(*commandElement, "CommandFrame", -1);
		CommandType commandType = (CommandType)ParseXmlAttribute(*commandElement, "CommandType", 999);

		switch(commandType)
		{
			case CREATEPEON:
			{
				CreateCommand* createCommand = new CreateCommand();
				createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				createCommand->m_theMap = m_theGame->GetGameMap();
				createCommand->m_theGame = m_theGame;
				createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
				createCommand->m_unit.m_handle = (uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0);
				createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				replayInfo.rtsCommand = createCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case CREATEWARRIOR:
			{
				CreateCommand* createCommand = new CreateCommand();
				createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				createCommand->m_theMap = m_theGame->GetGameMap();
				createCommand->m_theGame = m_theGame;
				createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
				createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				replayInfo.rtsCommand = createCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case MOVE:
			{
				MoveCommand* moveCommand = new MoveCommand();
				moveCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				moveCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				moveCommand->m_theGame = m_theGame;
				moveCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				moveCommand->m_theMap = m_theGame->GetGameMap();
				moveCommand->m_movePosition = ParseXmlAttribute(*commandElement, "MovePosition", Vec3(0.0f, 0.0f, 0.0f));
				replayInfo.rtsCommand = moveCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case ATTACK:
			{
				AttackCommand* attackCommand = new AttackCommand();
				attackCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				attackCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				attackCommand->m_entityToAttack = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "EntityToAttack", 0));
				attackCommand->m_theGame = m_theGame;
				attackCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				attackCommand->m_theMap = m_theGame->GetGameMap();
				replayInfo.rtsCommand = attackCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case DIE:
			{
				DieCommand* dieCommand = new DieCommand();
				dieCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				dieCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				dieCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				dieCommand->m_theMap = m_theGame->GetGameMap();
				replayInfo.rtsCommand = dieCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case FOLLOW:
			{
				FollowCommand* followCommand = new FollowCommand();
				followCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				followCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				followCommand->m_entityToFollow = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "EntityToFollow", 0));
				followCommand->m_theGame = m_theGame;
				followCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				followCommand->m_theMap = m_theGame->GetGameMap();
				replayInfo.rtsCommand = followCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case RIGHTCLICK:
			{
				RightClickCommand* rightClickCommand = new RightClickCommand();
				rightClickCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				rightClickCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				rightClickCommand->m_theGame = m_theGame;
				rightClickCommand->m_theMap = m_theGame->GetGameMap();
				rightClickCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));				
				rightClickCommand->m_rightClickPosition = ParseXmlAttribute(*commandElement, "RightClickPosition", Vec3(0.0f, 0.0f, 0.0f));
				rightClickCommand->m_rightClickEntity = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "RighClickedEntity", 0));
				replayInfo.rtsCommand = rightClickCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case CREATEPINETREE:
			{
				CreateCommand* createCommand = new CreateCommand();
				createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				createCommand->m_theMap = m_theGame->GetGameMap();
				createCommand->m_theGame = m_theGame;
				createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
				createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				replayInfo.rtsCommand = createCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case CREATETOWNHALL:
			{
				CreateCommand* createCommand = new CreateCommand();
				createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				createCommand->m_theMap = m_theGame->GetGameMap();
				createCommand->m_theGame = m_theGame;
				createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
				createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				replayInfo.rtsCommand = createCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case GATHER:
			{
				GatherCommand* gatherCommand = new GatherCommand();
				gatherCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				gatherCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				gatherCommand->m_theGame = m_theGame;
				gatherCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				gatherCommand->m_theMap = m_theGame->GetGameMap();
				gatherCommand->m_entityToGather = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "EntityToGather", 0));
				replayInfo.rtsCommand = gatherCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case CREATEPINEBARK:
			{
				CreateCommand* createCommand = new CreateCommand();
				createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				createCommand->m_theMap = m_theGame->GetGameMap();
				createCommand->m_theGame = m_theGame;
				createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
				createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				replayInfo.rtsCommand = createCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case CREATEPINESTUMP:
			{
				CreateCommand* createCommand = new CreateCommand();
				createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				createCommand->m_theMap = m_theGame->GetGameMap();
				createCommand->m_theGame = m_theGame;
				createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
				createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				replayInfo.rtsCommand = createCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case REPAIR:
			{
				RepairCommand* repairCommand = new RepairCommand();
				repairCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				repairCommand->m_theGame = g_theApp->m_theGame;
				repairCommand->m_theMap = g_theApp->m_theGame->GetGameMap();
				repairCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				repairCommand->m_entityToRepair = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "EntityToRepair", 0));
				repairCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				replayInfo.rtsCommand = repairCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case TRAINPEON:
			{
				TrainCommand* trainCommand = new TrainCommand();
				trainCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				trainCommand->m_training = ParseXmlAttribute(*commandElement, "Training", "");
				trainCommand->m_entityToTrain = ParseXmlAttribute(*commandElement, "EntityToTrain", "");
				trainCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				trainCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				trainCommand->m_theGame = m_theGame;
				trainCommand->m_theMap = m_theGame->GetGameMap();
				trainCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				replayInfo.rtsCommand = trainCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case TRAINWARRIOR:
			{
				TrainCommand* trainCommand = new TrainCommand();
				trainCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				trainCommand->m_training = ParseXmlAttribute(*commandElement, "Training", "");
				trainCommand->m_entityToTrain = ParseXmlAttribute(*commandElement, "EntityToTrain", "");
				trainCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				trainCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				trainCommand->m_theGame = m_theGame;
				trainCommand->m_theMap = m_theGame->GetGameMap();
				trainCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				replayInfo.rtsCommand = trainCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case BUILDTOWNHALL:
			{
				BuildCommand* buildCommand = new BuildCommand();
				buildCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				buildCommand->m_theGame = m_theGame;
				buildCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				buildCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				buildCommand->m_theMap = m_theGame->GetGameMap();
				buildCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				buildCommand->m_buildPosition = ParseXmlAttribute(*commandElement, "BuildPosition", Vec3(0.0f, 0.0f, 0.0f));
				replayInfo.rtsCommand = buildCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case TRAINGOBLIN:
			{
				TrainCommand* trainCommand = new TrainCommand();
				trainCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				trainCommand->m_training = ParseXmlAttribute(*commandElement, "Training", "");
				trainCommand->m_entityToTrain = ParseXmlAttribute(*commandElement, "EntityToTrain", "");
				trainCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				trainCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				trainCommand->m_theGame = m_theGame;
				trainCommand->m_theMap = m_theGame->GetGameMap();
				trainCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				replayInfo.rtsCommand = trainCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case CREATEGOBLIN:
			{
				CreateCommand* createCommand = new CreateCommand();
				createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				createCommand->m_theMap = m_theGame->GetGameMap();
				createCommand->m_theGame = m_theGame;
				createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
				createCommand->m_unit.m_handle = (uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0);
				createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				replayInfo.rtsCommand = createCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case BUILDHUT:
			{
				BuildCommand* buildCommand = new BuildCommand();
				buildCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				buildCommand->m_theGame = m_theGame;
				buildCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				buildCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				buildCommand->m_theMap = m_theGame->GetGameMap();
				buildCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				buildCommand->m_buildPosition = ParseXmlAttribute(*commandElement, "BuildPosition", Vec3(0.0f, 0.0f, 0.0f));
				replayInfo.rtsCommand = buildCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case BUILDTOWER:
			{
				BuildCommand* buildCommand = new BuildCommand();
				buildCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				buildCommand->m_theGame = m_theGame;
				buildCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				buildCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				buildCommand->m_theMap = m_theGame->GetGameMap();
				buildCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				buildCommand->m_buildPosition = ParseXmlAttribute(*commandElement, "BuildPosition", Vec3(0.0f, 0.0f, 0.0f));
				replayInfo.rtsCommand = buildCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case BUILDGOBLINHUT:
			{
				BuildCommand* buildCommand = new BuildCommand();
				buildCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				buildCommand->m_theGame = m_theGame;
				buildCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				buildCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				buildCommand->m_theMap = m_theGame->GetGameMap();
				buildCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				buildCommand->m_buildPosition = ParseXmlAttribute(*commandElement, "BuildPosition", Vec3(0.0f, 0.0f, 0.0f));
				replayInfo.rtsCommand = buildCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case BUILDGOBLINTOWER:
			{
				BuildCommand* buildCommand = new BuildCommand();
				buildCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				buildCommand->m_theGame = m_theGame;
				buildCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				buildCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				buildCommand->m_theMap = m_theGame->GetGameMap();
				buildCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				buildCommand->m_buildPosition = ParseXmlAttribute(*commandElement, "BuildPosition", Vec3(0.0f, 0.0f, 0.0f));
				replayInfo.rtsCommand = buildCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case CREATEFULLMINERAL:
			{
				CreateCommand* createCommand = new CreateCommand();
				createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				createCommand->m_theMap = m_theGame->GetGameMap();
				createCommand->m_theGame = m_theGame;
				createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
				createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				replayInfo.rtsCommand = createCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case CREATEMIDDLEMINERAL:
			{
				CreateCommand* createCommand = new CreateCommand();
				createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				createCommand->m_theMap = m_theGame->GetGameMap();
				createCommand->m_theGame = m_theGame;
				createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
				createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				replayInfo.rtsCommand = createCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}

			case CREATEEMPTYMINERAL:
			{
				CreateCommand* createCommand = new CreateCommand();
				createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
				createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
				createCommand->m_theMap = m_theGame->GetGameMap();
				createCommand->m_theGame = m_theGame;
				createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
				createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
				createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
				replayInfo.rtsCommand = createCommand;
				m_replayInfo.push_back(replayInfo);

				break;
			}
		}

		commandElement = commandElement->NextSiblingElement();
	}

	//std::reverse(m_replayInfo.begin(),m_replayInfo.end());
}

// -----------------------------------------------------------------------
void ReplayController::SaveReplayFile( std::string filename )
{
	tinyxml2::XMLDocument xmlSaveDoc;
	tinyxml2::XMLElement* rootNode = xmlSaveDoc.NewElement("Root");
	xmlSaveDoc.InsertFirstChild(rootNode);

	for(ReplayInfo& replayInfo: m_replayInfo)
	{
		tinyxml2::XMLElement* command = xmlSaveDoc.NewElement("Command");
		rootNode->InsertFirstChild(command);

		command->SetAttribute("CommandFrame", replayInfo.commandFrame);
		replayInfo.rtsCommand->AppendDataToXMLElement(*command);
	}

	std::string saveName = "Data/Replays/" + filename;
	xmlSaveDoc.SaveFile(saveName.c_str());
}
