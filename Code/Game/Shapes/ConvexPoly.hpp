#pragma once

#include "Engine/Math/Vec2.hpp"

#include <vector>

class ConvexPoly2D
{

public:

	ConvexPoly2D();
	~ConvexPoly2D();


public:

	int m_numPoints = 0;
	std::vector<Vec2> m_verts;



};