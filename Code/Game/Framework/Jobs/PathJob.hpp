#pragma once
#include "Engine/Job/Jobs.hpp"
#include "Engine/Math/IntVec2.hpp"


class Map;
class PathCreation;
class Pather;

class PathJob : public Job
{

public:

	virtual ~PathJob();

	virtual void Execute() override;



public:

	Pather* m_pather;
	PathCreation* m_pathCreation = nullptr;
	IntVec2 m_startTile = IntVec2(0, 0);
	std::vector<IntVec2> m_endTiles;
	IntVec2 m_tileDimensions;


};