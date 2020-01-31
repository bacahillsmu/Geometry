#include "Game/Gameplay/Map.hpp"

// Commons ----------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Framework/GameCommon.hpp"

// Engine Includes --------------------------------------------------------------------------------
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/WindowContext.hpp"

// Game Includes ----------------------------------------------------------------------------------
#include "Game/Framework/App.hpp"





// ------------------------------------------------------------------------------------------------
Map::Map()
{

}

// ------------------------------------------------------------------------------------------------
Map::~Map()
{

}

// ------------------------------------------------------------------------------------------------
// Flow;
// ------------------------------------------------------------------------------------------------
void Map::Startup()
{
	// Make random convex polys;




}

// ------------------------------------------------------------------------------------------------
void Map::Update(float deltaSeconds_)
{
	m_worldMousePosition.x = RangeMap(g_theInputSystem->GetMousePosition().x, 0.0f, (float)g_theWindowContext->GetClientDimensions().x, 0.0f, WIDTH);
	m_worldMousePosition.y = RangeMap(g_theInputSystem->GetMousePosition().y, 0.0f, (float)g_theWindowContext->GetClientDimensions().y, 0.0f, HEIGHT);
}

// ------------------------------------------------------------------
void Map::Render()
{
	std::vector<Vertex_PCU> mouseVerts;
	AddVertsForDisc2D(mouseVerts, m_worldMousePosition, 1.0f, Rgba::WHITE);

	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
	g_theRenderer->BindTextureViewWithSampler(0, nullptr);
	//g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Jobs/Knight/Knight1M-S.png"));
	g_theRenderer->DrawVertexArray((int)mouseVerts.size(), &mouseVerts[0]);
}


