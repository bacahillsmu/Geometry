#pragma once

#include "Engine/Math/Vec2.hpp"

#include <vector>

class ConvexPoly2D
{

public:

	ConvexPoly2D();
	~ConvexPoly2D();

	void Render();

public:

	int m_numVerts = 0;
	Vec2 m_center = Vec2(0.0f, 0.0f);
	std::vector<Vec2> m_verts;



};