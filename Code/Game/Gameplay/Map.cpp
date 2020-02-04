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
	
}

// ------------------------------------------------------------------
void Map::Render()
{
	std::vector<Vertex_PCU> mouseVerts;
	AddVertsForDisc2D(mouseVerts, g_theApp->m_theGame->m_worldMousePosition, 1.0f, Rgba::BLACK);

	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
	g_theRenderer->BindTextureViewWithSampler(0, nullptr);
	//g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Jobs/Knight/Knight1M-S.png"));
	g_theRenderer->DrawVertexArray((int)mouseVerts.size(), &mouseVerts[0]);
}


