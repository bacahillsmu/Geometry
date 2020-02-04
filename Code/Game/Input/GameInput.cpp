#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"

#include "Game/Input/GameInput.hpp"
#include "Game/Framework/App.hpp"

// -----------------------------------------------------------------------
GameInput* g_theGameInput = nullptr;

// -----------------------------------------------------------------------
GameInput::GameInput()
{
	// Initialize Game Input;
	m_arrowKeys[ARROWKEYS_UPARROW]		= false;
	m_arrowKeys[ARROWKEYS_DOWNARROW]	= false;
	m_arrowKeys[ARROWKEYS_LEFTARROW]	= false;
	m_arrowKeys[ARROWKEYS_RIGHTARROW]	= false;
	m_bracketKeys[BRACKETKEYS_LEFT]		= false;
	m_bracketKeys[BRACKETKEYS_RIGHT]	= false;
	m_keyboard[KEY_A]					= false;
	m_keyboard[KEY_B]					= false;
	m_keyboard[KEY_C]					= false;
	m_keyboard[KEY_D]					= false;
	m_keyboard[KEY_E]					= false;
	m_keyboard[KEY_F]					= false;
	m_keyboard[KEY_G]					= false;
	m_keyboard[KEY_H]					= false;
	m_keyboard[KEY_I]					= false;
	m_keyboard[KEY_J]					= false;
	m_keyboard[KEY_K]					= false;
	m_keyboard[KEY_L]					= false;
	m_keyboard[KEY_M]					= false;
	m_keyboard[KEY_N]					= false;
	m_keyboard[KEY_O]					= false;
	m_keyboard[KEY_P]					= false;
	m_keyboard[KEY_Q]					= false;
	m_keyboard[KEY_R]					= false;
	m_keyboard[KEY_S]					= false;
	m_keyboard[KEY_T]					= false;
	m_keyboard[KEY_U]					= false;
	m_keyboard[KEY_V]					= false;
	m_keyboard[KEY_W]					= false;
	m_keyboard[KEY_X]					= false;
	m_keyboard[KEY_Y]					= false;
	m_keyboard[KEY_Z]					= false;
	m_spacebarPressed					= false;
	m_F8Pressed							= false;
}

GameInput::~GameInput()
{

}

// -----------------------------------------------------------------------
// Flow;
// -----------------------------------------------------------------------
void GameInput::Startup()
{

}

// -----------------------------------------------------------------------
void GameInput::BeginFrame()
{

}

// -----------------------------------------------------------------------
void GameInput::EndFrame()
{
	m_arrowKeys[ARROWKEYS_UPARROW]		= false;
	m_arrowKeys[ARROWKEYS_DOWNARROW]	= false;
	m_arrowKeys[ARROWKEYS_LEFTARROW]	= false;
	m_arrowKeys[ARROWKEYS_RIGHTARROW]	= false;
	m_bracketKeys[BRACKETKEYS_LEFT]		= false;
	m_bracketKeys[BRACKETKEYS_RIGHT]	= false;
	m_keyboard[KEY_A] = false;
	m_keyboard[KEY_B] = false;
	m_keyboard[KEY_C] = false;
	m_keyboard[KEY_D] = false;
	m_keyboard[KEY_E] = false;
	m_keyboard[KEY_F] = false;
	m_keyboard[KEY_G] = false;
	m_keyboard[KEY_H] = false;
	m_keyboard[KEY_I] = false;
	m_keyboard[KEY_J] = false;
	m_keyboard[KEY_K] = false;
	m_keyboard[KEY_L] = false;
	m_keyboard[KEY_M] = false;
	m_keyboard[KEY_N] = false;
	m_keyboard[KEY_O] = false;
	m_keyboard[KEY_P] = false;
	m_keyboard[KEY_Q] = false;
	m_keyboard[KEY_R] = false;
	m_keyboard[KEY_S] = false;
	m_keyboard[KEY_T] = false;
	m_keyboard[KEY_U] = false;
	m_keyboard[KEY_V] = false;
	m_keyboard[KEY_W] = false;
	m_keyboard[KEY_X] = false;
	m_keyboard[KEY_Y] = false;
	m_keyboard[KEY_Z] = false;
	m_spacebarPressed					= false;
	m_F8Pressed							= false;
}

// -----------------------------------------------------------------------
void GameInput::Shutdown()
{

}

// -----------------------------------------------------------------------
// Special Keyboard Input;
// -----------------------------------------------------------------------
bool GameInput::IsUpArrowKeyPressed()
{
	return m_arrowKeys[ARROWKEYS_UPARROW];
}

// -----------------------------------------------------------------------
bool GameInput::IsDownArrowKeyPressed()
{
	return m_arrowKeys[ARROWKEYS_DOWNARROW];
}

// -----------------------------------------------------------------------
bool GameInput::IsLeftArrowKeyPressed()
{
	return m_arrowKeys[ARROWKEYS_LEFTARROW];
}

// -----------------------------------------------------------------------
bool GameInput::IsRightArrowKeyPressed()
{
	return m_arrowKeys[ARROWKEYS_RIGHTARROW];
}

// -----------------------------------------------------------------------
bool GameInput::IsLeftBracketKeyPressed()
{
	return m_bracketKeys[BRACKETKEYS_LEFT];
}

// -----------------------------------------------------------------------
bool GameInput::IsRightBracketKeyPressed()
{
	return m_bracketKeys[BRACKETKEYS_RIGHT];
}

// -----------------------------------------------------------------------
bool GameInput::IsSpacebarPressed()
{
	return m_spacebarPressed;
}

// -----------------------------------------------------------------------
// WASD;
// -----------------------------------------------------------------------
bool GameInput::IsWKeyPressed()
{
	return m_keyboard[KEY_W];
}

// -----------------------------------------------------------------------
bool GameInput::IsAKeyPressed()
{
	return m_keyboard[KEY_A];
}

// -----------------------------------------------------------------------
bool GameInput::IsSPressed()
{
	return m_keyboard[KEY_S];
}

// -----------------------------------------------------------------------
bool GameInput::IsDKeyPressed()
{
	return m_keyboard[KEY_D];
}

// -----------------------------------------------------------------------
bool GameInput::IsMKeyPressed()
{
	return m_keyboard[KEY_M];
}

// -----------------------------------------------------------------------
bool GameInput::IsNKeyPressed()
{
	return m_keyboard[KEY_N];
}

// -----------------------------------------------------------------------
bool GameInput::IsLKeyPressed()
{
	return m_keyboard[KEY_L];
}

// -----------------------------------------------------------------------
bool GameInput::IsKKeyPressed()
{
	return m_keyboard[KEY_K];
}

bool GameInput::IsEKeyPressed()
{
	return m_keyboard[KEY_E];
}

bool GameInput::IsSKeyPressed()
{
	return m_keyboard[KEY_S];
}

// -----------------------------------------------------------------------
// Character Keys;
// -----------------------------------------------------------------------
bool GameInput::HandleChar(unsigned char asKey_)
{
	asKey_;
	return false;
}

// -----------------------------------------------------------------------
// Virtual Keys;
// -----------------------------------------------------------------------
bool GameInput::HandleKeyPressed(unsigned char asKey_)
{
	// Handling the Virtual Key press...
	bool virtualKeyHandled = false;

	if(asKey_ == VirtualKey::ESC)
	{
		g_theApp->HandleCloseApplication();

		virtualKeyHandled = true;
	}
	else if(asKey_ == VirtualKey::UPARROW)
	{
		m_arrowKeys[ARROWKEYS_UPARROW] = true;

		virtualKeyHandled = true;
	}
	else if(asKey_ == VirtualKey::DOWNARROW)
	{
		m_arrowKeys[ARROWKEYS_DOWNARROW] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::LEFTARROW)
	{
		m_arrowKeys[ARROWKEYS_LEFTARROW] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::RIGHTARROW)
	{
		m_arrowKeys[ARROWKEYS_RIGHTARROW] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::LEFTBRACKET)
	{
		m_bracketKeys[BRACKETKEYS_LEFT] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::RIGHTBRACKET)
	{
		m_bracketKeys[BRACKETKEYS_RIGHT] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'W')
	{
		m_keyboard[KEY_W] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'A')
	{
		m_keyboard[KEY_A] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'S')
	{
		m_keyboard[KEY_S] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'D')
	{
		m_keyboard[KEY_D] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'M')
	{
		m_keyboard[KEY_M] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'N')
	{
		m_keyboard[KEY_N] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'L')
	{
		m_keyboard[KEY_L] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'K')
	{
		m_keyboard[KEY_K] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'E')
	{
		m_keyboard[KEY_E] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'S')
	{
		m_keyboard[KEY_S] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::SPACEBAR)
	{
		m_spacebarPressed = true;

		virtualKeyHandled = true;
	}
	else if(asKey_ == VirtualKey::F8)
	{
		m_F8Pressed = true;

		virtualKeyHandled = true;
	}

	return virtualKeyHandled;
}

// -----------------------------------------------------------------------
bool GameInput::HandleKeyReleased(unsigned char asKey_)
{
	// Handling the Virtual Key release...
	bool virtualKeyHandled = false;

	if (asKey_ == VirtualKey::UPARROW)
	{
		m_arrowKeys[ARROWKEYS_UPARROW] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::DOWNARROW)
	{
		m_arrowKeys[ARROWKEYS_DOWNARROW] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::LEFTARROW)
	{
		m_arrowKeys[ARROWKEYS_LEFTARROW] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::RIGHTARROW)
	{
		m_arrowKeys[ARROWKEYS_RIGHTARROW] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::LEFTBRACKET)
	{
		m_bracketKeys[BRACKETKEYS_LEFT] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::RIGHTBRACKET)
	{
		m_bracketKeys[BRACKETKEYS_RIGHT] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'W')
	{
		m_keyboard[KEY_W] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'A')
	{
		m_keyboard[KEY_A] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'S')
	{
		m_keyboard[KEY_S] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'D')
	{
		m_keyboard[KEY_D] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'M')
	{
		m_keyboard[KEY_M] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'N')
	{
		m_keyboard[KEY_N] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'L')
	{
		m_keyboard[KEY_L] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'K')
	{
		m_keyboard[KEY_K] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'E')
	{
		m_keyboard[KEY_E] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'S')
	{
		m_keyboard[KEY_S] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::SPACEBAR)
	{
		m_spacebarPressed = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::F8)
	{
		m_F8Pressed = true;

		virtualKeyHandled = true;
	}

	return virtualKeyHandled;
}

bool GameInput::IsF8Pressed()
{
	return m_F8Pressed;
}
