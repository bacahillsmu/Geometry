#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <math.h>
#include <cassert>
#include <crtdbg.h>

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Framework/App.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/UnitTests/UnitTests.hpp"

//App* g_theApp = nullptr;
//WindowContext* g_theWindowContext = nullptr;

// This is game handling input second, after engine decides maybe to ignore to it.
bool GameWindowsProcCallback( WindowContext* windowContext, uint msg, uintptr_t wParam, uintptr_t lParam )
{
	UNUSED(windowContext);
	UNUSED(lParam);

	switch( msg )
	{
		case WM_LBUTTONDOWN:
		{
			g_theApp->ClickLeftMouse();
			break;
		}
		case WM_LBUTTONUP:
		{
			g_theApp->ReleaseLeftMouse();
			break;
		}
		case WM_RBUTTONDOWN:
		{
			g_theApp->ClickRightMouse();
			break;
		}
		case WM_RBUTTONUP:
		{
			g_theApp->ReleaseRightMouse();
			break;
		}
		case WM_MBUTTONDOWN:
		{
			g_theApp->ClickMiddleMouse();
			break;
		}
		case WM_MBUTTONUP:
		{
			g_theApp->ReleaseMiddleMouse();
			break;
		}
		case WM_MOUSEWHEEL:
		{
			g_theApp->StoreWheelDirection((float)GET_WHEEL_DELTA_WPARAM(wParam));
			break;
		}
		case WM_CLOSE:		
		{
			g_theApp->HandleCloseApplication();
			break;
		}
		case WM_CHAR:
		{
			unsigned char asKey = (unsigned char) wParam;
			g_theApp->HandleChar( asKey );
			break;
		} 	
		case WM_KEYDOWN:
		{
			unsigned char asKey = (unsigned char) wParam;
			g_theApp->HandleKeyPressed( asKey );
			break;
		} 	
		case WM_KEYUP:
		{
			unsigned char asKey = (unsigned char) wParam;
			g_theApp->HandleKeyReleased( asKey );
			break;
		}
	}

	return false;
}	

void Init()
{
	//UnitTest::UnitTestsRunAllCategories();

	g_theApp = new App();
	g_theWindowContext = new WindowContext();

	// Create empty window
	g_theWindowContext->Create("SD-RTS", 2.0f, 0.9f, &GameWindowsProcCallback);

	// Create the Render Context for D3D11
	g_theRenderer = new RenderContext( g_theWindowContext );

	// Call the App's Startup
	//g_theApp->Startup();

	// Call the App's Init
	g_theApp->Init();
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

	while( !g_theApp->IsQuitting() )
	{
		g_theApp->RunFrame();
		Sleep(0);
	}

	Shutdown();
	return 0;
}



