#pragma once

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

#include <math.h>
#include <vector>

enum PathState
{
	PATH_STATE_UNVISITED = 0,
	PATH_STATE_OPEN,
	PATH_STATE_FINISHED
};


struct PathInfo
{
	float cost = INFINITY;
	int parentIndex = -1;
	PathState pathState = PATH_STATE_UNVISITED;
};

typedef std::vector<IntVec2> Path;
class PathCreation
{

public:

	~PathCreation()
	{ 
		delete m_path; 
		m_path = nullptr; 
	}

	Path* m_path = nullptr;
	double m_pathCreationTimeStamp = 0.0;
};



// -----------------------------------------------------------------------
// Pather
// -----------------------------------------------------------------------
class Pather
{

public:

	Pather() {}
	void Init(IntVec2 size, float initialCost);

	void ResetTileCosts(float cost);
	void SetCostTile(int tileIndex, float cost);
	void AddCostTile(int tileIndex, float cost);

	PathCreation* CreatePath(IntVec2 startTile, IntVec2 terminationPoint, IntVec2 m_tileDimensions);
	PathCreation* CreatePath(IntVec2 startTile, std::vector<IntVec2>& terminationPoints, IntVec2 m_tileDimensions);

	// Helpers;
	bool IsTileInEndTiles(IntVec2 currentTile, std::vector<IntVec2>* endTiles);
	int GetIndexOfSmallestCostFromOpenList(int& outSlot);
	bool IsIndexInTerminationIndexList(int index, std::vector<int>& terminationIndexList);
	std::vector<int> GetBoundedNeighbors(int index, IntVec2 m_tileDimensions);

	int GetCurrentTileIndexFromPath(IntVec2 currentTile, Path* path);
	IntVec2 GetClosestTileOnPath(Path* path, Vec3 position);
	IntVec2 GetChildTile(Path path, IntVec2 parentTile);

public:

	std::vector<int> m_openTileIndexList;
	std::vector<int> m_terminationTileIndexList;
	std::vector<PathInfo> m_pathInfo;
	

	// This gets updated at the beginning of every Frame;
	std::vector<float> m_tileCosts;
	

	
	

	

};

// -----------------------------------------------------------------------
// PathSolver
// -----------------------------------------------------------------------
class PathSolver
{

public:

	PathSolver(Pather* pather, IntVec2 startTile, std::vector<IntVec2>& endTiles);
	
	bool Step();
	void Solve();

	bool GetPath(Path* outPath, IntVec2 tile);


private:

	void VisitDijkstra(IntVec2 cell, IntVec2 parent, float parentCost);


private:

	Pather* m_pather = nullptr;
	

};



