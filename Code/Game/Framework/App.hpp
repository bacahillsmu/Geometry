#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/Framework/CommandFrame.hpp"


class Game;
class PlayerController;
class BitMapFont;
struct Camera;

class App
{
public:
	
	// Constructor/Deconstructor
	App();
	~App();
	
	// Game Flow
	void Init();
	void Startup();
	void RunFrame();
	void BeginFrame();
	void Update( float deltaSeconds );
	void Render();
	void EndFrame();
	void Shutdown();
	
	void CalculateDeltaSeconds();

	// Key Input
	bool HandleChar( unsigned char asKey );
	bool HandleKeyPressed( unsigned char asKey );
	bool HandleKeyReleased( unsigned char asKey );

	// Mouse Input
	void ClickLeftMouse();
	void ClickRightMouse();
	void ClickMiddleMouse();
	void ReleaseLeftMouse();
	void ReleaseRightMouse();	
	void ReleaseMiddleMouse();	
	void StoreWheelDirection(float wheelDelta);

	void LoadDevConsoleFonts();

	inline void HandleCloseApplication()	{ m_isQuitting = true; }
	inline bool IsQuitting()				{ return m_isQuitting; }

	

public:
	bool m_isFirstFrame			= true;
	bool m_intialStartupDone	= false;

	bool m_isQuitting			= false;
	bool m_isPaused				= false;
	bool m_isSlowMo				= false;

	int m_mouseWheelDirection	= 0;
	
	float m_deltaSeconds		= 0.0f;

	double m_timeLastFrameBegan = 0.0f;
	double m_timeNow			= 0.0f;

	BitMapFont* m_squirrelFont	= nullptr;
	Game* m_theGame				= nullptr;

	CommandFrame m_commandFrame;;
};



