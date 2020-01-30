// Defines and Includes ---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <winsock2.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

// Commons ----------------------------------------------------------------------------------------
#include "Game/Framework/GameCommon.hpp"

// Engine Includes --------------------------------------------------------------------------------
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/RandomNumberGenerator.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"

// Game Includes ----------------------------------------------------------------------------------
#include "Game/Framework/App.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Game/Input/GameInput.hpp"


// Global Singletons ------------------------------------------------------------------------------
App* g_theApp = nullptr;

// Constructor ------------------------------------------------------------------------------------
App::App()
{
	
}

// Deconstructor ----------------------------------------------------------------------------------
App::~App()
{

}

// Init -------------------------------------------------------------------------------------------
void App::Init()
{
	g_theWindowContext->HideMouse();

	// Create Systems;
	g_theInputSystem			= new InputSystem();
	g_theGameInput				= new GameInput();
	g_theDebugRenderer			= new DebugRender();
	g_theAudioSystem			= new AudioSystem();	
	g_theDevConsole				= new DevConsole();
	g_theRandomNumberGenerator	= new RandomNumberGenerator();
	g_theEventSystem			= new EventSystem();
	m_theGame					= new Game();

	// Init Systems;
	g_theRenderer->Init();
	m_theGame->Init();
}

// Startup ----------------------------------------------------------------------------------------
void App::Startup()
{	
	// Start Systems;
	g_theInputSystem->Startup();
	g_theGameInput->Startup();
	g_theRenderer->Startup();
	g_theDebugRenderer->Startup( g_theRenderer, 1080.0f );
	g_theAudioSystem->Startup();
	g_theDevConsole->Startup();
	g_theEventSystem->Startup();
	m_theGame->Startup();

	m_intialStartupDone = true;
}

// -----------------------------------------------------------------------
void App::RunFrame()
{
	BeginFrame();
	Update();
	Render();
	EndFrame();
}

// -----------------------------------------------------------------------
void App::Shutdown()
{
	// Delete Game;
	delete m_theGame;
	m_theGame = nullptr;

	// Shutdown System Pointers;
	g_theEventSystem->Shutdown();
	g_theDevConsole->Shutdown();
	g_theAudioSystem->Shutdown();
	g_theDebugRenderer->Shutdown();
	g_theRenderer->Shutdown();
	g_theGameInput->Shutdown();
	g_theInputSystem->Shutdown();

	// Delete System Pointers (template);
	DELETE_POINTER(g_theEventSystem);
	DELETE_POINTER(g_theDevConsole);
	DELETE_POINTER(g_theRandomNumberGenerator);
	DELETE_POINTER(g_theAudioSystem);
	DELETE_POINTER(g_theDebugRenderer);
	DELETE_POINTER(g_theRenderer);
	DELETE_POINTER(g_theGameInput);
	DELETE_POINTER(g_theInputSystem);
}

// -----------------------------------------------------------------------
void App::CalculateDeltaSeconds()
{
	m_timeNow = GetCurrentTimeSeconds();
	m_deltaSeconds = (float)(m_timeNow - m_timeLastFrameBegan);
	m_timeLastFrameBegan = m_timeNow;

	//m_deltaSeconds = Clamp(m_deltaSeconds, 0.0f, m_commandFrame.m_gameMaxFps);
}

// -----------------------------------------------------------------------
void App::BeginFrame()
{
	g_theWindowContext->BeginFrame();
	g_theAudioSystem->BeginFrame();
	g_theInputSystem->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theDebugRenderer->BeginFrame();
	g_theRenderer->BeginFrame();

	m_theGame->BeginFrame();
}

// -----------------------------------------------------------------------
void App::Update()
{
	if(m_isFirstFrame)
	{
		m_timeNow = GetCurrentTimeSeconds();
		m_timeLastFrameBegan = m_timeNow;
		return;
	}

	CalculateDeltaSeconds();
	
	m_theGame->Update(m_deltaSeconds);
	
	g_theDebugRenderer->Update( m_deltaSeconds );
	g_theDebugRenderer->Cleanup();

}

// -----------------------------------------------------------------------
void App::Render()
{
	

	if(m_isFirstFrame)
	{
		return;
	}

	// Render;
	m_theGame->Render();

	//g_theDebugRenderer->RenderToScreen();
	g_theDevConsole->Render(g_theRenderer->CreateOrGetBitmapFont("SquirrelFixedFont"));
}

// -----------------------------------------------------------------------
void App::EndFrame()
{
	m_theGame->EndFrame();
	g_theGameInput->EndFrame();
	g_theInputSystem->EndFrame();
	g_theRenderer->EndFrame();
	g_theDebugRenderer->EndFrame();
	g_theDevConsole->EndFrame();

	m_isFirstFrame = false;
}

// -----------------------------------------------------------------------
// Character Keys;
// -----------------------------------------------------------------------
bool App::HandleChar(unsigned char asKey_)
{
	UNUSED(asKey_);
	return false;
}

// -----------------------------------------------------------------------
// Virtual Keys;
// -----------------------------------------------------------------------
bool App::HandleKeyPressed(unsigned char asKey_)
{
	return g_theGameInput->HandleKeyPressed(asKey_);
}

// -----------------------------------------------------------------------
bool App::HandleKeyReleased(unsigned char asKey_)
{
	return g_theGameInput->HandleKeyReleased(asKey_);
}

