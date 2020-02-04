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

enum Key
{
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z
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

	// Keyboard;
	bool IsWKeyPressed();
	bool IsAKeyPressed();
	bool IsSPressed();
	bool IsDKeyPressed();
	bool IsMKeyPressed();
	bool IsNKeyPressed();
	bool IsLKeyPressed();
	bool IsKKeyPressed();
	bool IsEKeyPressed();
	bool IsSKeyPressed();

	// Character Keys;
	bool HandleChar(unsigned char asKey_);

	// Virtual Keys;
	bool HandleKeyPressed(unsigned char asKey_);
	bool HandleKeyReleased(unsigned char asKey_);

	// Additional Game Keys;
	bool IsF8Pressed();

private:

	// Keyboard Input;
	bool m_arrowKeys[4];
	bool m_bracketKeys[2];
	bool m_keyboard[26];
	bool m_spacebarPressed = false;
	bool m_F8Pressed = false;
};

extern GameInput* g_theGameInput;