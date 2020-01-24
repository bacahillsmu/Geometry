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
	m_WASD[WASD_W]						= false;
	m_WASD[WASD_A]						= false;
	m_WASD[WASD_S]						= false;
	m_WASD[WASD_D]						= false;
	m_spacebarPressed					= false;
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
	m_WASD[WASD_W]						= false;
	m_WASD[WASD_A]						= false;
	m_WASD[WASD_S]						= false;
	m_WASD[WASD_D]						= false;
	m_spacebarPressed					= false;
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
	return m_WASD[WASD_W];
}

// -----------------------------------------------------------------------
bool GameInput::IsAKeyPressed()
{
	return m_WASD[WASD_A];
}

// -----------------------------------------------------------------------
bool GameInput::IsSPressed()
{
	return m_WASD[WASD_S];
}

// -----------------------------------------------------------------------
bool GameInput::IsDKeyPressed()
{
	return m_WASD[WASD_D];
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
		m_WASD[WASD_W] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'A')
	{
		m_WASD[WASD_A] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'S')
	{
		m_WASD[WASD_S] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'D')
	{
		m_WASD[WASD_D] = true;

		virtualKeyHandled = true;
	}
	else if (asKey_ == VirtualKey::SPACEBAR)
	{
		m_spacebarPressed = true;

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
		m_WASD[WASD_W] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'A')
	{
		m_WASD[WASD_A] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'S')
	{
		m_WASD[WASD_S] = false;

		virtualKeyHandled = true;
	}
	else if (asKey_ == 'D')
	{
		m_WASD[WASD_D] = false;

		virtualKeyHandled = true;
	}

	return virtualKeyHandled;
}
