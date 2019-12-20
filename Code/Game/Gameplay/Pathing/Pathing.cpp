#include "Pathing.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time.hpp"




// -----------------------------------------------------------------------
// Pather
// -----------------------------------------------------------------------
void Pather::Init( IntVec2 size, float initialCost )
{
	m_tileCosts.clear();
	m_tileCosts.resize(size.x * size.y);

	for(int tileCostIndex = 0; tileCostIndex < m_tileCosts.size(); tileCostIndex++)
	{
		m_tileCosts[tileCostIndex] = initialCost;
	}
}

// -----------------------------------------------------------------------
void Pather::ResetTileCosts( float cost )
{
	for(int tileCostIndex = 0; tileCostIndex < m_tileCosts.size(); tileCostIndex++)
	{
		m_tileCosts[tileCostIndex] = cost;
	}
}

// -----------------------------------------------------------------------
void Pather::SetCostTile( int tileIndex, float cost )
{
	m_tileCosts[tileIndex] = cost;
}

// -----------------------------------------------------------------------
void Pather::AddCostTile( int tileIndex, float cost )
{
	m_tileCosts[tileIndex] += cost;
}

// -----------------------------------------------------------------------
PathCreation* Pather::CreatePath( IntVec2 startTile, IntVec2 terminationPoint, IntVec2 m_tileDimensions )
{
	std::vector<IntVec2> terminationPoints;
	terminationPoints.push_back(terminationPoint);

	PathCreation* pathCreation = CreatePath(startTile, terminationPoints, m_tileDimensions);

	return pathCreation;
}

// -----------------------------------------------------------------------
PathCreation* Pather::CreatePath(IntVec2 startTile, std::vector<IntVec2>& terminationPoints, IntVec2 tileDimensions)
{
	m_pathInfo.clear();
	m_pathInfo.resize(tileDimensions.x * tileDimensions.y);
	
	// Add the Index of each Termination Coord into a vector;
	for(IntVec2 terminationTileCoord: terminationPoints)
	{
		int tileIndex = GetIndexFromCoord(terminationTileCoord, tileDimensions);
		m_terminationTileIndexList.push_back(tileIndex);
	}
	int earlyOutTerminationIndex = 0;

	// Get the Index of the Starting Coord;
	int startingTileIndex = GetIndexFromCoord(startTile, tileDimensions);

	// Create our Starting Tile PathInfo;
	m_pathInfo[startingTileIndex].cost = 0;
	m_pathInfo[startingTileIndex].pathState = PATH_STATE_FINISHED;

	// Add the Starting Index to the OpenTileList;
	m_openTileIndexList.push_back(startingTileIndex);
	
	// Begin the Dijkstra!
	while(m_openTileIndexList.size() > 0)
	{
		// Get my Best Index from the Open List and then remove it from the Open List;
		int openIndexListSlot = 0;
		int currentIndex = GetIndexOfSmallestCostFromOpenList(openIndexListSlot);
		m_openTileIndexList.erase(m_openTileIndexList.begin() + openIndexListSlot);

		// Set the state of this index to be finished, we are going to process it now;
		m_pathInfo[currentIndex].pathState = PATH_STATE_FINISHED;

		// If we have our Termination Index, we know we have the shortest path set already;
		if(IsIndexInTerminationIndexList(currentIndex, m_terminationTileIndexList))
		{
			earlyOutTerminationIndex = currentIndex;
			break;
		}

		// If we do not have any of our Termination Indexes, then lets check our Neighbors;
		std::vector<int> boundedNeighborIndexs = GetBoundedNeighbors(currentIndex, tileDimensions);
		for(int boundedNeighborIndex: boundedNeighborIndexs)
		{
			// Calculate the costs to get to our Neighbors;
			float originalCost = m_pathInfo[boundedNeighborIndex].cost;
			m_pathInfo[boundedNeighborIndex].cost = GetMin(m_pathInfo[boundedNeighborIndex].cost, m_pathInfo[currentIndex].cost + m_tileCosts[boundedNeighborIndex]);
			
			// If we found a cheaper path to our Neighbor, then update our Parent;
			if(m_pathInfo[boundedNeighborIndex].cost < originalCost)
			{
				m_pathInfo[boundedNeighborIndex].parentIndex = currentIndex;
			}

			// If we never have visited this Coord, then put it in the Open List;
			if(m_pathInfo[boundedNeighborIndex].pathState == PATH_STATE_UNVISITED)
			{
				m_pathInfo[boundedNeighborIndex].pathState = PATH_STATE_OPEN;
				m_openTileIndexList.push_back(boundedNeighborIndex);
			}
		}
	}

	// Work backwards from our Termination Point to the Starting Point;
	Path* path = new Path();
	IntVec2 pathCoord = GetCoordFromIndex(earlyOutTerminationIndex, tileDimensions);
	path->push_back(pathCoord);

	int nextIndex = m_pathInfo[earlyOutTerminationIndex].parentIndex;

	bool workingBackwards = true;
	while(workingBackwards)
	{
		pathCoord = GetCoordFromIndex(nextIndex, tileDimensions);
		path->push_back(pathCoord);

		if(nextIndex == -1)
		{
			pathCoord = GetCoordFromIndex(nextIndex, tileDimensions);
			workingBackwards = false;
		}
		else
		{
			nextIndex = m_pathInfo[nextIndex].parentIndex;
		}
	}

	m_openTileIndexList.clear();
	m_terminationTileIndexList.clear();
	m_pathInfo.clear();

	PathCreation* pathCreation = new PathCreation();
	pathCreation->m_path = path;
	pathCreation->m_pathCreationTimeStamp = GetCurrentTimeSeconds();

	return pathCreation;
}

// -----------------------------------------------------------------------
bool Pather::IsTileInEndTiles( IntVec2 currentTile, std::vector<IntVec2>* endTiles )
{
	for(IntVec2& endTile: *endTiles)
	{
		if(currentTile == endTile)
		{
			return true;
		}
	}

	return false;
}

// -----------------------------------------------------------------------
int Pather::GetIndexOfSmallestCostFromOpenList(int& outSlot)
{
	int counter = 0;
	int smallestCostIndex = m_openTileIndexList.front();
	float smallestCost = m_pathInfo[smallestCostIndex].cost;

	for(int tileIndex: m_openTileIndexList)
	{
		if(m_pathInfo[tileIndex].cost < smallestCost)
		{
			smallestCostIndex = tileIndex;
			smallestCost = m_pathInfo[tileIndex].cost;
			outSlot = counter;
		}
		counter++;
	}
	
	return smallestCostIndex;
}

// -----------------------------------------------------------------------
bool Pather::IsIndexInTerminationIndexList( int index, std::vector<int>& terminationIndexList )
{
	for(int terminationIndex: terminationIndexList)
	{
		if(index == terminationIndex)
		{
			return true;
		}
	}

	return false;
}

// -----------------------------------------------------------------------
std::vector<int> Pather::GetBoundedNeighbors( int index, IntVec2 m_tileDimensions )
{
	std::vector<int> boundedNeighbors;

	IntVec2 tileCoord = GetCoordFromIndex(index, m_tileDimensions);

	IntVec2 north = IntVec2(tileCoord.x, tileCoord.y + 1);
	IntVec2 south = IntVec2(tileCoord.x, tileCoord.y - 1);
	IntVec2 east = IntVec2(tileCoord.x + 1, tileCoord.y);
	IntVec2 west = IntVec2(tileCoord.x - 1, tileCoord.y);

	// Check each direction to see if it is in the Dimensions;
	if(IsTileCoordInBounds(north, m_tileDimensions))
	{
		boundedNeighbors.push_back(GetIndexFromCoord(north, m_tileDimensions));
	}

	if(IsTileCoordInBounds(south, m_tileDimensions))
	{
		boundedNeighbors.push_back(GetIndexFromCoord(south, m_tileDimensions));
	}

	if(IsTileCoordInBounds(east, m_tileDimensions))
	{
		boundedNeighbors.push_back(GetIndexFromCoord(east, m_tileDimensions));
	}

	if(IsTileCoordInBounds(west, m_tileDimensions))
	{
		boundedNeighbors.push_back(GetIndexFromCoord(west, m_tileDimensions));
	}

	return boundedNeighbors;
}

// -----------------------------------------------------------------------
int Pather::GetCurrentTileIndexFromPath( IntVec2 currentTile, Path* path )
{
	for(int pathIndex = 0; pathIndex < path->size(); pathIndex++)
	{
		if(currentTile == (*path)[pathIndex])
		{
			return pathIndex;
		}
	}

	return -1;
}

// -----------------------------------------------------------------------
IntVec2 Pather::GetClosestTileOnPath( Path* path, Vec3 position )
{
	IntVec2 closestTile = path->back();
	float distance = INFINITY;

	for(int pathIndex = 0; pathIndex < path->size(); pathIndex++)
	{
		IntVec2 checkTile = (*path)[pathIndex];
		float checkDistance = GetDistance(position, Vec3((float)checkTile.x, (float)checkTile.y, 0.0f));
		if(checkDistance < distance)
		{
			distance = checkDistance;
			closestTile = checkTile;
		}
	}

	return closestTile;
}

IntVec2 Pather::GetChildTile( Path path, IntVec2 parentTile )
{
	if(path.size() > 1)
	{
		for(int i = 0; i < path.size(); i++)
		{
			IntVec2 check = path[i];
			if(check == parentTile)
			{
				if(i != 0)
				{
					return path[i-1];
				}
			}
		}
	}

	return parentTile;
}

// -----------------------------------------------------------------------
// PathSolver
// -----------------------------------------------------------------------

