#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Framework/GameCommon.hpp"

struct Camera;
class LobbyConsole;

#include <vector>

extern LobbyConsole* g_theLobbyConsole;


class LobbyConsole
{

public:

	LobbyConsole();
	~LobbyConsole();

	void Startup();
	void Render();

	// The Console itself;
	void DrawConsole();
	void DrawBackground();
	void DrawTypingBox();

	// Cursor;
	void DrawCursor();
	void UpdateCursorPosition(int moveDirection);
	void UpdateCursorPositionWithShift(int moveDirection);

	// Text;
	void DisplayTypingText();
	void DisplayChatText();
	void HighlightTypingText();
	void AddLetterToTypingText( const std::string& letter );
	void AddLineToChatText(const std::string& line);

	// Typing;
	bool HandleKeyPressed(unsigned char asKey);
	void Backspace();
	void Delete();
	void Enter();


	// Helpers;
	inline bool IsOpen() { return m_isOpen; }
	void ResetValues();
	void ResetHighlight();
	void HighlightCheck();

	


public:

	bool m_isOpen = false;
	float m_startTime = 0.0f;

	// Cursor;
	int m_cursorPosition = 0;

	// Text;
	float m_lineHeight = 0.4f;
	std::string m_currentTypingText = "";

	// Text Highlight;
	int m_startCursorPositionShift = -1;
	Vec2 m_startHighlightPosition = Vec2(0.0f, 0.0f);
	Vec2 m_endHighlightPosition = Vec2(0.0f, 0.0f);

	// Camera;
	Camera* m_theLobbyCamera = nullptr;
	BitMapFont* m_font = nullptr;


	std::vector<std::string> m_texts;

};