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
#include "Game/Gameplay/ConwaysGameOfLife.hpp"


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
	delete m_gameMainCamera;
	m_gameMainCamera = nullptr;

	delete m_uiCamera;
	m_uiCamera = nullptr;

	delete m_conwaysGameOfLife;
	m_conwaysGameOfLife = nullptr;

	delete m_masterUIWidget;
	m_masterUIWidget = nullptr;
}

// -----------------------------------------------------------------------
// Flow;
// -----------------------------------------------------------------------
void Game::Init()
{
	// Ask the WindowContext for the Client's dimensions;
	IntVec2 clientMins = g_theWindowContext->GetClientMins();
	IntVec2 clientMaxs = g_theWindowContext->GetClientMaxs();

	// Save off the Client's dimensions, keeping in mind that Windows has 0,0 is top left, we want it bottom right;
	m_clientMins = Vec2((float)clientMins.x, (float)clientMaxs.y);
	m_clientMaxs = Vec2((float)clientMaxs.x, (float)clientMins.y);

	// Save off the World dimensions;
	m_worldMins = m_clientMins;
	m_worldMaxs = m_clientMaxs;
	// m_worldMaxs = Vec2(ASPECTED_WORLD_HEIGHT, WORLD_HEIGHT);	// These are set in GameCommon;

	// Game Subscription Callbacks;
	g_theEventSystem->SubscriptionEventCallbackFunction("quit", QuitGame);

	// Conways Game of Life;
	m_conwaysGameOfLife = new ConwaysGameOfLife();
	m_conwaysGameOfLife->Init();
}

// -----------------------------------------------------------------------
void Game::Startup()
{
	// Cameras;
	CreateCameras();

	// Mouse Settings;
	g_theWindowContext->ShowMouse();							// During WindowContext's Init, we hid the mouse;
	g_theWindowContext->SetMouseMode(MOUSE_MODE_ABSOLUTE);		// Mouse position is where the mouse is;
	// g_theWindowContext->SetMouseMode(MOUSE_MODE_RELATIVE);	// Mouse position is locked to center;

	// Conways Game of Life;
	m_conwaysGameOfLife->Startup();

	// UI;
	m_masterUIWidget = new UIWidget(
		AABB2::MakeFromMinsMaxs(m_clientMins, m_clientMaxs),	// Bounding Box, entire screen;
		Vec4(1.0f, 1.0f, 0.0f, 0.0f),							// Virtual Size, entire bounding box;
		Vec4(0.5f, 0.5f, 0.0f, 0.0f)							// Virtual Position, center of bounding box;
	);
	


}

// -----------------------------------------------------------------------
void Game::BeginFrame()
{
	// Conways Game of Life;
	m_conwaysGameOfLife->BeginFrame();
}

// -----------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	UpdateGameCamera(deltaSeconds);

	// Conways Game of Life;
	m_conwaysGameOfLife->Update();
}

// -----------------------------------------------------------------------
void Game::Render()
{
	// Game Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_gameMainCamera->SetColorTargetView(m_colorTargetView);
	g_theRenderer->BeginCamera(m_gameMainCamera);
	g_theRenderer->ClearColorTargets(m_clearColor);

	// Conways Game of Life;
	m_conwaysGameOfLife->Render();

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
void Game::CreateCameras()
{
	// Create Game Camera that uses the World dimensions;
	m_gameMainCamera = new Camera();
	m_gameMainCamera->SetOrthographicProjection(m_worldMins, m_worldMaxs);

	// Make an Ortho UI Camera that uses the Client's dimensions;
	m_uiCamera = new Camera();
	m_uiCamera->SetOrthographicProjection(m_clientMins, m_clientMaxs);
}

// -----------------------------------------------------------------------
void Game::UpdateGameCamera( float deltaSeconds )
{
	UpdateFocalPointPosition( deltaSeconds );
	UpdateGameCameraPosition();	
}

// -----------------------------------------------------------------------
void Game::UpdateFocalPointPosition(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_focalPoint = m_worldMaxs / 2.0f;	// Focus on the center of the Camera;
}

// -----------------------------------------------------------------------
void Game::UpdateGameCameraPosition()
{
	AABB2 cameraBounds = AABB2::MakeFromMinsMaxs(m_gameMainCamera->m_minOrtho, m_gameMainCamera->m_maxOrtho);
	Vec2 originalOffset = cameraBounds.offset;
	cameraBounds.center = m_focalPoint;

	Vec2 newCameraBL = Vec2(cameraBounds.center - originalOffset);
	Vec2 newCameraTR = Vec2(cameraBounds.center + originalOffset);

	m_gameMainCamera->SetOrthographicProjection(newCameraBL, newCameraTR);
}
