#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"

#include <vector>

// -----------------------------------------------------------------------
struct Tile
{
	bool m_onOff = false;
	IntVec2 m_coord = IntVec2(0, 0);
	AABB2 m_tile;
};

// -----------------------------------------------------------------------
class ConwaysGameOfLife
{

public:

	ConwaysGameOfLife(){}
	~ConwaysGameOfLife(){}

	// Flow;
	void Init();
	void Startup();
	void BeginFrame();
	void Update();
	void Render();
	void EndFrame();

	// Tiles;
	void CreateTiles();
	void Run();

	// Tile Helpers;
	void TurnOffAllTiles();

private:

	int IsNeighborOn(IntVec2 neighborCoord, std::vector<Tile>& tilesSnapshot, IntVec2 dimensions);
	void UpdateSelfBasedOnNeighbors(IntVec2 selfCoord, std::vector<Tile>& tiles, std::vector<Tile>& tilesSnapshot, IntVec2 dimensions);

public:

	// Controlling Speed of the Simulation;
	bool m_isRunning = false;
	int m_frameCounter = 0;
	int m_stepSpeed = 60;	// The smaller the number, the faster the simulation;
	int m_shouldStep = false;	

	// Grid;
	int m_gridSizeID = 0;
	float m_gridSizeOptions[5] = { 27.0f, 32.0f, 36.0f, 48.0f, 54.0f };
	float m_currentGridSize = 0.0f;
	float m_gameWorldAspectRatio = 0.0f;	// Currently using this to keep everything square-ish; Needs revisiting;
	Vec2 m_gridDimensions = Vec2(0.0f, 0.0f);

	// Tiles;
	std::vector<Tile> m_tiles;
	Vec2 m_tileDimension = Vec2(0.0f, 0.0f);

	

};