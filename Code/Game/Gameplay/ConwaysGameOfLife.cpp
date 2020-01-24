#include "Game/Gameplay/ConwaysGameOfLife.hpp"

// Commons ----------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Framework/GameCommon.hpp"

// Engine Includes --------------------------------------------------------------------------------
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"

// Game Includes ----------------------------------------------------------------------------------
#include "Game/Framework/App.hpp"
#include "Game/Input/GameInput.hpp"

// Callbacks --------------------------------------------------------------------------------------
static bool ResetTiles(EventArgs& args)
{
	UNUSED(args);

	g_theApp->m_theGame->m_conwaysGameOfLife->m_isRunning = false;
	g_theApp->m_theGame->m_conwaysGameOfLife->CreateTiles();

	return true;
}

// -----------------------------------------------------------------------
static bool StartConwaysGameOfLife(EventArgs& args)
{
	UNUSED(args);

	g_theApp->m_theGame->m_conwaysGameOfLife->m_isRunning = true;

	return true;
}

// -----------------------------------------------------------------------
static bool StopConwaysGameOfLife(EventArgs& args)
{
	UNUSED(args);

	g_theApp->m_theGame->m_conwaysGameOfLife->m_isRunning = false;

	return true;
}

// -----------------------------------------------------------------------
static bool StepConwaysGameOfLife(EventArgs& args)
{
	UNUSED(args);

	g_theApp->m_theGame->m_conwaysGameOfLife->m_shouldStep = true;

	return true;
}


// -----------------------------------------------------------------------
// Flow;
// -----------------------------------------------------------------------
void ConwaysGameOfLife::Init()
{
	g_theEventSystem->SubscriptionEventCallbackFunction("reset", ResetTiles);
	g_theEventSystem->SubscriptionEventCallbackFunction("start", StartConwaysGameOfLife);
	g_theEventSystem->SubscriptionEventCallbackFunction("stop", StopConwaysGameOfLife);
	g_theEventSystem->SubscriptionEventCallbackFunction("step", StepConwaysGameOfLife);

	// Currently using this to keep everything square-ish; Needs revisiting;
	m_gameWorldAspectRatio = g_theApp->m_theGame->m_worldMaxs.x / g_theApp->m_theGame->m_worldMaxs.y;
	m_currentGridSize = m_gridSizeOptions[m_gridSizeID];
	m_gridDimensions = Vec2(m_currentGridSize * m_gameWorldAspectRatio, m_currentGridSize);
	m_tileDimension = Vec2(g_theApp->m_theGame->m_worldMaxs.x / m_gridDimensions.x, g_theApp->m_theGame->m_worldMaxs.y / m_gridDimensions.y);
}

// -----------------------------------------------------------------------
void ConwaysGameOfLife::Startup()
{
	// Tiles;
	CreateTiles();	
}

// -----------------------------------------------------------------------
void ConwaysGameOfLife::BeginFrame()
{
	if (m_isRunning)
	{
		++m_frameCounter;
		if (m_frameCounter % m_stepSpeed)
		{
			m_shouldStep = true;
		}
	}
}

// -----------------------------------------------------------------------
void ConwaysGameOfLife::Update()
{
	// Tiles;
	if (!m_isRunning)
	{
		if (g_theInputSystem->WasLeftMouseClickReleased())
		{
			Vec2 mousePosition = g_theInputSystem->GetMousePosition();
			for (int x = 0; x < m_tiles.size(); ++x)
			{
				if (DoesPointIntersectAABB2(mousePosition, m_tiles[x].m_tile))
				{
					m_tiles[x].m_onOff = !m_tiles[x].m_onOff;
					break;
				}
			}
		}
	}
	else if (m_isRunning)
	{
		if (m_shouldStep)
		{
			m_shouldStep = false;
			Run();
		}
	}
}

// -----------------------------------------------------------------------
void ConwaysGameOfLife::Render()
{
	// Making Tiles
	std::vector<Vertex_PCU> boxVerts;
	for (int x = 0; x < m_tiles.size(); ++x)
	{
		if (m_tiles[x].m_onOff)
		{
			AddVertsForAABB2D(boxVerts, m_tiles[x].m_tile, Rgba::WHITE);
		}
		else if (!m_tiles[x].m_onOff)
		{
			AddVertsForAABB2D(boxVerts, m_tiles[x].m_tile, Rgba::BLACK);
		}
	}

	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
	g_theRenderer->BindTextureViewWithSampler(0, nullptr);
	g_theRenderer->DrawVertexArray((int)boxVerts.size(), &boxVerts[0]);
}

// -----------------------------------------------------------------------
void ConwaysGameOfLife::EndFrame()
{

}

// -----------------------------------------------------------------------
// Tiles;
// -----------------------------------------------------------------------
void ConwaysGameOfLife::CreateTiles()
{
	// Making Tiles;
	m_tiles.clear();

	// Temp border around the tiles, I think it looks cooler without the border, but this helps understand space;
	float borderPercent = 0.95f;

	// We are starting in the bottom left of the world;
	// We will place a tile, scoot over by the size of a tile and place another;
	// Then go up a row and repeat;
	Vec2 startingPoint = g_theApp->m_theGame->m_worldMins;	
	Vec2 middleOfTile;
	Vec2 offsetOfTile = m_tileDimension / 2.0f;	// Another name would be halfExtentsOfTile;

	for (int y = 0; y < (int)m_gridDimensions.y; ++y)
	{
		for (int x = 0; x < (int)m_gridDimensions.x; ++x)
		{
			// Find the center point of this tile based on the current iteration's starting point;
			middleOfTile = startingPoint + offsetOfTile;

			// Create a tile;
			Tile tile;
			tile.m_tile			= AABB2(middleOfTile, offsetOfTile * borderPercent);
			tile.m_onOff		= false;
			tile.m_coord		= IntVec2(x, y);

			m_tiles.push_back(tile);

			// Scoot the starting point to be on the bottom right of the newly created tile;
			// This is the bottom left of the new-to-be tile in the current row;
			startingPoint.x += m_tileDimension.x;
		}

		// A row has finished being created;
		// Reset our x-position to be on the far left of our world;
		startingPoint.x = g_theApp->m_theGame->m_worldMins.x;

		// Increment our y-position, we are now on the top left of the tile below us, but the bottom left of the new-to-be tile;
		startingPoint.y += m_tileDimension.y;
	}
}

// -----------------------------------------------------------------------
void ConwaysGameOfLife::Run()
{
	std::vector<Tile> tilesSnapshot = m_tiles;

	TurnOffAllTiles();

	for (int x = 0; x < m_tiles.size(); ++x)
	{
		UpdateSelfBasedOnNeighbors(m_tiles[x].m_coord, m_tiles, tilesSnapshot, IntVec2((int)m_gridDimensions.x, (int)m_gridDimensions.y));
	}

}

// -----------------------------------------------------------------------
// Tile Helpers;
// -----------------------------------------------------------------------
void ConwaysGameOfLife::TurnOffAllTiles()
{
	for (int x = 0; x < m_tiles.size(); ++x)
	{
		m_tiles[x].m_onOff = false;
	}
}

// -----------------------------------------------------------------------
// Private;
// -----------------------------------------------------------------------
int ConwaysGameOfLife::IsNeighborOn(IntVec2 neighborCoord, std::vector<Tile>& tilesSnapshot, IntVec2 dimensions)
{
	if (IsTileCoordInBounds(neighborCoord, dimensions))
	{
		int neighborID = GetIndexFromCoord(neighborCoord, dimensions);
		if (tilesSnapshot[neighborID].m_onOff)
		{
			return 1;
		}
	}

	return 0;
}

// -----------------------------------------------------------------------
void ConwaysGameOfLife::UpdateSelfBasedOnNeighbors(IntVec2 selfCoord, std::vector<Tile>& tiles, std::vector<Tile>& tilesSnapshot, IntVec2 dimensions)
{
	int neighborCount = 0;

	// Top, Bottom, Left, Right, TopLeft, TopRight, BottomRight, BottomLeft
	neighborCount += IsNeighborOn(selfCoord + IntVec2(0, 1), tilesSnapshot, dimensions);
	neighborCount += IsNeighborOn(selfCoord + IntVec2(0, -1), tilesSnapshot, dimensions);
	neighborCount += IsNeighborOn(selfCoord + IntVec2(-1, 0), tilesSnapshot, dimensions);
	neighborCount += IsNeighborOn(selfCoord + IntVec2(1, 0), tilesSnapshot, dimensions);
	neighborCount += IsNeighborOn(selfCoord + IntVec2(-1, 1), tilesSnapshot, dimensions);
	neighborCount += IsNeighborOn(selfCoord + IntVec2(1, 1), tilesSnapshot, dimensions);
	neighborCount += IsNeighborOn(selfCoord + IntVec2(1, -1), tilesSnapshot, dimensions);
	neighborCount += IsNeighborOn(selfCoord + IntVec2(-1, -1), tilesSnapshot, dimensions);

	// Rules;
	int selfID = GetIndexFromCoord(selfCoord, dimensions);
	if (neighborCount < 2)
	{
		tiles[selfID].m_onOff = false;
	}
	else if ((neighborCount == 2 || neighborCount == 3) && tilesSnapshot[selfID].m_onOff)
	{
		tiles[selfID].m_onOff = true;
	}
	else if (neighborCount == 3)
	{
		tiles[selfID].m_onOff = true;
	}
	else if (neighborCount > 3)
	{
		tiles[selfID].m_onOff = false;
	}
}
