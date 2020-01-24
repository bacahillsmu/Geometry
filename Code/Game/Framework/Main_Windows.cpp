#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Framework/App.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/UnitTests/UnitTests.hpp"

#include <math.h>
#include <cassert>
#include <crtdbg.h>

//-----------------------------------------------------------------------------------------------
// This is game handling input second, after engine decides maybe to ignore to it.
bool GameWindowsProcCallback( WindowContext* windowContext, uint msg, uintptr_t wParam, uintptr_t lParam )
{
	UNUSED(windowContext);
	UNUSED(lParam);

	bool keyHandled = false;

	switch( msg )
	{		
		case WM_CLOSE:		
		{
			g_theApp->HandleCloseApplication();

			keyHandled = true;
			break;
		}

		// Character Keys;
		case WM_CHAR:
		{
			unsigned char asKey = (unsigned char) wParam;
			keyHandled = g_theApp->HandleChar( asKey );
			break;
		}

		// Virtual Keys;
		case WM_KEYDOWN:
		{
			unsigned char asKey = (unsigned char) wParam;
			keyHandled = g_theApp->HandleKeyPressed( asKey );
			break;
		}

		case WM_KEYUP:
		{
			unsigned char asKey = (unsigned char) wParam;
			keyHandled = g_theApp->HandleKeyReleased( asKey );
			break;
		}
	}

	return keyHandled;
}	

//-----------------------------------------------------------------------------------------------
void Init()
{
	g_theApp = new App();
	g_theWindowContext = new WindowContext();

	g_theWindowContext->Create("Proto", ASPECT, 0.9f, &GameWindowsProcCallback);

	g_theRenderer = new RenderContext( g_theWindowContext );

	g_theApp->Init();
}

//-----------------------------------------------------------------------------------------------
void Startup()
{
	g_theApp->Startup();
}

//-----------------------------------------------------------------------------------------------
void Shutdown()
{
	g_theApp->Shutdown();

	delete g_theApp;
	g_theApp = nullptr;
}

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
	Init();
	Startup();

	while( !g_theApp->IsQuitting() )
	{
		g_theApp->RunFrame();
		Sleep(0);
	}

	Shutdown();
	return 0;
}



