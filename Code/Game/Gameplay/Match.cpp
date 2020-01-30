#include "Game/Gameplay/Match.hpp"

// ------------------------------------------------------------------
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Framework/GameCommon.hpp"

// ------------------------------------------------------------------
#include "Engine/Input/InputSystem.hpp"

// ------------------------------------------------------------------
#include "Game/Gameplay/Entity.hpp"
#include "Game/Gameplay/Map.hpp"
#include "Game/Gameplay/Player.hpp"





// ------------------------------------------------------------------
Match::Match()
{

}

// ------------------------------------------------------------------
Match::~Match()
{
	for(int e = 0; e < m_entities.size(); ++e)
	{
		DELETE_POINTER(m_entities[e]);
	}

	for (int p = 0; p < m_players.size(); ++p)
	{
		DELETE_POINTER(m_players[p]);
	}

	DELETE_POINTER(m_map);
}

// ------------------------------------------------------------------
void Match::Startup()
{
	m_map = new Map();
	m_map->Startup();

	for (int p = 0; p < m_maxPlayers; ++p)
	{
		m_players.push_back(new Player(p));
	}
}

// ------------------------------------------------------------------
void Match::Update(float deltaSeconds_)
{
	if (g_theInputSystem->IsLeftMouseClicked())
	{
		Vec2 mouseLeftClickPosition = g_theInputSystem->GetMousePosition();
		int slotIndex = m_map->SlotForPosition(mouseLeftClickPosition);
		if(slotIndex >= -1)
		{
			
		}

	}
	
	m_map->Update(deltaSeconds_);

	for (int e = 0; e < m_entities.size(); ++e)
	{
		m_entities[e]->Update(deltaSeconds_);
	}

	for (int p = 0; p < m_players.size(); ++p)
	{
		m_players[p]->Update(deltaSeconds_);
	}
}

// ------------------------------------------------------------------
void Match::Render()
{
	m_map->Render();

	for (int e = 0; e < m_entities.size(); ++e)
	{
		m_entities[e]->Render();
	}
}

