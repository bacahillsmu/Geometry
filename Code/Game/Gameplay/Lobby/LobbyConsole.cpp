#include "ThirdParty/RakNet/RakNetInterface.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Game/Gameplay/Lobby/LobbyConsole.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Framework/CommandFrame.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/BitMapFont.hpp"
#include "Engine/Input/InputSystem.hpp"



LobbyConsole* g_theLobbyConsole = nullptr;



// -----------------------------------------------------------------------
LobbyConsole::LobbyConsole()
{
	m_font = g_theRenderer->CreateOrGetBitmapFont("SquirrelFixedFont");
}

// -----------------------------------------------------------------------
LobbyConsole::~LobbyConsole()
{
	delete m_theLobbyCamera;
	m_theLobbyCamera = nullptr;
}

// -----------------------------------------------------------------------
void LobbyConsole::Startup()
{
	m_startTime = (float)GetCurrentTimeSeconds();

	m_theLobbyCamera = new Camera();
	m_theLobbyCamera->SetOrthographicProjection( Vec2( 0.0f, 0.0f ), Vec2( 20.0f, 10.0f ) );
}

// -----------------------------------------------------------------------
void LobbyConsole::Render()
{
	if(!m_isOpen)
	{
		return;
	}

	m_theLobbyCamera->SetModelMatrix(Matrix4x4::IDENTITY);
	g_theRenderer->BindModelMatrix(m_theLobbyCamera->m_cameraModel);
	m_theLobbyCamera->SetColorTargetView(g_theRenderer->GetFrameColorTarget());
	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");

	g_theRenderer->BeginCamera(m_theLobbyCamera);

	// Console;
	DrawConsole();
	DrawCursor();

	// Text;
	DisplayTypingText();
	HighlightTypingText();
	DisplayChatText();

	g_theRenderer->EndCamera();
}

// -----------------------------------------------------------------------
void LobbyConsole::DrawConsole()
{
	g_theRenderer->BindTextureViewWithSampler(0, nullptr);

	DrawBackground();
	DrawTypingBox();
}

// -----------------------------------------------------------------------
void LobbyConsole::DrawBackground()
{
	std::vector<Vertex_PCU> backGroundQuad;

	AABB2 whiteBox = AABB2::MakeFromMinsMaxs( Vec2( 0.0f, 0.0f ), Vec2( 20.0f, 3.0f ) );
	AABB2 blackBox = AABB2::MakeFromMinsMaxs( Vec2( 0.05f, 0.05f ), Vec2( 19.95f, 2.95f ) );	

	AddVertsForAABB2D(backGroundQuad, whiteBox, Rgba::WHITE);
	AddVertsForAABB2D(backGroundQuad, blackBox, Rgba(0.0f, 0.0f, 0.0f, 1.0f));	

	g_theRenderer->DrawVertexArray( backGroundQuad );
}

// -----------------------------------------------------------------------
void LobbyConsole::DrawTypingBox()
{
	std::vector<Vertex_PCU> typingAreaQuad;

	AABB2 border = AABB2::MakeFromMinsMaxs( Vec2( 0.1f, 0.1f ), Vec2( 19.85f, 0.75f ) );
	AABB2 box = AABB2::MakeFromMinsMaxs( Vec2( 0.15f, 0.15f ), Vec2( 19.8f, 0.7f ) );	

	AddVertsForAABB2D(typingAreaQuad, border, Rgba::WHITE);
	AddVertsForAABB2D(typingAreaQuad, box, Rgba::BLACK);	

	g_theRenderer->DrawVertexArray( typingAreaQuad );
}

// -----------------------------------------------------------------------
// Cursor;
// -----------------------------------------------------------------------
void LobbyConsole::DrawCursor()
{
	g_theRenderer->BindTextureViewWithSampler(0, nullptr);
	std::vector<Vertex_PCU> consoleCursorQuad;

	float textOffset = m_cursorPosition * 0.1f;

	Vec2 bottomLeft = Vec2( 0.152f + textOffset, 0.18f );
	Vec2 topRight = Vec2( 0.162f + textOffset, 0.65f );

	AABB2 box = AABB2::MakeFromMinsMaxs(bottomLeft, topRight);

	AddVertsForAABB2D(consoleCursorQuad, box, Rgba::WHITE);
	int time = (int)GetCurrentTimeSeconds();
	if(time % 2 == 0)
	{
		g_theRenderer->DrawVertexArray( consoleCursorQuad );
	}
}

// -----------------------------------------------------------------------
void LobbyConsole::UpdateCursorPosition( int moveDirection )
{
	if(m_startCursorPositionShift == -1)
	{
		m_cursorPosition += moveDirection * 2;
		m_cursorPosition = (int)Clamp((float)m_cursorPosition, (float)0, (float)(m_currentTypingText.size() * 2));
	}
	else
	{
		if(m_cursorPosition < m_startCursorPositionShift && moveDirection == 1)
		{
			m_cursorPosition = m_startCursorPositionShift;
		}
		else if(m_cursorPosition > m_startCursorPositionShift && moveDirection == -1)
		{
			m_cursorPosition = m_startCursorPositionShift;
		}
	}

	ResetValues();
}

// -----------------------------------------------------------------------
void LobbyConsole::UpdateCursorPositionWithShift( int moveDirection )
{
	if(m_startCursorPositionShift == -1)
	{
		m_startCursorPositionShift = m_cursorPosition;
	}	

	m_cursorPosition += moveDirection * 2;
	m_cursorPosition = (int)Clamp((float)m_cursorPosition, (float)0, (float)(m_currentTypingText.size() * 2));

	HighlightCheck();
}

// -----------------------------------------------------------------------
// Text;
// -----------------------------------------------------------------------
void LobbyConsole::DisplayTypingText()
{
	float cellHeight = 0.2f;

	std::vector<Vertex_PCU> textVerts;
	Vec2 textStartPosition = Vec2( 0.15f, 0.15f );

	m_font->AddVertsForText2D(textVerts, textStartPosition, cellHeight, m_currentTypingText, Rgba::WHITE);

	if((int)textVerts.size() > 0)
	{
		g_theRenderer->BindTextureView( 0u, m_font->GetTextureView() );
		g_theRenderer->DrawVertexArray((int)textVerts.size(), &textVerts[0]);
	}
}

// -----------------------------------------------------------------------
void LobbyConsole::DisplayChatText()
{
	float cellHeight = 0.2f;

	std::vector<Vertex_PCU> textVerts;
	Vec2 textStartPosition( 0.5f, 1.5f);

	for(int textIndex = 0; textIndex < m_texts.size(); textIndex++)
	{
		Vec2 printPosition(0.0f, (float)(m_texts.size() - textIndex));

		m_font->AddVertsForText2D(textVerts, (textStartPosition + printPosition) * 0.4f, cellHeight, m_texts[textIndex], Rgba::WHITE);
	}

	if((int)textVerts.size() > 0)
	{
		g_theRenderer->BindTextureView( 0u, m_font->GetTextureView() );
		g_theRenderer->DrawVertexArray((int)textVerts.size(), &textVerts[0]);
	}
}

// -----------------------------------------------------------------------
void LobbyConsole::HighlightTypingText()
{
	if(m_startHighlightPosition != Vec2(0.0f, 0.0f) && m_endHighlightPosition != Vec2(0.0f, 0.0f))
	{
		AABB2 box = AABB2::MakeFromMinsMaxs(m_startHighlightPosition, m_endHighlightPosition);
		std::vector<Vertex_PCU> someQuad;
		Rgba boxColor = Rgba(1.0f, 1.0f, 1.0f, 0.5f);

		AddVertsForAABB2D(someQuad, box, boxColor);

		g_theRenderer->BindTextureViewWithSampler(0, nullptr);
		g_theRenderer->DrawVertexArray( someQuad );
	}
}

// -----------------------------------------------------------------------
void LobbyConsole::AddLetterToTypingText( const std::string& letter )
{
	std::string firstChunk = m_currentTypingText.substr(0, m_cursorPosition/2);
	std::string secondChunk = m_currentTypingText.substr(m_cursorPosition/2, m_currentTypingText.size());

	m_currentTypingText = firstChunk + letter + secondChunk;
}

// -----------------------------------------------------------------------
void LobbyConsole::AddLineToChatText( const std::string& line )
{
	m_texts.push_back(line);
}

// -----------------------------------------------------------------------
// Typing;
// -----------------------------------------------------------------------
bool LobbyConsole::HandleKeyPressed( unsigned char asKey )
{
	if(!IsOpen())
	{
		return false;
	}

	if(asKey == Key::RIGHTARROW)
	{
		if(InputSystem::IsShiftDown())
		{
			UpdateCursorPositionWithShift(1);
		}
		else
		{
			UpdateCursorPosition(1);			
		}
	}

	if(asKey == Key::LEFTARROW)
	{
		if(InputSystem::IsShiftDown())
		{
			UpdateCursorPositionWithShift(-1);
		}
		else
		{
			UpdateCursorPosition(-1);			
		}		
	}

	if(asKey == Key::BACKSPACE)
	{
		Backspace();
		ResetValues();
	}

	if(asKey == Key::DEL)
	{
		Delete();
		ResetValues();
	}

	if(asKey == Key::ENTER)
	{
		Enter();
	}

	return true;



}

// -----------------------------------------------------------------------
void LobbyConsole::Backspace()
{
	std::string firstChunk = m_currentTypingText.substr(0, m_cursorPosition/2);
	std::string secondChunk = m_currentTypingText.substr(m_cursorPosition/2, m_currentTypingText.size());

	if(m_startCursorPositionShift == -1)
	{
		firstChunk = firstChunk.substr(0, (m_cursorPosition/2) - 1);
		UpdateCursorPosition(-1);
	}
	else if(m_startCursorPositionShift != -1)
	{
		if(m_startCursorPositionShift < m_cursorPosition)
		{
			firstChunk = m_currentTypingText.substr(0, (m_startCursorPositionShift/2));
			secondChunk = m_currentTypingText.substr((m_cursorPosition/2), m_currentTypingText.size());
			m_cursorPosition = m_startCursorPositionShift;
		}
		else if(m_cursorPosition < m_startCursorPositionShift)
		{
			firstChunk = m_currentTypingText.substr(0, (m_cursorPosition/2));
			secondChunk = m_currentTypingText.substr((m_startCursorPositionShift/2), m_currentTypingText.size());
		}

		UpdateCursorPosition(0);
	}

	m_currentTypingText = firstChunk + secondChunk;
}

// -----------------------------------------------------------------------
void LobbyConsole::Delete()
{
	std::string firstChunk = m_currentTypingText.substr(0, m_cursorPosition/2);
	std::string secondChunk = m_currentTypingText.substr(m_cursorPosition/2, m_currentTypingText.size());

	if(m_startCursorPositionShift == -1)
	{
		if(secondChunk.size() != 0)
		{
			secondChunk = secondChunk.substr(1, m_currentTypingText.size());
		}	
	}
	else if(m_startCursorPositionShift != -1)
	{
		if(m_startCursorPositionShift < m_cursorPosition)
		{
			firstChunk = m_currentTypingText.substr(0, (m_startCursorPositionShift/2));
			secondChunk = m_currentTypingText.substr((m_cursorPosition/2), m_currentTypingText.size());
			m_cursorPosition = m_startCursorPositionShift;
		}
		else if(m_cursorPosition < m_startCursorPositionShift)
		{
			firstChunk = m_currentTypingText.substr(0, (m_cursorPosition/2));
			secondChunk = m_currentTypingText.substr((m_startCursorPositionShift/2), m_currentTypingText.size());
		}

		UpdateCursorPosition(0);
	}


	m_currentTypingText = firstChunk + secondChunk;
}

// -----------------------------------------------------------------------
void LobbyConsole::Enter()
{
	if(m_currentTypingText.size() == 0)
	{
		return;
	}

	unsigned short numberOfSystems;
	g_theRakNetInterface->m_peer->GetConnectionList( 0, &numberOfSystems );

	if(numberOfSystems > 0)
	{
		g_theRakNetInterface->SendMessage(g_theRakNetInterface->m_themSystemAddress, m_currentTypingText);
		RakNet::RakNetGUID guid = g_theRakNetInterface->m_peer->GetMyGUID();
		std::string newText = Stringf("[%s]: %s", guid.ToString(), m_currentTypingText.c_str());
		AddLineToChatText(newText);
	}
	else
	{
		AddLineToChatText(m_currentTypingText);
	}

	

	m_currentTypingText = "";
	m_cursorPosition = 0;
	ResetHighlight();
	ResetValues();
}

// -----------------------------------------------------------------------
// Helpers;
// -----------------------------------------------------------------------
void LobbyConsole::ResetValues()
{
	m_startCursorPositionShift = -1;
	m_startHighlightPosition = Vec2(0.0f, 0.0f);
	m_endHighlightPosition = Vec2(0.0f, 0.0f);
}

// -----------------------------------------------------------------------
void LobbyConsole::ResetHighlight()
{
	if(m_startCursorPositionShift != -1)
	{
		m_cursorPosition = m_startCursorPositionShift;
	}
	m_startCursorPositionShift = -1;
	m_startHighlightPosition = Vec2(0.0f, 0.0f);
	m_endHighlightPosition = Vec2(0.0f, 0.0f);
}

// -----------------------------------------------------------------------
void LobbyConsole::HighlightCheck()
{
	if(m_startCursorPositionShift != -1)
	{
		if(m_startCursorPositionShift < m_cursorPosition)
		{
			m_startHighlightPosition = Vec2(0.15f + (float)(m_startCursorPositionShift) * 0.1f, 0.15f);
			m_endHighlightPosition = Vec2(0.15f + (float)(m_cursorPosition) * 0.1f , 0.7f);
		}
		else if(m_cursorPosition < m_startCursorPositionShift)
		{
			m_startHighlightPosition = Vec2(0.15f + (float)(m_cursorPosition) * 0.1f, 0.18f);
			m_endHighlightPosition = Vec2(0.15f + (float)(m_startCursorPositionShift) * 0.1f, 0.7f);
		}
		else
		{
			m_startHighlightPosition = Vec2(0.0f, 0.0f);
			m_endHighlightPosition = Vec2(0.0f, 0.0f);
		}
	}
}


