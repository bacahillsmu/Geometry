#pragma once

struct IntVec2;

enum ArrowKeys
{
	ARROWKEYS_UPARROW,
	ARROWKEYS_DOWNARROW,
	ARROWKEYS_LEFTARROW,
	ARROWKEYS_RIGHTARROW
};

enum BracketKeys
{
	BRACKETKEYS_LEFT,
	BRACKETKEYS_RIGHT
};

enum WASD
{
	WASD_W,
	WASD_A,
	WASD_S,
	WASD_D
};

class GameInput
{

public:

	GameInput();
	~GameInput();

	// Flow;
	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	// Special Keyboard Input;
	bool IsUpArrowKeyPressed();
	bool IsDownArrowKeyPressed();
	bool IsLeftArrowKeyPressed();
	bool IsRightArrowKeyPressed();
	bool IsLeftBracketKeyPressed();
	bool IsRightBracketKeyPressed();
	bool IsSpacebarPressed();

	// WASD;
	bool IsWKeyPressed();
	bool IsAKeyPressed();
	bool IsSPressed();
	bool IsDKeyPressed();

	// Character Keys;
	bool HandleChar(unsigned char asKey_);

	// Virtual Keys;
	bool HandleKeyPressed(unsigned char asKey_);
	bool HandleKeyReleased(unsigned char asKey_);

private:

	// Keyboard Input;
	bool m_arrowKeys[4];
	bool m_bracketKeys[2];
	bool m_WASD[4];
	bool m_spacebarPressed = false;
};

extern GameInput* g_theGameInput;