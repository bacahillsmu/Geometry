#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Line.hpp"

#include <vector>


class ConvexPoly2D
{

public:

	ConvexPoly2D();
	~ConvexPoly2D();

	void Render();
	void SpecialRender();
	bool IsPointInside(Vec2 point);

public:

	float m_radius = 0.0f;
	int m_numVerts = 0;
	std::vector<Vec2> m_verts;
	std::vector<Line> m_edges;

};