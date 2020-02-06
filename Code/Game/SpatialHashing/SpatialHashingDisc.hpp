#pragma once

#include "Engine/Math/Vec2.hpp"

#include <vector>

class ConvexPoly2D;

class SpatialHashingDisc
{

public:

	SpatialHashingDisc();
	~SpatialHashingDisc();

	void Update();
	void Render(std::vector<Vertex_PCU>* verts);
	void ShrinkToFit();
	void GenerateChildren();

	SpatialHashingDisc* GetParent();
	SpatialHashingDisc* GetLeftChild();
	SpatialHashingDisc* GetRightChild();

	Vec2 GetCenter();
	float GetRadius();

	void AddToRadius(float radiusAddition);
	void SetRadius(float radius);
	void SetCenter(Vec2 center);

	// Raycasting Against Me and Children;
	void RaycastAgainstMyselfAndChildren(std::vector<ConvexPoly2D*>* outPolys, const Vec2& rayStart, const Vec2& rayEnd);


public:

	Vec2 m_center;
	float m_radius;
	Vec2 m_up = Vec2(0.0f, 1.0f);
	Vec2 m_down = Vec2(0.0f, -1.0f);
	Vec2 m_top;
	Vec2 m_bottom;

	SpatialHashingDisc* m_parent = nullptr;
	SpatialHashingDisc* m_leftChild = nullptr;
	SpatialHashingDisc* m_rightChild = nullptr;

	std::vector<ConvexPoly2D*> m_convexPoly2Ds;

};