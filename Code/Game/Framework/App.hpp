#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"


class Game;
class BitMapFont;

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
	void Update();
	void Render();
	void EndFrame();
	void Shutdown();
	
	void CalculateDeltaSeconds();

	// Character Keys;
	bool HandleChar( unsigned char asKey_ );

	// Virtual Keys;
	bool HandleKeyPressed( unsigned char asKey_ );
	bool HandleKeyReleased( unsigned char asKey_ );

	inline void HandleCloseApplication()	{ m_isQuitting = true; }
	inline bool IsQuitting()				{ return m_isQuitting; }
	

public:

	bool m_isFirstFrame			= true;
	bool m_intialStartupDone	= false;

	bool m_isQuitting			= false;
	bool m_isPaused				= false;
	bool m_isSlowMo				= false;
	
	double m_timeLastFrameBegan = 0.0f;
	double m_timeNow			= 0.0f;
	float m_deltaSeconds = 0.0f;

	Game* m_theGame				= nullptr;
};

extern App* g_theApp;

