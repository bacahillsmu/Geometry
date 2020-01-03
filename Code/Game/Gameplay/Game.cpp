// Defines and Includes ---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN	
#include <winsock2.h>
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Game/Gameplay/Map/Map.hpp"
#include "Game/Gameplay/Input/PlayerController.hpp"
#include "Game/Gameplay/Input/ReplayController.hpp"
#include "Game/Gameplay/Input/AIController.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimationDefinition.hpp"
#include "Engine/Renderer/BitMapFont.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Core/VertexLit.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Game/Gameplay/Camera/OrbitCamera.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Gameplay/UI/UIWidget.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Prop.hpp"
#include "Engine/Renderer/Model.hpp"
#include "Game/Gameplay/RTSCommand.hpp"
#include "ThirdParty/RakNet/RakNetInterface.hpp"
#include "Game/Gameplay/EntityDefinition.hpp"
#include "Game/Gameplay/Lobby/LobbyConsole.hpp"
#include "Engine/Memory/Memory.hpp"
#include "Engine/Profile/Profile.hpp"
#include "Engine/Job/Jobs.hpp"
#include "Engine/Job/SaveImageJob.hpp"

#include "Engine/Log/Log.hpp"
#include "Engine/Profile/Profile.hpp"

#include "Engine/Scripting/Python/Python.hpp"

// Callbacks
static bool PythonTest(EventArgs& args)
{
	UNUSED(args);

	const wchar_t* program = L"TestProgram";

	// Pre Setup;
	Py_SetProgramName(program);
	Py_SetPath(L"./python37.zip");
	
	// add in my module
	PyImport_AppendInittab("bEngine", &MyModuleInit);

	// Initialize the interpreter;
	Py_Initialize();

	// Do Things;
	PyRun_SimpleString(
		"import sys\n"
		"import bEngine\n"
		"class LogOutput:\n"
		"   def write(self, txt):\n"
		"      bEngine.Print(txt)\n"
		"   def flush(self):\n"
		"      pass\n"
		"logOutput = LogOutput()\n"
		"sys.stdout = logOutput\n"
		"sys.stderr = logOutput\n"
	);

	PyRun_SimpleString(
		"import bEngine\n"
		"bEngine.Print('Hello')\n"
	);

	return true;
}

// static bool PythonTest(EventArgs& args)
// {
// 	char const* filename = "data/scripts/main.py";
// 	FILE* file = fopen(filename, "r+");
// 	int auto_close = 1;
// 
// 	PyRun_SimpleFileExFlags(file, filename, auto_close, nullptr);
// 
// 	PyRun_SimpleString("SomeFunction(99,123)\n");
// 
// 	return true;
// }



static bool ResizeMap(EventArgs& args)
{
	// do stuff with the word that comes before and after "="
	// example: color=red

	IntVec2 mapSize = args.GetValue("size", IntVec2(1, 1));

	if(mapSize.x != 1 && mapSize.y != 1)
	{
		Map*& map = g_theApp->m_theGame->GetMap();
		if(map)
		{
			delete map;
			map = nullptr;

			map = new Map(g_theApp->m_theGame);

			GUARANTEE_RECOVERABLE(g_theApp->m_theGame->GetGameState() == GAMESTATE_EDIT, "Must be in Edit Mode to Resize Map.");

			map->Load("edit", mapSize.x, mapSize.y);
		}
	}


	return true;
}

static bool QuitGame(EventArgs& args)
{
	// do stuff with the word that comes before and after "="
	// example: color=red

	bool quit = args.GetValue("quit", false);

	if(quit)
	{
		g_theApp->HandleCloseApplication();
	}


	return true;
}

static bool LogMemory(EventArgs& args)
{
	args;

	MemTrackLogLiveAllocations();

	return true;
}

static bool LogFile(EventArgs& args)
{
	args;

	std::string asd = "ggg";
	LogCallstackf("Testing", "testttt %s %d", asd.c_str(), 5);

	return true;
}

static bool LoadReplay(EventArgs& args)
{
	// do stuff with the word that comes before and after "="
	// example: color=red

	g_theApp->m_theGame->m_replayFilename = args.GetValue("filename", "ERROR");
	g_theApp->m_theGame->m_readFromReplay = true;
	g_thePlayerController->SetGameState(GAMESTATE_PLAYMAPLOAD);


	return true;
}

static bool CreateServer(EventArgs& args)
{
	UNUSED(args);
	return g_theRakNetInterface->CreateServer();
}

static bool JoinServerAsClient(EventArgs& args)
{
	std::string ip = args.GetValue("ip", "127.0.0.1");
	g_theRakNetInterface->JoinServerAsClient(ip);
	return true;
}

static bool GetReport(EventArgs& args)
{
	UNUSED(args);

	g_theProfilerReport->GetFrameInHistory();
	
	return true;
}

static bool SetSort(EventArgs& args)
{
	std::string sort = args.GetValue("sort", "total");

	if(sort == "total")
	{
		g_theProfilerReport->SetSort(true);
	}
	else if(sort == "self")
	{
		g_theProfilerReport->SetSort(false);
	}

	return true;
}

static bool SetTreeType(EventArgs& args)
{
	std::string tree = args.GetValue("type", "tree");

	if (tree == "tree")
	{
		g_theProfilerReport->SetFlat(false);
	}
	else if (tree == "flat")
	{
		g_theProfilerReport->SetFlat(true);
	}

	return true;
}

static bool SaveScreenshot(EventArgs& args)
{
	std::string name = args.GetValue("name", "screenshot.png");

	g_theRenderer->RequestScreenshot("Data/Screenshots/" + name + ".png");

	return true;
}


// Constructor ------------------------------------------------------------------------------------
Game::Game() 
{
}

// Deconstructor ----------------------------------------------------------------------------------
Game::~Game()
{
	delete m_gameMainCamera;
	m_gameMainCamera = nullptr;

	delete m_uiCamera;
	m_uiCamera = nullptr;

	delete m_cubeMesh;
	m_cubeMesh = nullptr;

	delete m_sphereMesh;
	m_sphereMesh = nullptr;

	delete m_titleMesh;
	m_titleMesh = nullptr;

	delete m_map;
	m_map = nullptr;

	DeleteUIWidgets();

	
}

void Game::Init()
{
	LogSeconds log("Game Init: ", 0.0);
	// Set the GameState;
	SetGameState(GAMESTATE_INIT);

	// Store the bounds of our Game;
	IntVec2 clientMins = g_theWindowContext->GetClientMins();
	IntVec2 clientMaxs = g_theWindowContext->GetClientMaxs();

	m_clientMins = Vec2((float)clientMins.x, (float)clientMaxs.y);
	m_clientMaxs = Vec2((float)clientMaxs.x, (float)clientMins.y);

	CreateCameras();

	CreateLoadingScreen();	
}

// -----------------------------------------------------------------------
// Startup
// -----------------------------------------------------------------------
void Game::Startup()
{
	
	g_theRakNetInterface->m_packetCallback = [=](RakNet::Packet* packet)
	{
		this->OnIncomingPacket(packet);
	};

	g_theEventSystem->SubscriptionEventCallbackFunction( "resize_map", ResizeMap);
	g_theEventSystem->SubscriptionEventCallbackFunction( "quit_game", QuitGame);
	g_theEventSystem->SubscriptionEventCallbackFunction( "log_memory", LogMemory);
	g_theEventSystem->SubscriptionEventCallbackFunction( "log_file", LogFile);
	g_theEventSystem->SubscriptionEventCallbackFunction( "load_replay", LoadReplay);
	g_theEventSystem->SubscriptionEventCallbackFunction( "create_server", CreateServer);
	g_theEventSystem->SubscriptionEventCallbackFunction( "join_server", JoinServerAsClient);
	g_theEventSystem->SubscriptionEventCallbackFunction( "get_report", GetReport);

	g_theEventSystem->SubscriptionEventCallbackFunction( "set_treetype", SetTreeType);
	g_theEventSystem->SubscriptionEventCallbackFunction( "set_sort", SetSort);

	g_theEventSystem->SubscriptionEventCallbackFunction( "save_screenshot", SaveScreenshot);
	
	
	g_theEventSystem->SubscriptionEventCallbackFunction( "pytest", PythonTest);

	StartLoadingAssets();	

	SetGameState(GAMESTATE_LOADING);

	g_theWindowContext->ShowMouse();
	g_theWindowContext->SetMouseMode(MOUSE_MODE_ABSOLUTE);
}

// -----------------------------------------------------------------------
void Game::BeginFrame()
{
	g_theJobSystem->ProcessJobCategoryForMS(JOBCATEGORY_MAIN, 5);
	g_theJobSystem->ProcessJobCategoryForMS(JOBCATEGORY_RENDER, 5);
	g_theJobSystem->ProcessFinishCallbacksForJobCategory(JOBCATEGORY_MAIN);
	g_theJobSystem->ProcessFinishCallbacksForJobCategory(JOBCATEGORY_RENDER);
	g_theJobSystem->ProcessFinishCallbacksForJobCategory(JOBCATEGORY_GENERIC);

}

// -----------------------------------------------------------------------
void Game::PreRenderStateStartup()
{
	switch(m_gameState)
	{
		case GAMESTATE_TITLE:
		{
			if(m_titleStartupDone)
			{
				return;
			}

			TitleStartup();

			break;
		}

		case GAMESTATE_LOBBY:
		{
			if(m_lobbyStartupDone)
			{
				return;
			}

			LobbyStartup();

			break;
		}

		case GAMESTATE_PLAYMAPLOAD:
		{
			if(m_playStartupDone)
			{
				return;
			}

			PlayMapLoadStartup();

			break;
		}

		case GAMESTATE_EDITMAPLOAD:
		{
			if(m_editStartupDone)
			{
				return;
			}

			EditMapLoadStartup();

			break;
		}
	}
}

// -----------------------------------------------------------------------
// Render
// -----------------------------------------------------------------------
void Game::Render()
{
	PROFILE_FUNCTION();

	PreRenderStateStartup();

	switch(m_gameState)
	{
		case GAMESTATE_INIT:
		{
			InitRender();
			break;
		}

		case GAMESTATE_LOADING:
		{
			LoadingTitleRender();
			break;
		}

		case GAMESTATE_TITLE:
		{
			TitleRender();
			break;
		}

		case GAMESTATE_LOBBY:
		{
			LobbyRender();
			break;
		}

		case GAMESTATE_PLAYMAPLOAD:
		{
			PlayMapLoadRender();
			break;
		}

		case GAMESTATE_EDITMAPLOAD:
		{
			EditMapLoadRender();
			break;
		}

		case GAMESTATE_PLAY:
		{
			PlayRender();

			if(m_isPaused)
			{
				PauseRender();
			}
			break;
		}

		case GAMESTATE_EDIT:
		{
			EditRender();

			if(m_isPaused)
			{
				PauseRender();
			}
			break;
		}
	}
}

// -----------------------------------------------------------------------
void Game::StartLoadingTexture( std::string nameOfTexture )
{
	ImageLoading loading = ImageLoading(nameOfTexture);

	m_objectLoading++; 
	imageLoadingFromDiscQueue.Enqueue( loading );
}

// -----------------------------------------------------------------------
void Game::StartLoadingGPUMesh( std::string nameOfGPUMesh )
{
	CPUMeshLoading loading = CPUMeshLoading(nameOfGPUMesh);

	m_objectLoading++; 
	cpuLoadingFromDiscQueue.Enqueue( loading );
}

// -----------------------------------------------------------------------
void Game::ImageAndMeshLoadThread()
{
	bool doneLoading = false;

	ImageLoading imageLoading;
	CPUMeshLoading cpuLoading;

	while(!doneLoading)
	{
		bool gotImageWork = imageLoadingFromDiscQueue.Dequeue(&imageLoading);
		if(gotImageWork)
		{
			imageLoading.image = new Image(imageLoading.imageName.c_str());
			imageCreatingTextureQueue.Enqueue(imageLoading);
		}

		bool gotCPUWork = cpuLoadingFromDiscQueue.Dequeue(&cpuLoading);
		if(gotCPUWork)
		{
			cpuLoading.cpuMesh = new CPUMesh();
			cpuLoading.cpuMesh->SetLayout<Vertex_Lit>();
			CreateMeshFromFile(cpuLoading.meshName.c_str(), cpuLoading.cpuMesh);
			cpuCreatingGPUMeshQueue.Enqueue(cpuLoading);
		}

		if(!gotImageWork && !gotCPUWork)
		{
			doneLoading = true;
		}

		Sleep(0);
	}
}

// -----------------------------------------------------------------------
void Game::EndLoadingThreads()
{
	for (std::thread& thread: m_threads) 
	{
		thread.join(); 
	}

	m_threads.clear();
}

// -----------------------------------------------------------------------
void Game::TitleStartup()
{
	// Create Parent Widget;
	m_titleWidget = new UIWidget();

	m_titleWidget->SetWorldBounds(AABB2::MakeFromMinsMaxs(m_clientMins, m_clientMaxs));
	m_titleWidget->UpdateBounds();
	
	// Create Button Widget that is a Child of Parent Widget;
	m_titlePlayButton = new UIButton(m_titleWidget);
	m_titlePlayButton->SetVirtualPosition(Vec4(0.8f, 0.7f, 0.0f, 0.0f));
	m_titlePlayButton->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_titlePlayButton->UpdateBounds();
	m_titlePlayButton->SetAction("lobbyload");
	m_titleWidget->AddChild(static_cast<UIWidget*>(m_titlePlayButton));

	// Create Label Widget that is a Child of Parent Widget;
	m_titlePlayLabel = new UILabel(m_titleWidget);
	m_titlePlayLabel->SetVirtualPosition(Vec4(0.8f, 0.7f, 0.0f, 0.0f));
	m_titlePlayLabel->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_titlePlayLabel->UpdateBounds();
	m_titlePlayLabel->SetLabel(" ");
	m_titlePlayLabel->SetFont(m_squirrelFont);
	m_titleWidget->AddChild(static_cast<UIWidget*>(m_titlePlayLabel));

	// Create Button Widget that is a Child of Parent Widget;
	m_editPlayButton = new UIButton(m_titleWidget);
	m_editPlayButton->SetVirtualPosition(Vec4(0.8f, 0.5f, 0.0f, 0.0f));
	m_editPlayButton->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_editPlayButton->UpdateBounds();
	m_editPlayButton->SetAction("editmapload");
	m_titleWidget->AddChild(static_cast<UIWidget*>(m_editPlayButton));

	// Create Label Widget that is a Child of Parent Widget;
	m_editPlayLabel = new UILabel(m_titleWidget);
	m_editPlayLabel->SetVirtualPosition(Vec4(0.8f, 0.5f, 0.0f, 0.0f));
	m_editPlayLabel->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_editPlayLabel->UpdateBounds();
	m_editPlayLabel->SetLabel(" ");
	m_editPlayLabel->SetFont(m_squirrelFont);
	m_titleWidget->AddChild(static_cast<UIWidget*>(m_editPlayLabel));

	// Create Button Widget that is a Child of Parent Widget;
	m_quitPlayButton = new UIButton(m_titleWidget);
	m_quitPlayButton->SetVirtualPosition(Vec4(0.8f, 0.3f, 0.0f, 0.0f));
	m_quitPlayButton->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_quitPlayButton->UpdateBounds();
	m_quitPlayButton->SetAction("quit");
	m_titleWidget->AddChild(static_cast<UIWidget*>(m_quitPlayButton));

	// Create Label Widget that is a Child of Parent Widget;
	m_quitPlayLabel = new UILabel(m_titleWidget);
	m_quitPlayLabel->SetVirtualPosition(Vec4(0.8f, 0.3f, 0.0f, 0.0f));
	m_quitPlayLabel->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_quitPlayLabel->UpdateBounds();
	m_quitPlayLabel->SetLabel(" ");
	m_quitPlayLabel->SetFont(m_squirrelFont);
	m_titleWidget->AddChild(static_cast<UIWidget*>(m_quitPlayLabel));	

	m_titleStartupDone = true;
}

void Game::LobbyStartup()
{
	// Create Parent Widget;
	m_lobbyWidget = new UIWidget();

	m_lobbyWidget->SetWorldBounds(AABB2::MakeFromMinsMaxs(m_clientMins, m_clientMaxs));
	m_lobbyWidget->UpdateBounds();

	// Create Button Widget that is a Child of Parent Widget;
	m_lobbyPlayButton = new UIButton(m_lobbyWidget);
	m_lobbyPlayButton->SetVirtualPosition(Vec4(0.8f, 0.7f, 0.0f, 0.0f));
	m_lobbyPlayButton->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_lobbyPlayButton->UpdateBounds();
	m_lobbyPlayButton->SetAction("playmapload");
	m_lobbyWidget->AddChild(static_cast<UIWidget*>(m_lobbyPlayButton));

	// Create Label Widget that is a Child of Parent Widget;
	m_lobbyPlayLabel = new UILabel(m_lobbyWidget);
	m_lobbyPlayLabel->SetVirtualPosition(Vec4(0.8f, 0.7f, 0.0f, 0.0f));
	m_lobbyPlayLabel->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_lobbyPlayLabel->UpdateBounds();
	m_lobbyPlayLabel->SetLabel(" ");
	m_lobbyPlayLabel->SetFont(m_squirrelFont);
	m_lobbyWidget->AddChild(static_cast<UIWidget*>(m_lobbyPlayLabel));

	// Create Button Widget that is a Child of Parent Widget;
	m_lobbyBackButton = new UIButton(m_lobbyWidget);
	m_lobbyBackButton->SetVirtualPosition(Vec4(0.8f, 0.5f, 0.0f, 0.0f));
	m_lobbyBackButton->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_lobbyBackButton->UpdateBounds();
	m_lobbyBackButton->SetAction("backtotitle");
	m_lobbyWidget->AddChild(static_cast<UIWidget*>(m_lobbyBackButton));

	// Create Label Widget that is a Child of Parent Widget;
	m_lobbyBackLabel = new UILabel(m_lobbyWidget);
	m_lobbyBackLabel->SetVirtualPosition(Vec4(0.8f, 0.5f, 0.0f, 0.0f));
	m_lobbyBackLabel->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_lobbyBackLabel->UpdateBounds();
	m_lobbyBackLabel->SetLabel(" ");
	m_lobbyBackLabel->SetFont(m_squirrelFont);
	m_lobbyWidget->AddChild(static_cast<UIWidget*>(m_lobbyBackLabel));



	m_lobbyStartupDone = true;
}

void Game::PlayMapLoadStartup()
{
	if(!m_pauseWidget)
	{
		LoadPauseWidgets();
	}
	
	// Create Parent Widget;
	m_playWidget = new UIWidget();

	m_playWidget->SetWorldBounds(AABB2::MakeFromMinsMaxs(m_clientMins, m_clientMaxs));
	m_playWidget->UpdateBounds();

	// Load Task UI Widgets;
	//LoadTaskWidgets();

	m_playStartupDone = true;
}

void Game::EditMapLoadStartup()
{
	if(!m_pauseWidget)
	{
		LoadPauseWidgets();
	}

	// Create Parent Widget;
	m_editWidget = new UIWidget();
	m_editRadioGroup = new UIRadioGroup();

	m_editWidget->SetWorldBounds(AABB2::MakeFromMinsMaxs(m_clientMins, m_clientMaxs));
	m_editWidget->UpdateBounds();

	// Create Button Widget that is a Child of Parent Widget;
	m_editTexture1Button = new UIButton(m_editWidget);
	m_editTexture1Button->SetVirtualPosition(Vec4(0.06f, 0.92f, 0.0f, 0.0f));
	m_editTexture1Button->SetVirtualSize(Vec4(0.0f, 0.0f, 50.0f, 50.0f));
	m_editTexture1Button->UpdateBounds();
	m_editTexture1Button->SetAction("dirttexture");
	m_editRadioGroup->AddButtonToRadioGroup(m_editTexture1Button);
	m_editTexture1Button->SetRadioGroup(m_editRadioGroup);
	m_editWidget->AddChild(static_cast<UIWidget*>(m_editTexture1Button));

	// Create Label Widget that is a Child of Parent Widget;
	m_editTexture1Label = new UILabel(m_editWidget);
	m_editTexture1Label->SetVirtualPosition(Vec4(0.06f, 0.92f, 0.0f, 0.0f));
	m_editTexture1Label->SetVirtualSize(Vec4(0.0f, 0.0f, 50.0f, 50.0f));
	m_editTexture1Label->UpdateBounds();
	m_editTexture1Label->SetLabel("1");
	m_editTexture1Label->SetFont(m_squirrelFont);
	m_editTexture1Label->SetTextureView(g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/Terrain/Dirt_DIFFU.png"));
	m_editWidget->AddChild(static_cast<UIWidget*>(m_editTexture1Label));

	// Create Button Widget that is a Child of Parent Widget;
	m_editTexture2Button = new UIButton(m_editWidget);
	m_editTexture2Button->SetVirtualPosition(Vec4(0.18f, 0.92f, 0.0f, 0.0f));
	m_editTexture2Button->SetVirtualSize(Vec4(0.0f, 0.0f, 50.0f, 50.0f));
	m_editTexture2Button->UpdateBounds();
	m_editTexture2Button->SetAction("dirtstonetexture");
	m_editRadioGroup->AddButtonToRadioGroup(m_editTexture2Button);
	m_editTexture2Button->SetRadioGroup(m_editRadioGroup);
	m_editWidget->AddChild(static_cast<UIWidget*>(m_editTexture2Button));

	// Create Label Widget that is a Child of Parent Widget;
	m_editTexture2Label = new UILabel(m_editWidget);
	m_editTexture2Label->SetVirtualPosition(Vec4(0.18f, 0.92f, 0.0f, 0.0f));
	m_editTexture2Label->SetVirtualSize(Vec4(0.0f, 0.0f, 50.0f, 50.0f));
	m_editTexture2Label->UpdateBounds();
	m_editTexture2Label->SetLabel("2");
	m_editTexture2Label->SetFont(m_squirrelFont);
	m_editTexture2Label->SetTextureView(g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/Terrain/Dirt_Stone_DIFFU.png"));
	m_editWidget->AddChild(static_cast<UIWidget*>(m_editTexture2Label));

	// Create Button Widget that is a Child of Parent Widget;
	m_editTexture3Button = new UIButton(m_editWidget);
	m_editTexture3Button->SetVirtualPosition(Vec4(0.3f, 0.92f, 0.0f, 0.0f));
	m_editTexture3Button->SetVirtualSize(Vec4(0.0f, 0.0f, 50.0f, 50.0f));
	m_editTexture3Button->UpdateBounds();
	m_editTexture3Button->SetAction("grasstexture");
	m_editRadioGroup->AddButtonToRadioGroup(m_editTexture3Button);
	m_editTexture3Button->SetRadioGroup(m_editRadioGroup);
	m_editWidget->AddChild(static_cast<UIWidget*>(m_editTexture3Button));

	// Create Label Widget that is a Child of Parent Widget;
	m_editTexture3Label = new UILabel(m_editWidget);
	m_editTexture3Label->SetVirtualPosition(Vec4(0.3f, 0.92f, 0.0f, 0.0f));
	m_editTexture3Label->SetVirtualSize(Vec4(0.0f, 0.0f, 50.0f, 50.0f));
	m_editTexture3Label->UpdateBounds();
	m_editTexture3Label->SetLabel("3");
	m_editTexture3Label->SetFont(m_squirrelFont);
	m_editTexture3Label->SetTextureView(g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/Terrain/Grass_DIFFU.png"));
	m_editWidget->AddChild(static_cast<UIWidget*>(m_editTexture3Label));

	// Create Button Widget that is a Child of Parent Widget;
	m_editTexture4Button = new UIButton(m_editWidget);
	m_editTexture4Button->SetVirtualPosition(Vec4(0.42f, 0.92f, 0.0f, 0.0f));
	m_editTexture4Button->SetVirtualSize(Vec4(0.0f, 0.0f, 50.0f, 50.0f));
	m_editTexture4Button->UpdateBounds();
	m_editTexture4Button->SetAction("grassrubbletexture");
	m_editRadioGroup->AddButtonToRadioGroup(m_editTexture4Button);
	m_editTexture4Button->SetRadioGroup(m_editRadioGroup);
	m_editWidget->AddChild(static_cast<UIWidget*>(m_editTexture4Button));

	// Create Label Widget that is a Child of Parent Widget;
	m_editTexture4Label = new UILabel(m_editWidget);
	m_editTexture4Label->SetVirtualPosition(Vec4(0.42f, 0.92f, 0.0f, 0.0f));
	m_editTexture4Label->SetVirtualSize(Vec4(0.0f, 0.0f, 50.0f, 50.0f));
	m_editTexture4Label->UpdateBounds();
	m_editTexture4Label->SetLabel("4");
	m_editTexture4Label->SetFont(m_squirrelFont);
	m_editTexture4Label->SetTextureView(g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/Terrain/GrassRubble_DIFFU.png"));
	m_editWidget->AddChild(static_cast<UIWidget*>(m_editTexture4Label));

	// Create Button Widget that is a Child of Parent Widget;
	m_editTexture5Button = new UIButton(m_editWidget);
	m_editTexture5Button->SetVirtualPosition(Vec4(0.54f, 0.92f, 0.0f, 0.0f));
	m_editTexture5Button->SetVirtualSize(Vec4(0.0f, 0.0f, 50.0f, 50.0f));
	m_editTexture5Button->UpdateBounds();
	m_editTexture5Button->SetAction("stonetexture");
	m_editRadioGroup->AddButtonToRadioGroup(m_editTexture5Button);
	m_editTexture5Button->SetRadioGroup(m_editRadioGroup);
	m_editWidget->AddChild(static_cast<UIWidget*>(m_editTexture5Button));

	// Create Label Widget that is a Child of Parent Widget;
	m_editTexture5Label = new UILabel(m_editWidget);
	m_editTexture5Label->SetVirtualPosition(Vec4(0.54f, 0.92f, 0.0f, 0.0f));
	m_editTexture5Label->SetVirtualSize(Vec4(0.0f, 0.0f, 50.0f, 50.0f));
	m_editTexture5Label->UpdateBounds();
	m_editTexture5Label->SetLabel("5");
	m_editTexture5Label->SetFont(m_squirrelFont);
	m_editTexture5Label->SetTextureView(g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/Terrain/Stone_DIFFU1.png"));
	m_editWidget->AddChild(static_cast<UIWidget*>(m_editTexture5Label));



	m_editStartupDone = true;
}

// -----------------------------------------------------------------------
void Game::LoadPlayModels()
{
	

	
}

// -----------------------------------------------------------------------
void Game::LoadEditModels()
{
	
}

// -----------------------------------------------------------------------
void Game::InitRender()
{
	// Put the Title Screen centered to Camera World Space (1920x1080);
	m_titleModel.SetT(Vec3(0.0f, 0.0f, 0.0f));

	// Get and Set the Render Target View for the Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_uiCamera->SetColorTargetView( m_colorTargetView );

	// Begin Camera with the UICamera;
	g_theRenderer->BeginCamera( m_uiCamera );
	g_theRenderer->ClearColorTargets( m_clearColor );
	g_theRenderer->ClearDepthStencilTarget();

	// Draw the Title Screen;
	g_theRenderer->BindModelMatrix(m_titleModel);
	g_theRenderer->BindShader(g_theRenderer->GetOrCreateShader( "Data/Shaders/default_unlit_devconsole.shader" ));
	g_theRenderer->BindTextureViewWithSampler(0u, m_loadingScreen);
	g_theRenderer->DrawMesh(m_titleMesh);	

	g_theRenderer->EndCamera();
}

// -----------------------------------------------------------------------
void Game::TitleRender()
{
	// Put the Title Screen centered to Camera World Space (1920x1080);
	m_titleModel.SetT(Vec3(0.0f, 0.0f, 0.0f));

	// Get and Set the Render Target View for the Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_uiCamera->SetColorTargetView( m_colorTargetView );

	// Begin Camera with the UICamera;
	g_theRenderer->BeginCamera( m_uiCamera );
	g_theRenderer->ClearColorTargets( m_clearColor );
	g_theRenderer->ClearDepthStencilTarget();

	// Draw the Title Screen;
	g_theRenderer->BindModelMatrix(m_titleModel);
	g_theRenderer->BindShader(g_theRenderer->GetOrCreateShader( "Data/Shaders/default_unlit_devconsole.shader" ));
	g_theRenderer->BindTextureViewWithSampler(0u, m_titleScreen);
	g_theRenderer->DrawMesh(m_titleMesh);

	// Render UI Widgets for the Title Screen;
	m_titleWidget->Render();

	g_theRenderer->EndCamera();
}

// -----------------------------------------------------------------------
void Game::LobbyRender()
{
	// Put the Title Screen centered to Camera World Space (1920x1080);
	m_lobbyModel.SetT(Vec3(0.0f, 0.0f, 0.0f));

	// Get and Set the Render Target View for the Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_uiCamera->SetColorTargetView( m_colorTargetView );

	// Begin Camera with the UICamera;
	g_theRenderer->BeginCamera( m_uiCamera );
	g_theRenderer->ClearColorTargets( m_clearColor );
	g_theRenderer->ClearDepthStencilTarget();

	// Draw the Title Screen;
	g_theRenderer->BindModelMatrix(m_lobbyModel);
	g_theRenderer->BindShader(g_theRenderer->GetOrCreateShader( "Data/Shaders/default_unlit_devconsole.shader" ));
	g_theRenderer->BindTextureViewWithSampler(0u, m_lobbyScreen);
	g_theRenderer->DrawMesh(m_titleMesh);

	// Render UI Widgets for the Title Screen;
	m_lobbyWidget->Render();

	// Draw Lobby Players;
	DrawLobbyPlayers();
	

	g_theRenderer->EndCamera();

	// Draw the Lobby Console;
	g_theLobbyConsole->Render();
}

// -----------------------------------------------------------------------
void Game::PlayMapLoadRender()
{
	// Put the Title Screen centered to Camera World Space (1920x1080);
	m_titleModel.SetT(Vec3(0.0f, 0.0f, 0.0f));

	// Get and Set the Render Target View for the Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_uiCamera->SetColorTargetView( m_colorTargetView );

	// Begin Camera with the UICamera;
	g_theRenderer->BeginCamera( m_uiCamera );
	g_theRenderer->ClearColorTargets( m_clearColor );
	g_theRenderer->ClearDepthStencilTarget();

	// Draw the Title Screen;
	g_theRenderer->BindModelMatrix(m_titleModel);
	g_theRenderer->BindShader(g_theRenderer->GetOrCreateShader( "Data/Shaders/default_unlit_devconsole.shader" ));
	g_theRenderer->BindTextureViewWithSampler(0u, m_playMapLoading);
	g_theRenderer->DrawMesh(m_titleMesh);

	g_theRenderer->EndCamera();	
}

// -----------------------------------------------------------------------
void Game::EditMapLoadRender()
{
	// Put the Title Screen centered to Camera World Space (1920x1080);
	m_titleModel.SetT(Vec3(0.0f, 0.0f, 0.0f));

	// Get and Set the Render Target View for the Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_uiCamera->SetColorTargetView( m_colorTargetView );

	// Begin Camera with the UICamera;
	g_theRenderer->BeginCamera( m_uiCamera );
	g_theRenderer->ClearColorTargets( m_clearColor );
	g_theRenderer->ClearDepthStencilTarget();

	// Draw the Title Screen;
	g_theRenderer->BindModelMatrix(m_titleModel);
	g_theRenderer->BindShader(g_theRenderer->GetOrCreateShader( "Data/Shaders/default_unlit_devconsole.shader" ));
	g_theRenderer->BindTextureViewWithSampler(0u, m_editMapLoading);
	g_theRenderer->DrawMesh(m_titleMesh);

	g_theRenderer->EndCamera();
}

// -----------------------------------------------------------------------
void Game::PlayRender()
{
	// Get and Set the Render Target View for the Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_gameMainCamera->SetColorTargetView( m_colorTargetView );

	// World stuff
	g_theRenderer->BeginCamera( m_gameMainCamera );
	g_theRenderer->ClearColorTargets( m_clearColor );
	g_theRenderer->ClearDepthStencilTarget();

	// Light;
	g_theRenderer->SetAmbientLight( Rgba::WHITE, 1.0f );

	// Render Map;
	m_map->Render();

	// Debug Render;
	//g_theDebugRenderer->CreateDebugRenderPoint( -1.0f, DEBUG_RENDER_USE_DEPTH, m_focalPoint, Rgba::WHITE, Rgba::WHITE, 0.1f );
	//g_theDebugRenderer->CreateDebugRenderPoint( -1.0f, DEBUG_RENDER_USE_DEPTH, g_thePlayerController->GetTerrainSpot(), Rgba::RED, Rgba::RED, 0.1f );
	//g_theDebugRenderer->RenderToCamera();	
	//g_theDebugRenderer->RenderToScreen(m_uiCamera);
	

	g_theRenderer->EndCamera();

	// Get and Set the Render Target View for the Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_uiCamera->SetColorTargetView( m_colorTargetView );

	// Begin Camera with the UICamera;
	g_theRenderer->BeginCamera( m_uiCamera );

	// Render UI Widgets for the Edit Screen;
	//g_theRenderer->BindShader(g_theRenderer->GetOrCreateShader( "Data/Shaders/default_unlit_devconsole.shader" ));
	m_playWidget->Render();
	//m_playTaskWidget->Render();

	// Render the Selection Box;
	RenderSelectionBox();	

	std::vector<Vertex_PCU> verts;
	BitMapFont* bitmapFont = g_theRenderer->CreateOrGetBitmapFont("SquirrelFixedFont");
	TextureView* textureViewFont = bitmapFont->GetTextureView();
	m_map->RenderScreenSelectedUnits(AABB2::MakeFromMinsMaxs(m_clientMins, m_clientMaxs), &verts);	
	m_map->RenderScreenSelectedTasks(AABB2::MakeFromMinsMaxs(m_clientMins, m_clientMaxs), &verts);	
	m_map->RenderScreenResourceInfo(AABB2::MakeFromMinsMaxs(m_clientMins, m_clientMaxs), &verts);


	if((int)verts.size() > 0)
	{
		g_theRenderer->BindTextureView( 0u, textureViewFont );
		g_theRenderer->DrawVertexArray((int)verts.size(), &verts[0]);
	}

	g_theRenderer->EndCamera();
}

// -----------------------------------------------------------------------
void Game::EditRender()
{
	// Get and Set the Render Target View for the Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_gameMainCamera->SetColorTargetView( m_colorTargetView );

	// World stuff
	g_theRenderer->BeginCamera( m_gameMainCamera );
	g_theRenderer->ClearColorTargets( m_clearColor );
	g_theRenderer->ClearDepthStencilTarget();

	// Light;
	g_theRenderer->SetAmbientLight( Rgba::WHITE, 1.0f );

	// Render Map;
	m_map->Render();

	// Debug Render;
	g_theDebugRenderer->CreateDebugRenderPoint( -1.0f, DEBUG_RENDER_USE_DEPTH, m_focalPoint, Rgba::WHITE, Rgba::WHITE, 0.1f );
	g_theDebugRenderer->RenderToCamera();
	

	g_theRenderer->EndCamera();

	// Get and Set the Render Target View for the Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_uiCamera->SetColorTargetView( m_colorTargetView );

	// Begin Camera with the UICamera;
	g_theRenderer->BeginCamera( m_uiCamera );

	g_theDebugRenderer->RenderToScreen(m_uiCamera);

	// Render UI Widgets for the Edit Screen;
	m_editWidget->Render();

	g_theRenderer->EndCamera();
}

void Game::PauseRender()
{	
	float percent = m_fadeTimer / m_fadeTime;
	float intensity = Lerp(0.0f, 1.0f, percent);	

	tonemap_buffer_t buffer;
	buffer.intensity = intensity;

	m_tonemapMaterial->SetUniforms(&buffer, sizeof(buffer));

	g_theRenderer->ApplyEffect(m_tonemapMaterial);

	// Get and Set the Render Target View for the Camera;
	if(intensity != 1.0f)
	{
		return;
	}
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_uiCamera->SetColorTargetView( m_colorTargetView );
	Shader* shader = g_theRenderer->GetOrCreateShader( "Data/Shaders/default_unlit_devconsole.shader" );
	g_theRenderer->BindShader(shader);

	// Begin Camera with the UICamera;
	g_theRenderer->BeginCamera( m_uiCamera );

	// Render UI Widgets for the Edit Screen;
	m_pauseWidget->Render();

	g_theRenderer->EndCamera();	
}

// -----------------------------------------------------------------------
// Selection Box
// -----------------------------------------------------------------------
void Game::RenderSelectionBox()
{
	Vec2 selectionBoxStart = g_thePlayerController->GetMouseSelectionBoxStart();
	Vec2 selectionBoxEnd = g_thePlayerController->GetMouseSelectionBoxEnd();
	if(selectionBoxStart != Vec2::VNULL && selectionBoxEnd != Vec2::VNULL)
	{


		Vec2 bottomLeft = Vec2(GetMin(selectionBoxStart.x, selectionBoxEnd.x), GetMin(selectionBoxStart.y, selectionBoxEnd.y));
		Vec2 bottomRight = Vec2(GetMax(selectionBoxStart.x, selectionBoxEnd.x), GetMin(selectionBoxStart.y, selectionBoxEnd.y));
		Vec2 topLeft = Vec2(GetMin(selectionBoxStart.x, selectionBoxEnd.x), GetMax(selectionBoxStart.y, selectionBoxEnd.y));
		Vec2 topRight = Vec2(GetMax(selectionBoxStart.x, selectionBoxEnd.x), GetMax(selectionBoxStart.y, selectionBoxEnd.y));

		AABB2 selectionBox = AABB2::MakeFromMinsMaxs(bottomLeft, topRight);
		g_theRenderer->BindShader(g_theRenderer->GetOrCreateShader("Data/Shaders/default_unlit_devconsole.shader"));
		g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->m_blankGreenTexture);

		std::vector<Vertex_PCU> quadVerts;			
		AddVertsForAABB2D(quadVerts, selectionBox, Rgba::WHITE);
		g_theRenderer->DrawVertexArray((int)quadVerts.size(), &quadVerts[0]);

		g_theRenderer->BindShader(g_theRenderer->GetOrCreateShader("Data/Shaders/DefaultLit.00.shader"));
		g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->m_blankGreenTexture);

		std::vector<Vertex_PCU> lineVerts;
		AddVertsForLine2D( lineVerts, bottomLeft, bottomRight, 2.0f, Rgba::WHITE );
		AddVertsForLine2D( lineVerts, bottomRight, topRight, 2.0f, Rgba::WHITE );
		AddVertsForLine2D( lineVerts, topRight, topLeft, 2.0f, Rgba::WHITE );
		AddVertsForLine2D( lineVerts, topLeft, bottomLeft, 2.0f, Rgba::WHITE );		
		g_theRenderer->DrawVertexArray((int)lineVerts.size(), &lineVerts[0]);
	}
}

// -----------------------------------------------------------------------
// Update
// -----------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	PROFILE_FUNCTION();

	switch(m_gameState)
	{
		case GAMESTATE_INIT:
		{


			break;
		}

		case GAMESTATE_LOADING:
		{
			ContinueLoading();

			bool done = DoneLoading();
			if(done)
			{
				EndLoadingThreads();
				SetGameAssets();
				m_loadEndTime = GetCurrentTimeSeconds();
				double loadTime = (m_loadEndTime - m_loadStartTime ) * 1000.0;
				DebuggerPrintf("GameLoad: %f ms.\n", loadTime);
				SetGameState(GAMESTATE_TITLE);
			}

			break;
		}

		case GAMESTATE_TITLE:
		{
			TitleUpdate(deltaSeconds);
			break;
		}

		case GAMESTATE_LOBBY:
		{
			LobbyUpdate(deltaSeconds);
			break;
		}

		case GAMESTATE_PLAYMAPLOAD:
		{
			PlayMapLoadUpdate();
			break;
		}

		case GAMESTATE_EDITMAPLOAD:
		{
			EditMapLoadUpdate();
			break;
		}

		case GAMESTATE_PLAY:
		{
			PlayUpdate( deltaSeconds );
			break;
		}

		case GAMESTATE_EDIT:
		{
			EditUpdate( deltaSeconds );
			break;
		}
	}

	CheckAndChangeGameState();	
}

// -----------------------------------------------------------------------
void Game::EndFrame()
{
	if(m_gameState == GAMESTATE_PLAY)
	{
		m_map->EndFrame();
	}
}

// -----------------------------------------------------------------------
void Game::ContinueLoading()
{
	ImageLoading imageLoading;
	CPUMeshLoading cpuLoading;

	bool gotImageWork = imageCreatingTextureQueue.Dequeue(&imageLoading);
	if(gotImageWork)
	{
		g_theRenderer->CreateTextureViewFromImage(imageLoading.image, imageLoading.imageName);
		m_objectLoading--;
		delete imageLoading.image;
		imageLoading.image = nullptr;
	}

	bool gotGPUWork = cpuCreatingGPUMeshQueue.Dequeue(&cpuLoading);
	if(gotGPUWork)
	{
		g_theRenderer->CreateAndRegisterGPUMesh(cpuLoading.cpuMesh, cpuLoading.meshName);
		m_objectLoading--;
		delete cpuLoading.cpuMesh;
		cpuLoading.cpuMesh = nullptr;
	}

}

// -----------------------------------------------------------------------
bool Game::DoneLoading()
{
	return m_objectLoading == 0;
}

// -----------------------------------------------------------------------
void Game::LoadingTitleRender()
{
	// Put the Title Screen centered to Camera World Space (1920x1080);
	m_titleModel.SetT(Vec3(0.0f, 0.0f, 0.0f));

	// Get and Set the Render Target View for the Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_uiCamera->SetColorTargetView( m_colorTargetView );

	// Begin Camera with the UICamera;
	g_theRenderer->BeginCamera( m_uiCamera );
	g_theRenderer->ClearColorTargets( m_clearColor );
	g_theRenderer->ClearDepthStencilTarget();

	// Draw the Title Screen;
	g_theRenderer->BindModelMatrix(m_titleModel);
	g_theRenderer->BindShader(g_theRenderer->GetOrCreateShader( "Data/Shaders/default_unlit_devconsole.shader" ));
	g_theRenderer->BindTextureViewWithSampler(0u, m_loadingScreen);
	g_theRenderer->DrawMesh(m_titleMesh);	


	// Black;
	m_uiCamera->SetColorTargetView(g_theRenderer->GetFrameColorTarget());
	g_theRenderer->BindModelMatrix(Matrix4x4::IDENTITY);
	g_theRenderer->BindTextureViewWithSampler(0u, nullptr);
	
	float start = 400.0f;
	float startHeight = 5.0f;
	float height = 20.0f;
	float width = 1318.0f;
	float loadingRatio = 1.0f - ((float)m_objectLoading / (float)m_objectsToLoad);
	float progressWidth1 = width * loadingRatio;
	float progressWidth = RangeMap(progressWidth1, 0.0f, width, 400.0f, width);

	std::vector<Vertex_PCU> barVerts;

	AABB2 barBackground = AABB2::MakeFromMinsMaxs(Vec2(start, startHeight), Vec2(width, height));
	AddVertsForAABB2D(barVerts, barBackground, Rgba::BLACK);
	
	AABB2 loadingBar = AABB2::MakeFromMinsMaxs(Vec2(start, startHeight), Vec2(progressWidth, height));
	AddVertsForAABB2D(barVerts, loadingBar, Rgba::WHITE);

	g_theRenderer->DrawVertexArray( barVerts );


	g_theRenderer->EndCamera();
}

// -----------------------------------------------------------------------
void Game::TitleUpdate(float deltaSeconds)
{
	PROFILE_FUNCTION();

	if(!m_mainMenuMusicPlaying)
	{
		StopAllMusic();
		m_mainMenuMusicPlaying = true;

		ChannelGroupID bgmGroup = g_theAudioSystem->CreateOrGetChannelGroup("BGM");
		SoundID mainMenuMusic = g_theAudioSystem->CreateOrGetSound( "Data/Audio/BGM/mainmenu.mp3" );
		m_mainMenuMusicPlaybackID = g_theAudioSystem->PlaySound( mainMenuMusic, bgmGroup, true );
	}

	if(m_map)
	{
		delete m_map;
		m_map = nullptr;
	}

	g_thePlayerController->UpdateInput( deltaSeconds );
}

// -----------------------------------------------------------------------
void Game::LobbyUpdate( float deltaSeconds )
{
	g_thePlayerController->UpdateInput( deltaSeconds );

	// Setup Connection Information
	SetupConnectionInformation();
}

// -----------------------------------------------------------------------
void Game::PlayMapLoadUpdate()
{
	LogSeconds log("PlayMapLoadUpdate: ", 0.0);
	if(!m_playMapLoadDone)
	{
		// Models;
		LoadPlayModels();

		// Load in a Map from a file;
		m_map = new Map(this);
		m_map->Load("play", 16, 16);

		// Game will also load information about the map that it needs;
		// Starting Focal Point;
		m_focalPoint = Vec3(3.5f, 3.5f, 0.0f);

		// Replay;
		if(m_readFromReplay)
		{
			g_theReplayController->LoadReplayFile(m_replayFilename);
		}

		m_playMapLoadDone = true;
	}	

	if(!m_isConnected)
	{
		SetGameState(GAMESTATE_PLAY);

		g_theApp->m_commandFrame.m_commandFrameNumber = 0u;
	}
	else if(m_isConnected && m_connectedPlayMapLoadDone && m_connectedPlayMapLoadSent)
	{
		SetGameState(GAMESTATE_PLAY);

		g_theApp->m_commandFrame.m_commandFrameNumber = 0u;
	}
	else if(m_isConnected && (!m_connectedPlayMapLoadDone || !m_connectedPlayMapLoadSent))
	{
		g_theRakNetInterface->SendPlayMapLoadComplete(g_theRakNetInterface->m_themSystemAddress);
		m_connectedPlayMapLoadSent = true;
	}
	
}

// -----------------------------------------------------------------------
void Game::EditMapLoadUpdate()
{
	// Load in a Map from a file;
	m_map = new Map(this);
	m_map->Load("edit", 16, 16);

	// Game will also load information about the map that it needs;
	// Starting Focal Point;
	m_focalPoint = Vec3(3.5f, 3.5f, 0.0f);

	// Models;
	LoadEditModels();

	SetGameState(GAMESTATE_EDIT);
}

// -----------------------------------------------------------------------
void Game::PlayUpdate( float deltaSeconds )
{
	PROFILE_FUNCTION();

	if(!m_playMusicPlaying)
	{
		StopAllMusic();
		m_playMusicPlaying = true;

		g_theAudioSystem->StopSound(m_mainMenuMusicPlaybackID);
		ChannelGroupID bgmGroup = g_theAudioSystem->CreateOrGetChannelGroup("BGM");
		SoundID playMusic = g_theAudioSystem->CreateOrGetSound( "Data/Audio/BGM/gameplay.mp3" );
		m_playMusicPlaybackID = g_theAudioSystem->PlaySound( playMusic, bgmGroup, true );
	}

	g_thePlayerController->UpdateInput( deltaSeconds );
	
	m_isPaused = g_thePlayerController->IsPaused();
	
	if(m_isPaused)
	{
		m_fadeTimer += deltaSeconds;
		m_fadeTimer = Clamp(m_fadeTimer, 0.0f, m_fadeTime);
		return;
	}
	else
	{
		m_fadeTimer = 0.0f;
	}
	
	if(m_isConnected)
	{
		ProcessNetworkCommands();
	}
	else
	{
		g_theAIController->UpdateThoughts();
		ProcessCommands();
	}

	m_map->Update(deltaSeconds);

	UpdateGameCamera( deltaSeconds );

	m_myCommandFrameComplete = g_theApp->m_commandFrame.GetCommandFrame();
	m_myHashedFrame = m_map->HashEntities();
	if(m_isConnected)
	{
		g_theRakNetInterface->SendPlayMapEndOfFrameComplete(g_theRakNetInterface->m_themSystemAddress, m_myCommandFrameComplete, m_myHashedFrame);
	}
}

// -----------------------------------------------------------------------
void Game::EditUpdate( float deltaSeconds )
{
	g_thePlayerController->UpdateInput( deltaSeconds );
	m_isPaused = g_thePlayerController->IsPaused();

	if(m_isPaused)
	{
		m_fadeTimer += deltaSeconds;
		m_fadeTimer = Clamp(m_fadeTimer, 0.0f, m_fadeTime);
		return;
	}
	else
	{
		m_fadeTimer = 0.0f;
	}

	m_map->Update(deltaSeconds);

	UpdateGameCamera( deltaSeconds );
}

void Game::LoadPauseWidgets()
{
	// Create Parent Widget;
	m_pauseWidget = new UIWidget();

	m_pauseWidget->SetWorldBounds(AABB2::MakeFromMinsMaxs(m_clientMins, m_clientMaxs));
	m_pauseWidget->UpdateBounds();

	// Create Button Widget that is a Child of Parent Widget;
	m_pauseResumeButton = new UIButton(m_pauseWidget);
	m_pauseResumeButton->SetVirtualPosition(Vec4(0.5f, 0.70f, 0.0f, 0.0f));
	m_pauseResumeButton->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_pauseResumeButton->UpdateBounds();
	m_pauseResumeButton->SetAction("resume");
	m_pauseWidget->AddChild(static_cast<UIWidget*>(m_pauseResumeButton));

	// Create Label Widget that is a Child of Parent Widget;
	m_pauseResumeLabel = new UILabel(m_pauseWidget);
	m_pauseResumeLabel->SetVirtualPosition(Vec4(0.5f, 0.70f, 0.0f, 0.0f));
	m_pauseResumeLabel->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_pauseResumeLabel->UpdateBounds();
	m_pauseResumeLabel->SetLabel("Resume");
	m_pauseResumeLabel->SetFont(m_squirrelFont);
	m_pauseWidget->AddChild(static_cast<UIWidget*>(m_pauseResumeLabel));

	// Create Button Widget that is a Child of Parent Widget;
	m_playMainMenuButton = new UIButton(m_pauseWidget);
	m_playMainMenuButton->SetVirtualPosition(Vec4(0.5f, 0.55f, 0.0f, 0.0f));
	m_playMainMenuButton->SetVirtualSize(Vec4(0.2f, 0.1f, 30.0f, 0.0f));
	m_playMainMenuButton->UpdateBounds();
	m_playMainMenuButton->SetAction("mainmenu");
	m_pauseWidget->AddChild(static_cast<UIWidget*>(m_playMainMenuButton));

	// Create Label Widget that is a Child of Parent Widget;
	m_playMainMenuLabel = new UILabel(m_pauseWidget);
	m_playMainMenuLabel->SetVirtualPosition(Vec4(0.5f, 0.55f, 0.0f, 0.0f));
	m_playMainMenuLabel->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_playMainMenuLabel->UpdateBounds();
	m_playMainMenuLabel->SetLabel("Quit To Main Menu");
	m_playMainMenuLabel->SetFont(m_squirrelFont);
	m_pauseWidget->AddChild(static_cast<UIWidget*>(m_playMainMenuLabel));

	// Create Button Widget that is a Child of Parent Widget;
	m_pauseQuitButton = new UIButton(m_pauseWidget);
	m_pauseQuitButton->SetVirtualPosition(Vec4(0.5f, 0.40f, 0.0f, 0.0f));
	m_pauseQuitButton->SetVirtualSize(Vec4(0.2f, 0.1f, 35.0f, 0.0f));
	m_pauseQuitButton->UpdateBounds();
	m_pauseQuitButton->SetAction("quit");
	m_pauseWidget->AddChild(static_cast<UIWidget*>(m_pauseQuitButton));

	// Create Label Widget that is a Child of Parent Widget;
	m_pauseQuitLabel = new UILabel(m_pauseWidget);
	m_pauseQuitLabel->SetVirtualPosition(Vec4(0.5f, 0.40f, 0.0f, 0.0f));
	m_pauseQuitLabel->SetVirtualSize(Vec4(0.1f, 0.1f, 0.0f, 0.0f));
	m_pauseQuitLabel->UpdateBounds();
	m_pauseQuitLabel->SetLabel("Quit To Desktop");
	m_pauseQuitLabel->SetFont(m_squirrelFont);
	m_pauseWidget->AddChild(static_cast<UIWidget*>(m_pauseQuitLabel));

	// Create Button Widget that is a Child of Parent Widget;
	m_pauseQuitAndSaveButton = new UIButton(m_pauseWidget);
	m_pauseQuitAndSaveButton->SetVirtualPosition(Vec4(0.5f, 0.25f, 0.0f, 0.0f));
	m_pauseQuitAndSaveButton->SetVirtualSize(Vec4(0.2f, 0.1f, 35.0f, 0.0f));
	m_pauseQuitAndSaveButton->UpdateBounds();
	m_pauseQuitAndSaveButton->SetAction("quitandsavereplay");
	m_pauseWidget->AddChild(static_cast<UIWidget*>(m_pauseQuitAndSaveButton));

	// Create Label Widget that is a Child of Parent Widget;
	m_pauseQuitAndSaveLabel = new UILabel(m_pauseWidget);
	m_pauseQuitAndSaveLabel->SetVirtualPosition(Vec4(0.5f, 0.25f, 0.0f, 0.0f));
	m_pauseQuitAndSaveLabel->SetVirtualSize(Vec4(0.6f, 0.1f, 0.0f, 0.0f));
	m_pauseQuitAndSaveLabel->UpdateBounds();
	m_pauseQuitAndSaveLabel->SetLabel("Quit To Desktop & Save Replay");
	m_pauseQuitAndSaveLabel->SetFont(m_squirrelFont);
	m_pauseWidget->AddChild(static_cast<UIWidget*>(m_pauseQuitAndSaveLabel));
}

// -----------------------------------------------------------------------
// Load Assets
// -----------------------------------------------------------------------
void Game::StartLoadingAssets()
{
	m_loadStartTime = GetCurrentTimeSeconds();
	EnqueueWorkForTexturesAndGPUMeshes();

	uint coreCount = std::thread::hardware_concurrency(); 
	for (uint i = 0; i < coreCount; ++i)
	{
		//std::thread loadThread( CallImageAndMeshLoadThread ); 
		//m_threads.push_back( loadThread );
		m_threads.emplace_back( CallImageAndMeshLoadThread );
	}
}

// -----------------------------------------------------------------------
void Game::SetGameAssets()
{
	// Load Shader
	LoadGameShaders();

	// Load Font
	LoadGameFonts();

	// Load Materials
	LoadGameMaterials();

	m_titleScreen			= g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/SplashScreens/title.png");
	m_lobbyScreen			= g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/SplashScreens/lobby.png");
	m_playMapLoading		= g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/SplashScreens/playmaploading.png");
	m_editMapLoading		= g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/SplashScreens/editmaploading.png");

}

// -----------------------------------------------------------------------
void CallImageAndMeshLoadThread()
{ 
	g_theApp->m_theGame->ImageAndMeshLoadThread(); 
}

// -----------------------------------------------------------------------
void Game::EnqueueWorkForTexturesAndGPUMeshes()
{
	
	StartLoadingTexture( "Data/Images/SplashScreens/title.png" );
	StartLoadingTexture( "Data/Images/SplashScreens/lobby.png" );
	StartLoadingTexture( "Data/Images/SplashScreens/playmaploading.png" );
	StartLoadingTexture( "Data/Images/SplashScreens/editmaploading.png" );
	StartLoadingTexture( "Data/Fonts/SquirrelFixedFont.png" );
	StartLoadingTexture( "Data/Images/Foliage/foliage.color.png" );
	StartLoadingTexture( "Data/Sprites/goblin.attack.png" );
	StartLoadingTexture( "Data/Sprites/goblin.walkdeath.png" );
	StartLoadingTexture( "Data/Sprites/Laborer_spriteshee_2k.png" );
	StartLoadingTexture( "Data/Sprites/Warrior_spritesheet_attack.png" );
	StartLoadingTexture( "Data/Sprites/Warrior_spritesheet_moveDeath.png" );
	StartLoadingTexture( "Data/Images/Terrain/Dirt_DIFFU.png" );
	StartLoadingTexture( "Data/Images/Terrain/Dirt_Stone_DIFFU.png" );
	StartLoadingTexture( "Data/Images/Terrain/Grass_DIFFU.png" );
	StartLoadingTexture( "Data/Images/Terrain/Grass_DIFFU_WithUpTest.png" );
	StartLoadingTexture( "Data/Images/Terrain/GrassRubble_DIFFU.png" );
	StartLoadingTexture( "Data/Images/Terrain/Stone_DIFFU1.png" );
	StartLoadingTexture( "Data/Images/Terrain/Stone_DIFFU1_WithUpTest.png" );
	StartLoadingTexture( "Data/Images/goblin.diffuse.png" );
	StartLoadingTexture( "Data/Images/goblintower.color.png" );
	StartLoadingTexture( "Data/Images/towncenter.color.png" );
	StartLoadingTexture( "Data/Images/wood.png" );

	StartLoadingGPUMesh( "Data/Meshes/emptymineral.mesh" );
	StartLoadingGPUMesh( "Data/Meshes/fullmineral.mesh" );
	StartLoadingGPUMesh( "Data/Meshes/goblinhut.mesh" );
	StartLoadingGPUMesh( "Data/Meshes/goblintower.mesh" );
	StartLoadingGPUMesh( "Data/Meshes/hut.mesh" );
	StartLoadingGPUMesh( "Data/Meshes/middlemineral.mesh" );
	StartLoadingGPUMesh( "Data/Meshes/pinebark.mesh" );
	StartLoadingGPUMesh( "Data/Meshes/pineleaves.mesh" );
	StartLoadingGPUMesh( "Data/Meshes/pinestump.mesh" );
	StartLoadingGPUMesh( "Data/Meshes/pinewhole.mesh" );
	StartLoadingGPUMesh( "Data/Meshes/tower.mesh" );
	StartLoadingGPUMesh( "Data/Meshes/towncenter.mesh" );
	StartLoadingGPUMesh( "Data/Meshes/wood.mesh" );

	m_objectsToLoad = m_objectLoading;
}

// -----------------------------------------------------------------------
void Game::LoadGameShaders()
{

}

// -----------------------------------------------------------------------
void Game::LoadGameMaterials()
{
	m_tonemapMaterial = g_theRenderer->GetOrCreateMaterial("Data/Materials/effect.mat");
}

// -----------------------------------------------------------------------
void Game::CreateLoadingScreen()
{
	Vec2 minBounds;
	Vec2 maxBounds;
	minBounds.x = (float)m_clientMins.x;
	minBounds.y = (float)m_clientMins.y;
	maxBounds.x = (float)m_clientMaxs.x;
	maxBounds.y = (float)m_clientMaxs.y;

	// Load just the Title Mesh;
	m_titleMesh = new GPUMesh( g_theRenderer );
	CPUMesh mesh;
	mesh.SetLayout<Vertex_PCU>();
	CPUMeshAddQuad( &mesh, AABB2::MakeFromMinsMaxs(minBounds, maxBounds));  
	m_titleMesh->CreateFromCPUMesh( &mesh );

	m_loadingScreen = g_theRenderer->CreateOrGetTextureViewFromFile("Data/Images/SplashScreens/loading.png");
}

// -----------------------------------------------------------------------
void Game::CreateCameras()
{
	// Make an Ortho UI Camera;
	m_uiCamera = new Camera();
	m_uiCamera->SetOrthographicProjection(m_clientMins, m_clientMaxs);
	//m_uiCamera->SetOrthographicProjection(Vec2(0.0f, 0.0f), Vec2(1920.0f, 1080.0f));

	// Create Game Camera;
	m_gameMainCamera = new OrbitCamera();
	m_gameMainCamera->SetPerspectiveProjection( m_fieldOfView, ASPECT );
}

void Game::UpdateGameCamera( float deltaSeconds )
{
	UpdateFocalPointPosition( deltaSeconds );
	UpdateGameCameraPan( deltaSeconds );
	UpdateGameCameraZoom();
	UpdateGameCameraPosition();	
}

void Game::LoadGameFonts()
{
	m_squirrelFont = g_theRenderer->CreateOrGetBitmapFont("SquirrelFixedFont");
}

void Game::UpdateFocalPointPosition( float deltaSeconds )
{
	float& angle = m_gameMainCamera->m_defaultAngle;

	float x1 = 0.0f;
	float y1 = 1.0f;
	float finalX1 = (x1 * CosDegrees(angle)) - (y1 * SinDegrees(angle));
	float finalY1 = (y1 * CosDegrees(angle)) + (x1 * SinDegrees(angle));

	float x2 = 1.0f;
	float y2 = 0.0f;
	float finalX2 = (x2 * CosDegrees(angle)) - (y2 * SinDegrees(angle));
	float finalY2 = (y2 * CosDegrees(angle)) + (x2 * SinDegrees(angle));

	Vec3 focalPointRight = Vec3(finalX1, finalY1, 0.0f);
	Vec3 focalPointForward = Vec3(finalX2, finalY2, 0.0f) * -1.0f;

	focalPointForward.Normalize();
	focalPointRight.Normalize();

	m_focalPoint += focalPointForward * g_thePlayerController->GetFrameScroll().x * deltaSeconds;
	m_focalPoint += focalPointRight * g_thePlayerController->GetFrameScroll().y * deltaSeconds;

	// Stay within our map bounds;
	AABB2 mapBounds = m_map->GetXYBounds();
	m_focalPoint.x = m_focalPoint.x < mapBounds.mins.x ? mapBounds.mins.x : m_focalPoint.x;
	m_focalPoint.x = m_focalPoint.x > mapBounds.maxs.x ? mapBounds.maxs.x : m_focalPoint.x;
	m_focalPoint.y = m_focalPoint.y < mapBounds.mins.y ? mapBounds.mins.y : m_focalPoint.y;
	m_focalPoint.y = m_focalPoint.y > mapBounds.maxs.y ? mapBounds.maxs.y : m_focalPoint.y;

	m_gameMainCamera->SetFocalPoint( m_focalPoint );
}

void Game::UpdateGameCameraPan( float deltaSeconds )
{
	float panAmount = g_thePlayerController->GetFramePan();
	m_gameMainCamera->UpdateCameraPan( panAmount, deltaSeconds );
}

void Game::UpdateGameCameraZoom()
{
	float zoomDistance = g_thePlayerController->GetFrameZoomDistance();
	m_gameMainCamera->SetDistance( zoomDistance );
}

void Game::UpdateGameCameraPosition()
{
	m_gameMainCamera->Update();	
	m_cameraPosition = m_gameMainCamera->m_cameraModel.GetT();
}

OrbitCamera* Game::GetGameCamera()
{
	return m_gameMainCamera;
}

void Game::SetGameState( GameState gameState )
{
	m_gameState = gameState;

	if(g_thePlayerController)
	{
		g_thePlayerController->SetGameState(gameState);
	}	
}

void Game::CheckAndChangeGameState()
{
	m_gameState = g_thePlayerController->GetGameState();
	
}

GameState Game::GetGameState()
{
	return m_gameState;
}

void Game::DrawLobbyPlayers()
{
	DrawLobbyPlayersBoxes();
	DrawLobbyPlayersNames();	
}

void Game::DrawLobbyPlayersBoxes()
{
	std::vector<Vertex_PCU> backGroundQuad;
	AABB2 blackBox1 = AABB2::MakeFromMinsMaxs( Vec2( 50.0f, 600.0f ), Vec2( 700.0f, 650.0f ) );	
	AABB2 blackBox2 = AABB2::MakeFromMinsMaxs( Vec2( 50.0f, 500.0f ), Vec2( 700.0f, 550.0f ) );	
	AddVertsForAABB2D(backGroundQuad, blackBox1, Rgba(0.0f, 0.0f, 0.0f, 0.5f));
	AddVertsForAABB2D(backGroundQuad, blackBox2, Rgba(0.0f, 0.0f, 0.0f, 0.5f));
	g_theRenderer->BindTextureViewWithSampler(0u, nullptr);
	g_theRenderer->DrawVertexArray( backGroundQuad );
}

void Game::DrawLobbyPlayersNames()
{
	unsigned short numberOfSystems;
	g_theRakNetInterface->m_peer->GetConnectionList( 0, &numberOfSystems );

	if(numberOfSystems == 0)
	{
		std::vector<Vertex_PCU> textVerts;
		Vec2 textStartPosition1 = Vec2( 60.0f, 610.0f );
		m_squirrelFont->AddVertsForText2D(textVerts, textStartPosition1, 20.0f, "ME: SINGLE", Rgba::WHITE);
		g_theRenderer->BindTextureView( 0u, m_squirrelFont->GetTextureView() );
		g_theRenderer->DrawVertexArray((int)textVerts.size(), &textVerts[0]);

		std::vector<Vertex_PCU> textVerts2;
		Vec2 textStartPosition2 = Vec2( 60.0f, 510.0f );
		m_squirrelFont->AddVertsForText2D(textVerts2, textStartPosition2, 20.0f, "THEM: NONE", Rgba::WHITE);
		g_theRenderer->BindTextureView( 0u, m_squirrelFont->GetTextureView() );
		g_theRenderer->DrawVertexArray((int)textVerts2.size(), &textVerts2[0]);
	}
	else
	{
		if(g_theRakNetInterface->m_isServer)
		{
			// Me. I am Server;
			RakNet::RakNetGUID guid = g_theRakNetInterface->m_peer->GetMyGUID();
			std::string name = Stringf("ME: %s", guid.ToString());
			
			std::vector<Vertex_PCU> textVerts;
			Vec2 textStartPosition1 = Vec2( 60.0f, 610.0f );
			m_squirrelFont->AddVertsForText2D(textVerts, textStartPosition1, 20.0f, name, Rgba::WHITE);
			g_theRenderer->BindTextureView( 0u, m_squirrelFont->GetTextureView() );
			g_theRenderer->DrawVertexArray((int)textVerts.size(), &textVerts[0]);

			// Them. They are a Client;
			RakNet::SystemAddress systemAddresses;
			unsigned short numberOfSystems2 = 1;
			g_theRakNetInterface->m_peer->GetConnectionList(&systemAddresses, &numberOfSystems2);
			RakNet::RakNetGUID guid2 = g_theRakNetInterface->m_peer->GetGuidFromSystemAddress(systemAddresses);
			std::string name2 = Stringf("THEM: %s", guid2.ToString());

			std::vector<Vertex_PCU> textVerts2;
			Vec2 textStartPosition2 = Vec2( 60.0f, 510.0f );
			m_squirrelFont->AddVertsForText2D(textVerts2, textStartPosition2, 20.0f, name2, Rgba::WHITE);
			g_theRenderer->BindTextureView( 0u, m_squirrelFont->GetTextureView() );
			g_theRenderer->DrawVertexArray((int)textVerts2.size(), &textVerts2[0]);
		}
		else
		{
			// Them. They are Server;
			RakNet::SystemAddress systemAddresses;
			unsigned short numberOfSystems1 = 1;
			g_theRakNetInterface->m_peer->GetConnectionList(&systemAddresses, &numberOfSystems1);
			RakNet::RakNetGUID guid2 = g_theRakNetInterface->m_peer->GetGuidFromSystemAddress(systemAddresses);
			std::string name2 = Stringf("THEM: %s", guid2.ToString());

			std::vector<Vertex_PCU> textVerts2;
			Vec2 textStartPosition2 = Vec2( 60.0f, 610.0f );
			m_squirrelFont->AddVertsForText2D(textVerts2, textStartPosition2, 20.0f, name2, Rgba::WHITE);
			g_theRenderer->BindTextureView( 0u, m_squirrelFont->GetTextureView() );
			g_theRenderer->DrawVertexArray((int)textVerts2.size(), &textVerts2[0]);
			
			// Me. I am a Client;
			RakNet::RakNetGUID guid = g_theRakNetInterface->m_peer->GetMyGUID();
			std::string name = Stringf("ME: %s", guid.ToString());

			std::vector<Vertex_PCU> textVerts;
			Vec2 textStartPosition1 = Vec2( 60.0f, 510.0f );
			m_squirrelFont->AddVertsForText2D(textVerts, textStartPosition1, 20.0f, name, Rgba::WHITE);
			g_theRenderer->BindTextureView( 0u, m_squirrelFont->GetTextureView() );
			g_theRenderer->DrawVertexArray((int)textVerts.size(), &textVerts[0]);
		}
	}
}

UIWidget* Game::GetUIWidgetOfCurrentState()
{
	switch(m_gameState)
	{
		case GAMESTATE_TITLE:
		{
			return m_titleWidget;

			break;
		}
		case GAMESTATE_LOBBY:
		{
			return m_lobbyWidget;

			break;
		}
		case GAMESTATE_PLAY:
		{
			if(m_isPaused)
			{
				return m_pauseWidget;
			}
			
			return m_playWidget;

			break;
		}
		case GAMESTATE_EDIT:
		{
			if(m_isPaused)
			{
				return m_pauseWidget;
			}

			return m_editWidget;

			break;
		}
	}

	return nullptr;
}

void Game::DeleteUIWidgets()
{
	delete m_titleWidget;
	m_titleWidget = nullptr;

	delete m_titlePlayButton;
	m_titlePlayButton = nullptr;

	delete m_titlePlayLabel;
	m_titlePlayLabel = nullptr;

	delete m_editPlayButton;
	m_editPlayButton = nullptr;

	delete m_editPlayLabel;
	m_editPlayLabel = nullptr;

	delete m_quitPlayButton;
	m_quitPlayButton = nullptr;

	delete m_quitPlayLabel;
	m_quitPlayLabel = nullptr;

	delete m_playWidget;
	m_playWidget = nullptr;

	delete m_editWidget;
	m_editWidget = nullptr;	

	delete m_editTexture1Button;
	m_editTexture1Button = nullptr;

	delete m_editTexture1Label;
	m_editTexture1Label = nullptr;

	delete m_editTexture2Button;
	m_editTexture2Button = nullptr;

	delete m_editTexture2Label;
	m_editTexture2Label = nullptr;

	delete m_editTexture3Button;
	m_editTexture3Button = nullptr;

	delete m_editTexture3Label;
	m_editTexture3Label = nullptr;

	delete m_editTexture4Button;
	m_editTexture4Button = nullptr;

	delete m_editTexture4Label;
	m_editTexture4Label = nullptr;

	delete m_editTexture5Button;
	m_editTexture5Button = nullptr;

	delete m_editTexture5Label;
	m_editTexture5Label = nullptr;

	delete m_editRadioGroup;
	m_editRadioGroup = nullptr;

	delete m_pauseWidget;
	m_pauseWidget = nullptr;

	delete m_pauseResumeButton;
	m_pauseResumeButton = nullptr;

	delete m_pauseResumeLabel;
	m_pauseResumeLabel = nullptr;

	delete m_pauseQuitButton;
	m_pauseQuitButton = nullptr;

	delete m_pauseQuitLabel;
	m_pauseQuitLabel = nullptr;

	delete m_playMainMenuButton;
	m_playMainMenuButton = nullptr;

	delete m_playMainMenuLabel;
	m_playMainMenuLabel = nullptr;

	delete m_pauseQuitAndSaveButton;
	m_pauseQuitAndSaveButton = nullptr;

	delete m_pauseQuitAndSaveLabel;
	m_pauseQuitAndSaveLabel = nullptr;

	delete m_lobbyWidget;
	m_lobbyWidget = nullptr;

	delete m_lobbyPlayButton;
	m_lobbyPlayButton = nullptr;

	delete m_lobbyPlayLabel;
	m_lobbyPlayLabel = nullptr;

	delete m_lobbyBackButton;
	m_lobbyBackButton = nullptr;

	delete m_lobbyBackLabel;
	m_lobbyBackLabel = nullptr;
}

// -----------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------
void Game::EnQueueCommand( RTSCommand* command )
{
	m_myRTSCommands.push_back(command);

	if(m_isConnected)
	{
		g_theRakNetInterface->SendPlayMapRTSCommand(g_theRakNetInterface->m_themSystemAddress, command);
	}
}

// -----------------------------------------------------------------------
void Game::ProcessCommands()
{
	if(m_readFromReplay)
	{
		bool doneProcessingCommandsForThisFrame = false;
		while(!doneProcessingCommandsForThisFrame && (int)g_theReplayController->m_replayInfo.size() > 0)
		{
			ReplayInfo replayInfo = g_theReplayController->m_replayInfo.back();
			int thisCommandFrame = (int)g_theApp->m_commandFrame.GetCommandFrame();
			if(replayInfo.commandFrame == thisCommandFrame)
			{
				m_myRTSCommands.push_back(replayInfo.rtsCommand);
				g_theReplayController->m_replayInfo.pop_back();
			}
			else
			{
				doneProcessingCommandsForThisFrame = true;
			}
		}
	}
	
	for(RTSCommand* rtsCommand: m_myRTSCommands)
	{
		rtsCommand->Execute();
		
		//delete rtsCommand;
		//rtsCommand = nullptr;
	}

	ClearCommands();
}

// -----------------------------------------------------------------------
void Game::ProcessNetworkCommands()
{
	m_commandFrameToProcess = g_theApp->m_commandFrame.GetCommandFrame() - m_commandFrameDelay;
	m_commandFrameToProcess = m_commandFrameToProcess < 0 ? 0 : m_commandFrameToProcess;

	for(RTSCommand* rtsCommand: m_myRTSCommands)
	{
		if(rtsCommand)
		{
			if(rtsCommand->m_commandFrameIssued == m_commandFrameToProcess)
			{
				rtsCommand->Execute();
				delete rtsCommand;
				rtsCommand = nullptr;
			}
		}
	}

	for(RTSCommand* rtsCommand2: m_themRTSCommands)
	{
		if(rtsCommand2)
		{
			if(rtsCommand2->m_commandFrameIssued == m_commandFrameToProcess)
			{
				rtsCommand2->Execute();
				delete rtsCommand2;
				rtsCommand2 = nullptr;
			}
		}
	}

	//m_myRTSCommands.clear();
	//m_themRTSCommands.clear();
}

// -----------------------------------------------------------------------
void Game::ClearCommands()
{
	m_myRTSCommands.clear();
}

// -----------------------------------------------------------------------
// Team
// -----------------------------------------------------------------------
void Game::SetCurrentTeam( int teamID )
{
	m_currentTeam = teamID;
}

// -----------------------------------------------------------------------
int Game::GetCurrentTeam()
{
	return m_currentTeam;
}

// -----------------------------------------------------------------------
void Game::StopAllMusic()
{
	m_mainMenuMusicPlaying = false;
	m_playMusicPlaying = false;

	g_theAudioSystem->StopSound(m_playMusicPlaybackID);
	g_theAudioSystem->StopSound(m_mainMenuMusicPlaybackID);
}

// -----------------------------------------------------------------------
// Networking
// -----------------------------------------------------------------------
void Game::OnIncomingPacket( RakNet::Packet* packet )
{
	
	switch(packet->data[0])
	{
		case ID_GAME_MESSAGE_1:
		{
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(rs);
			std::string message = rs.C_String();
			g_theLobbyConsole->AddLineToChatText(message);
			break;
		}

		case ID_GAME_PLAYMAPLOADCOMPLETE:
		{
			bool loadComplete;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(loadComplete);
			m_connectedPlayMapLoadDone = loadComplete;
			break;
		}

		case ID_GAME_PLAYMAPENDOFFRAMECOMPLETE:
		{
			unsigned int frameComplete;
			unsigned int hashedFrame;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(frameComplete);
			bsIn.Read(hashedFrame);
			m_themCommandFrameComplete = frameComplete;
			m_themHashedFrame = hashedFrame;

			break;
		}

		case ID_GAME_PLAYMAPRTSCOMMAND:
		{
			RakNet::RakString rs;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
			bsIn.Read(rs);
			ReadMessageIntoRTSCommand(rs.C_String());

			


			break;
		}
	}
}

// -----------------------------------------------------------------------
void Game::SetupConnectionInformation()
{
	if(m_setupConnectionInfoComplete)
	{
		return;
	}
	
	unsigned short numberOfSystems;
	g_theRakNetInterface->m_peer->GetConnectionList( 0, &numberOfSystems );

	if(numberOfSystems > 0)
	{
		RakNet::SystemAddress mySystemAddress;
		RakNet::SystemAddress themSystemAddress;
		RakNet::RakNetGUID myGuid;
		RakNet::RakNetGUID themGuid;
		unsigned short systemNumber = 0;
		unsigned short systemNumber1 = 1;
		g_theRakNetInterface->m_peer->GetConnectionList(&themSystemAddress, &systemNumber);
		g_theRakNetInterface->m_peer->GetConnectionList(&mySystemAddress, &systemNumber1);
		myGuid = g_theRakNetInterface->m_peer->GetGuidFromSystemAddress(mySystemAddress);
		themGuid = g_theRakNetInterface->m_peer->GetGuidFromSystemAddress(themSystemAddress);

		g_theRakNetInterface->m_myGuid = myGuid;
		g_theRakNetInterface->m_themGuid = themGuid;
		g_theRakNetInterface->m_mySystemAddress = mySystemAddress;
		g_theRakNetInterface->m_themSystemAddress = themSystemAddress;

		m_setupConnectionInfoComplete = true;
		m_isConnected = true;

		if(g_theRakNetInterface->m_isServer)
		{
			SetCurrentTeam(0);
		}
		else
		{
			SetCurrentTeam(1);
		}
	}
}

void Game::ReadMessageIntoRTSCommand( const char* message )
{
	tinyxml2::XMLDocument xmlLoadCommand;
	xmlLoadCommand.Parse(message);

	XmlElement* commandElement = xmlLoadCommand.RootElement();
	CommandType commandType = (CommandType)ParseXmlAttribute(*commandElement, "CommandType", 999);

	switch(commandType)
	{
		case CREATEPEON:
		{
			CreateCommand* createCommand = new CreateCommand();
			createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			createCommand->m_theMap = GetGameMap();
			createCommand->m_theGame = this;
			createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
			createCommand->m_unit.m_handle = (uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0);
			createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			m_themRTSCommands.push_back(createCommand);

			break;
		}

		case CREATEWARRIOR:
		{
			CreateCommand* createCommand = new CreateCommand();
			createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			createCommand->m_theMap = GetGameMap();
			createCommand->m_theGame = this;
			createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
			createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			m_themRTSCommands.push_back(createCommand);

			break;
		}

		case MOVE:
		{
			MoveCommand* moveCommand = new MoveCommand();
			moveCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			moveCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			moveCommand->m_theGame = this;
			moveCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			moveCommand->m_theMap = GetGameMap();
			moveCommand->m_movePosition = ParseXmlAttribute(*commandElement, "MovePosition", Vec3(0.0f, 0.0f, 0.0f));
			m_themRTSCommands.push_back(moveCommand);

			break;
		}

		case ATTACK:
		{
			AttackCommand* attackCommand = new AttackCommand();
			attackCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			attackCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			attackCommand->m_entityToAttack = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "EntityToAttack", 0));
			attackCommand->m_theGame = this;
			attackCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			attackCommand->m_theMap = GetGameMap();
			m_themRTSCommands.push_back(attackCommand);

			break;
		}

		case DIE:
		{
			DieCommand* dieCommand = new DieCommand();
			dieCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			dieCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			dieCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			dieCommand->m_theMap = GetGameMap();
			m_themRTSCommands.push_back(dieCommand);

			break;
		}

		case FOLLOW:
		{
			FollowCommand* followCommand = new FollowCommand();
			followCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			followCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			followCommand->m_entityToFollow = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "EntityToFollow", 0));
			followCommand->m_theGame = this;
			followCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			followCommand->m_theMap = GetGameMap();
			m_themRTSCommands.push_back(followCommand);

			break;
		}

		case RIGHTCLICK:
		{
			RightClickCommand* rightClickCommand = new RightClickCommand();
			rightClickCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			rightClickCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			rightClickCommand->m_theGame = this;
			rightClickCommand->m_theMap = GetGameMap();
			rightClickCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));				
			rightClickCommand->m_rightClickPosition = ParseXmlAttribute(*commandElement, "RightClickPosition", Vec3(0.0f, 0.0f, 0.0f));
			rightClickCommand->m_rightClickEntity = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "RighClickedEntity", 0));
			m_themRTSCommands.push_back(rightClickCommand);

			break;
		}

		case CREATEPINETREE:
		{
			CreateCommand* createCommand = new CreateCommand();
			createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			createCommand->m_theMap = GetGameMap();
			createCommand->m_theGame = this;
			createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
			createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			m_themRTSCommands.push_back(createCommand);

			break;
		}

		case CREATETOWNHALL:
		{
			CreateCommand* createCommand = new CreateCommand();
			createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			createCommand->m_theMap = GetGameMap();
			createCommand->m_theGame = this;
			createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
			createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			m_themRTSCommands.push_back(createCommand);

			break;
		}

		case GATHER:
		{
			GatherCommand* gatherCommand = new GatherCommand();
			gatherCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			gatherCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			gatherCommand->m_theGame = this;
			gatherCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			gatherCommand->m_theMap = GetGameMap();
			gatherCommand->m_entityToGather = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "EntityToGather", 0));
			m_themRTSCommands.push_back(gatherCommand);

			break;
		}

		case CREATEPINEBARK:
		{
			CreateCommand* createCommand = new CreateCommand();
			createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			createCommand->m_theMap = GetGameMap();
			createCommand->m_theGame = this;
			createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
			createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			m_themRTSCommands.push_back(createCommand);

			break;
		}

		case CREATEPINESTUMP:
		{
			CreateCommand* createCommand = new CreateCommand();
			createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			createCommand->m_theMap = GetGameMap();
			createCommand->m_theGame = this;
			createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
			createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			m_themRTSCommands.push_back(createCommand);

			break;
		}

		case REPAIR:
		{
			RepairCommand* repairCommand = new RepairCommand();
			repairCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			repairCommand->m_theGame = this;
			repairCommand->m_theMap = GetGameMap();
			repairCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			repairCommand->m_entityToRepair = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "EntityToRepair", 0));
			repairCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			m_themRTSCommands.push_back(repairCommand);

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
			trainCommand->m_theGame = this;
			trainCommand->m_theMap = GetGameMap();
			trainCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			m_themRTSCommands.push_back(trainCommand);

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
			trainCommand->m_theGame = this;
			trainCommand->m_theMap = GetGameMap();
			trainCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			m_themRTSCommands.push_back(trainCommand);

			break;
		}

		case BUILDTOWNHALL:
		{
			BuildCommand* buildCommand = new BuildCommand();
			buildCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			buildCommand->m_theGame = this;
			buildCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			buildCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			buildCommand->m_theMap = GetGameMap();
			buildCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			buildCommand->m_buildPosition = ParseXmlAttribute(*commandElement, "BuildPosition", Vec3(0.0f, 0.0f, 0.0f));
			m_themRTSCommands.push_back(buildCommand);

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
			trainCommand->m_theGame = this;
			trainCommand->m_theMap = GetGameMap();
			trainCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			m_themRTSCommands.push_back(trainCommand);

			break;
		}

		case CREATEGOBLIN:
		{
			CreateCommand* createCommand = new CreateCommand();
			createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			createCommand->m_theMap = GetGameMap();
			createCommand->m_theGame = this;
			createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
			createCommand->m_unit.m_handle = (uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0);
			createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			m_themRTSCommands.push_back(createCommand);

			break;
		}

		case BUILDHUT:
		{
			BuildCommand* buildCommand = new BuildCommand();
			buildCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			buildCommand->m_theGame = this;
			buildCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			buildCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			buildCommand->m_theMap = GetGameMap();
			buildCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			buildCommand->m_buildPosition = ParseXmlAttribute(*commandElement, "BuildPosition", Vec3(0.0f, 0.0f, 0.0f));
			m_themRTSCommands.push_back(buildCommand);

			break;
		}

		case BUILDTOWER:
		{
			BuildCommand* buildCommand = new BuildCommand();
			buildCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			buildCommand->m_theGame = this;
			buildCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			buildCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			buildCommand->m_theMap = GetGameMap();
			buildCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			buildCommand->m_buildPosition = ParseXmlAttribute(*commandElement, "BuildPosition", Vec3(0.0f, 0.0f, 0.0f));
			m_themRTSCommands.push_back(buildCommand);

			break;
		}

		case BUILDGOBLINHUT:
		{
			BuildCommand* buildCommand = new BuildCommand();
			buildCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			buildCommand->m_theGame = this;
			buildCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			buildCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			buildCommand->m_theMap = GetGameMap();
			buildCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			buildCommand->m_buildPosition = ParseXmlAttribute(*commandElement, "BuildPosition", Vec3(0.0f, 0.0f, 0.0f));
			m_themRTSCommands.push_back(buildCommand);

			break;
		}

		case BUILDGOBLINTOWER:
		{
			BuildCommand* buildCommand = new BuildCommand();
			buildCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			buildCommand->m_theGame = this;
			buildCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			buildCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			buildCommand->m_theMap = GetGameMap();
			buildCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			buildCommand->m_buildPosition = ParseXmlAttribute(*commandElement, "BuildPosition", Vec3(0.0f, 0.0f, 0.0f));
			m_themRTSCommands.push_back(buildCommand);

			break;
		}

		case CREATEFULLMINERAL:
		{
			CreateCommand* createCommand = new CreateCommand();
			createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			createCommand->m_theMap = GetGameMap();
			createCommand->m_theGame = this;
			createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
			createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			m_themRTSCommands.push_back(createCommand);

			break;
		}

		case CREATEMIDDLEMINERAL:
		{
			CreateCommand* createCommand = new CreateCommand();
			createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			createCommand->m_theMap = GetGameMap();
			createCommand->m_theGame = this;
			createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
			createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			m_themRTSCommands.push_back(createCommand);

			break;
		}

		case CREATEEMPTYMINERAL:
		{
			CreateCommand* createCommand = new CreateCommand();
			createCommand->m_commandFrameIssued = ParseXmlAttribute(*commandElement, "CommandFrameIssued", 999);
			createCommand->m_commandType = ParseXmlAttribute(*commandElement, "CommandType", 999);
			createCommand->m_theMap = GetGameMap();
			createCommand->m_theGame = this;
			createCommand->m_createPosition = ParseXmlAttribute(*commandElement, "CreatePosition", Vec3(0.0f, 0.0f, 0.0f));
			createCommand->m_unit = GameHandle((uint32_t)ParseXmlAttribute(*commandElement, "GameHandle", 0));
			createCommand->m_team = ParseXmlAttribute(*commandElement, "Team", 0);
			m_themRTSCommands.push_back(createCommand);

			break;
		}
	}
}


ImageLoading::ImageLoading()
{
}

ImageLoading::ImageLoading( std::string name )
	: imageName(name)
{
}

CPUMeshLoading::CPUMeshLoading()
{
}

CPUMeshLoading::CPUMeshLoading( std::string name )
	: meshName(name)
{
}


