// Defines and Includes ---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN	
#include <winsock2.h>

// Commons ----------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Framework/GameCommon.hpp"

// Engine Includes --------------------------------------------------------------------------------
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/VertexLit.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimationDefinition.hpp"
#include "Engine/Renderer/BitMapFont.hpp"
#include "Engine/Renderer/Prop.hpp"
#include "Engine/Renderer/Model.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/UI/UIWidget.hpp"

// Game Includes ----------------------------------------------------------------------------------
#include "Game/Framework/App.hpp"
#include "Game/Input/GameInput.hpp"
#include "Game/Gameplay/Map.hpp"

// Callbacks --------------------------------------------------------------------------------------
static bool QuitGame(EventArgs& args)
{
	UNUSED(args);

	g_theApp->HandleCloseApplication();

	return true;
}

// Constructor ------------------------------------------------------------------------------------
Game::Game() 
{
}

// Deconstructor ----------------------------------------------------------------------------------
Game::~Game()
{
	// Delete Pointers (template);
	DELETE_POINTER(m_gameMainCamera);
	DELETE_POINTER(m_uiCamera);
	DELETE_POINTER(m_map);
}

// -----------------------------------------------------------------------
// Flow;
// -----------------------------------------------------------------------
void Game::Init()
{
	// Ask the WindowContext for the Client's dimensions;
	IntVec2 clientMins = g_theWindowContext->GetClientMins();
	IntVec2 clientMaxs = g_theWindowContext->GetClientMaxs();

	// Save off the Client's dimensions, keeping in mind that Windows has 0,0 is top left, we want it bottom left;
	m_clientMins = Vec2((float)clientMins.x, (float)clientMaxs.y);
	m_clientMaxs = Vec2((float)clientMaxs.x, (float)clientMins.y);

	// Game Subscription Callbacks;
	g_theEventSystem->SubscriptionEventCallbackFunction("quit", QuitGame);

	m_map				= new Map();
	m_gameMainCamera	= new Camera();
	m_uiCamera			= new Camera();	

	m_gameMainCamera->SetOrthographicProjection(Vec2::ZERO, Vec2(Map::WIDTH, Map::HEIGHT));
	m_uiCamera->SetOrthographicProjection(Vec2::ZERO, Vec2(Map::WIDTH, Map::HEIGHT));


	/*
	m_masterUIWidget = new UIWidget(
		AABB2::MakeFromMinsMaxs(m_clientMins, m_clientMaxs),	// Bounding Box, entire screen;
		Vec4(1.0f, 1.0f, 0.0f, 0.0f),							// Virtual Size, entire bounding box;
		Vec4(0.5f, 0.5f, 0.0f, 0.0f)							// Virtual Position, center of bounding box;
	);
	*/
	
}

// -----------------------------------------------------------------------
void Game::Startup()
{
	// Mouse Settings;
	g_theWindowContext->ShowMouse();							// During WindowContext's Init, we hid the mouse;
	g_theWindowContext->SetMouseMode(MOUSE_MODE_ABSOLUTE);		// Mouse position is where the mouse is;
	// g_theWindowContext->SetMouseMode(MOUSE_MODE_RELATIVE);	// Mouse position is locked to center;

	m_clearColor = Rgba::BLACK;

	StartLoadingAssets();

	for(int i = 0; i < 8; ++i)
	{



	}


	m_map = new Map();
	m_map->Startup();
}

// -----------------------------------------------------------------------
void Game::BeginFrame()
{
	
}

// -----------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	// Async Loading;
	if(m_stillLoading)
	{
		if(!DoneLoading())
		{
			ContinueLoading();
		}
		else
		{
			EndLoadingThreads();
			m_stillLoading = false;
		}
	}

	m_map->Update(deltaSeconds);
}

// -----------------------------------------------------------------------
void Game::Render()
{
	// Game Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_gameMainCamera->SetColorTargetView(m_colorTargetView);
	g_theRenderer->BeginCamera(m_gameMainCamera);
	g_theRenderer->ClearColorTargets(m_clearColor);

	m_map->Render();

	g_theRenderer->EndCamera();

	// UI Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_uiCamera->SetColorTargetView(m_colorTargetView);
	g_theRenderer->BeginCamera(m_uiCamera);

	g_theRenderer->EndCamera();
}

// -----------------------------------------------------------------------
void Game::EndFrame()
{
	
}

// -----------------------------------------------------------------------
// Cameras;
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// Assets;
// -----------------------------------------------------------------------
void Game::StartLoadingAssets()
{
	EnqueueWorkForTexturesAndGPUMeshes();

	uint coreCount = std::thread::hardware_concurrency();
	for (uint i = 0; i < coreCount; ++i)
	{
		//std::thread loadThread( CallImageAndMeshLoadThread ); 
		//m_threads.push_back( loadThread );
		m_threads.emplace_back(CallImageAndMeshLoadThread);
	}
}

// -----------------------------------------------------------------------
void Game::EnqueueWorkForTexturesAndGPUMeshes()
{
	StartLoadingTexture("Data/Sprites/Jobs/Knight/Knight1M-S.png");

	m_stillLoading = true;
}

// -----------------------------------------------------------------------
void Game::ImageAndMeshLoadThread()
{
	bool doneLoading = false;

	ImageLoading imageLoading;
	CPUMeshLoading cpuLoading;

	while (!doneLoading)
	{
		bool gotImageWork = imageLoadingFromDiscQueue.Dequeue(&imageLoading);
		if (gotImageWork)
		{
			imageLoading.image = new Image(imageLoading.imageName.c_str());
			imageCreatingTextureQueue.Enqueue(imageLoading);
		}

		bool gotCPUWork = cpuLoadingFromDiscQueue.Dequeue(&cpuLoading);
		if (gotCPUWork)
		{
			cpuLoading.cpuMesh = new CPUMesh();
			cpuLoading.cpuMesh->SetLayout<Vertex_Lit>();
			CreateMeshFromFile(cpuLoading.meshName.c_str(), cpuLoading.cpuMesh);
			cpuCreatingGPUMeshQueue.Enqueue(cpuLoading);
		}

		if (!gotImageWork && !gotCPUWork)
		{
			doneLoading = true;
		}

		Sleep(0);
	}
}

// -----------------------------------------------------------------------
void Game::StartLoadingTexture(std::string nameOfTexture)
{
	ImageLoading loading = ImageLoading(nameOfTexture);

	m_objectLoading++;
	imageLoadingFromDiscQueue.Enqueue(loading);
}

// -----------------------------------------------------------------------
void Game::ContinueLoading()
{
	ImageLoading imageLoading;
	CPUMeshLoading cpuLoading;

	bool gotImageWork = imageCreatingTextureQueue.Dequeue(&imageLoading);
	if (gotImageWork)
	{
		g_theRenderer->CreateTextureViewFromImage(imageLoading.image, imageLoading.imageName);
		m_objectLoading--;
		delete imageLoading.image;
		imageLoading.image = nullptr;
	}

	bool gotGPUWork = cpuCreatingGPUMeshQueue.Dequeue(&cpuLoading);
	if (gotGPUWork)
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
void Game::EndLoadingThreads()
{
	for (std::thread& t : m_threads)
	{
		t.join();
	}

	m_threads.clear();
}

// -----------------------------------------------------------------------
// static;
// -----------------------------------------------------------------------
void CallImageAndMeshLoadThread()
{
	g_theApp->m_theGame->ImageAndMeshLoadThread();
}