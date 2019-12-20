
// Defines and Includes ---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <winsock2.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

#include "Game/Framework/GameCommon.hpp"
#include "Game/Framework/App.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/Gameplay/Input/PlayerController.hpp"
#include "Game/Gameplay/Input/ReplayController.hpp"
#include "Game/Gameplay/Input/AIController.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "ThirdParty/RakNet/RakNetInterface.hpp"
#include "Game/Gameplay/Lobby/LobbyConsole.hpp"
#include "Engine/UnitTests/UnitTests.hpp"
#include "Engine/Callstack/Callstack.hpp"
#include "Engine/Log/Log.hpp"
#include "Engine/Profile/Profile.hpp"
#include "Engine/Job/Jobs.hpp"

// Global Singletons ------------------------------------------------------------------------------
App* g_theApp = nullptr;
RandomNumberGenerator* g_theRandomNumberGenerator = nullptr;
InputSystem* g_theInputSystem = nullptr;
AudioSystem* g_theAudioSystem = nullptr;


// Constructor ------------------------------------------------------------------------------------
App::App()
{
	
}

// Deconstructor ----------------------------------------------------------------------------------
App::~App()
{

}

void App::Init()
{
	LogSeconds log("App Init: ", 0.0);
	g_theWindowContext->HideMouse();

	// Create Systems;
	g_theDebugRenderer			= new DebugRender();
	g_theInputSystem			= new InputSystem();
	g_theAudioSystem			= new AudioSystem();	
	g_theDevConsole				= new DevConsole();
	g_theRandomNumberGenerator	= new RandomNumberGenerator();
	g_theEventSystem			= new EventSystem();
	g_theRakNetInterface		= new RakNetInterface();
	m_theGame					= new Game();
	g_theLobbyConsole			= new LobbyConsole();
	g_theJobSystem				= new JobSystem();

	// Init Systems;
	g_theRenderer->Init();
	m_theGame->Init();
	LogSystemInit("Data/Log/logfile.txt");
	ProfilerSystemInit();
	g_theProfilerReport			= new ProfilerReport();

	g_thePlayerController		= new PlayerController( m_theGame );
	g_theAIController			= new AIController( m_theGame );
	g_theReplayController		= new ReplayController( m_theGame );
}

// ------------------------------------------------------------------------------------------------
void App::Startup()
{	
// 	if(m_isFirstFrame || m_intialStartupDone)
// 	{
// 		return;
// 	}

	// Start Systems;
	g_theRenderer->Startup();
	g_theDebugRenderer->Startup( g_theRenderer, 1080.0f );
	g_theInputSystem->Startup();
	g_theAudioSystem->Startup();
	g_theDevConsole->Startup();
	g_theEventSystem->Startup();
	g_theRakNetInterface->Startup();
	m_theGame->Startup();
	g_theLobbyConsole->Startup();
	g_theJobSystem->Startup();

	// Do extras for DevConsole;
	LoadDevConsoleFonts();	

	// Unit Tests;
	//UnitTest::UnitTestsRunAllCategories(5);
	//UnitTest::UnitTestsRunCategory("BasicBitchTests");

	m_intialStartupDone = true;

	//Callstack callstack = GetBenStack();
	//CallstackToString(callstack);
}

// -----------------------------------------------------------------------
void App::RunFrame()
{

	BeginFrame();
	Update( m_deltaSeconds );
	Render();
	EndFrame();
}

// -----------------------------------------------------------------------
void App::Shutdown()
{
	// Delete Game;
	delete m_theGame;
	m_theGame = nullptr;

	// Shutdown each System;
	g_theJobSystem->Shutdown();
	g_theRakNetInterface->Shutdown();
	g_theEventSystem->Shutdown();
	g_theDevConsole->Shutdown();
	g_theAudioSystem->Shutdown();
	g_theInputSystem->Shutdown();
	g_theDebugRenderer->Shutdown();
	g_theRenderer->Shutdown();

	LogSystemShutdown();
	ProfilerSystemDeinit();

	// Delete System Pointers;
	delete g_theJobSystem;
	g_theJobSystem = nullptr;

	delete g_theLobbyConsole;
	g_theLobbyConsole = nullptr;

	delete g_theRakNetInterface;
	g_theRakNetInterface = nullptr;

	delete g_theReplayController;
	g_theReplayController = nullptr;

	delete g_theAIController;
	g_theAIController = nullptr;

	delete g_thePlayerController;
	g_thePlayerController = nullptr;

	delete g_theEventSystem;
	g_theEventSystem = nullptr;

	delete g_theDevConsole;
	g_theDevConsole = nullptr;

	delete g_theAudioSystem;
	g_theAudioSystem = nullptr;

	delete g_theInputSystem;
	g_theInputSystem = nullptr;

	delete g_theDebugRenderer;
	g_theDebugRenderer = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;
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
bool App::HandleChar( unsigned char asKey )
{
	if(!g_theDevConsole->IsOpen())
	{
		// Press `
		if(asKey == 96)
		{
			g_theDevConsole->ToggleOpen();
			return 0;
		}
	}
	if(g_theDevConsole->IsOpen())
	{
		if(asKey != Key::BACKSPACE
		&& asKey != Key::ESC
		&& asKey != Key::PERIOD
		&& asKey != Key::ENTER)
		{
			std::string letter(1, asKey);
			g_theDevConsole->AddLetterToCurrentTypingText( Rgba::WHITE, letter );
			g_theDevConsole->UpdateCursorPosition( 1 );
			g_theDevConsole->ResetValues();
		}
	}
	else if(g_theLobbyConsole->IsOpen())
	{
		if(asKey != Key::BACKSPACE
		&& asKey != Key::ESC
		&& asKey != Key::PERIOD
		&& asKey != Key::ENTER)
		{
			std::string letter(1, asKey);
			g_theLobbyConsole->AddLetterToTypingText( letter );
			g_theLobbyConsole->UpdateCursorPosition( 1 );
			g_theLobbyConsole->ResetValues();
		}
	}

	return 0;
}

// -----------------------------------------------------------------------
bool App::HandleKeyPressed( unsigned char asKey )
{
	if( !g_theDevConsole->HandleKeyPressed( asKey ) )
	{
		if(!g_theLobbyConsole->HandleKeyPressed(asKey))
		{
			g_thePlayerController->HandleKeyPressed( asKey );
		}
	}	
	
	return 0;
}

// -----------------------------------------------------------------------
bool App::HandleKeyReleased( unsigned char asKey )
{
	if( !g_theDevConsole->IsOpen() )
	{
		g_thePlayerController->HandleKeyReleased( asKey );
	}
	
	return 0;
}

// -----------------------------------------------------------------------
void App::BeginFrame()
{
	ProfileBeginFrame("App::BeginFrame");

	g_theWindowContext->BeginFrame();
	g_theAudioSystem->BeginFrame();
	g_theInputSystem->BeginFrame();
	g_theDevConsole->BeginFrame();
	g_theDebugRenderer->BeginFrame();
	g_theRenderer->BeginFrame();

	m_theGame->BeginFrame();
}


// -----------------------------------------------------------------------
void App::Update( float deltaSeconds )
{
	ProfilerUpdate();

	if(m_isFirstFrame)
	{
		return;
	}

	// Figure out our DeltaSeconds from frame to frame;
	CalculateDeltaSeconds();
	
	// Add our DeltaSeconds to our "pool" of time;
	m_commandFrame.m_timeLeftOver += deltaSeconds;	
	
	// Keep pulling out a fixed DeltaSeconds from our "pool" of time;
	while(m_commandFrame.m_timeLeftOver >= m_commandFrame.m_fixedDeltaSeconds)
	{
		// Receive commands over the Network on a command-frame to command-frame basis
		g_theRakNetInterface->ProcessIncomingPackets();

		// Networked gameplay;
		if(m_theGame->m_isConnected && m_theGame->GetGameState() == GAMESTATE_PLAY)
		{
			unsigned int nextFrame = m_commandFrame.m_commandFrameNumber + 1;

			// Keeping track of the delay on our command-frames to know if we can process commands;
			int frameImExecuting = (int)nextFrame - (int)m_theGame->m_commandFrameDelay;
			int theMinFrame = min((int)m_theGame->m_myCommandFrameComplete, (int)m_theGame->m_themCommandFrameComplete);
			if(frameImExecuting < theMinFrame)
			{
				// Run a command-frame Update;
				m_theGame->Update( m_commandFrame.m_fixedDeltaSeconds );
				m_commandFrame.m_commandFrameNumber++;
			}

			// Take time used out of the "pool";
			m_commandFrame.m_timeLeftOver -= m_commandFrame.m_fixedDeltaSeconds;
		}
		// Single player gameplay;
		else
		{
			m_theGame->Update( m_commandFrame.m_fixedDeltaSeconds );
			m_commandFrame.m_commandFrameNumber++;
			m_commandFrame.m_timeLeftOver -= m_commandFrame.m_fixedDeltaSeconds;
		}
	}
	
	g_theDebugRenderer->Update( deltaSeconds );
	g_theDebugRenderer->Cleanup();

}


// -----------------------------------------------------------------------
void App::Render()
{
	
	// Render;
	m_theGame->Render();

	if(m_isFirstFrame)
	{
		return;
	}

	//g_theDebugRenderer->RenderToScreen();
	g_theDevConsole->Render( m_squirrelFont ); 
}

// -----------------------------------------------------------------------
void App::EndFrame()
{
	m_theGame->EndFrame();
	g_theRenderer->EndFrame();
	g_theDebugRenderer->EndFrame();
	g_theDevConsole->EndFrame();

	m_isFirstFrame = false;

	ProfileEndFrame();
}

// -----------------------------------------------------------------------
void App::LoadDevConsoleFonts()
{
	m_squirrelFont = g_theRenderer->CreateOrGetBitmapFont("SquirrelFixedFont");
}

// -----------------------------------------------------------------------
// Mouse Click
// -----------------------------------------------------------------------
void App::ClickLeftMouse()
{
	g_thePlayerController->ClickLeftMouse();
}

void App::ClickRightMouse()
{
	g_thePlayerController->ClickRightMouse();
}

void App::ClickMiddleMouse()
{
	g_thePlayerController->ClickMiddleMouse();
}

void App::ReleaseLeftMouse()
{
	g_thePlayerController->ReleaseLeftMouse();
}

void App::ReleaseRightMouse()
{
	g_thePlayerController->ReleaseRightMouse();
}

void App::ReleaseMiddleMouse()
{
	g_thePlayerController->ReleaseMiddleMouse();
}

// -----------------------------------------------------------------------
// Mouse Wheel
// -----------------------------------------------------------------------
void App::StoreWheelDirection( float wheelDelta )
{
	g_thePlayerController->StoreWheelMovement( wheelDelta );	
}

