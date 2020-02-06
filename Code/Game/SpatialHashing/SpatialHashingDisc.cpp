#include "Game/SpatialHashing/SpatialHashingDisc.hpp"

//-------------------------------------------------------------------
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/Ray.hpp"

#include "Game/Shapes/ConvexPoly.hpp"
#include "Game/Gameplay/Map.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Gameplay/Game.hpp"


//-------------------------------------------------------------------
SpatialHashingDisc::SpatialHashingDisc()
{

}

//-------------------------------------------------------------------
SpatialHashingDisc::~SpatialHashingDisc()
{
	if(m_leftChild)
	{
		delete m_leftChild;
	}
	
	if(m_rightChild)
	{
		delete m_rightChild;
	}
}

void SpatialHashingDisc::Update()
{
	m_top = m_center + (m_up * m_radius);
	m_bottom = m_center + (m_down * m_radius);
}

//-------------------------------------------------------------------
void SpatialHashingDisc::Render(std::vector<Vertex_PCU>* verts)
{
	AddVertsForRing2D(*verts, m_center, m_radius, 0.5f, Rgba::YELLOW);

	if(m_leftChild)
	{
		m_leftChild->Render(verts);
	}

	if(m_rightChild)
	{
		m_rightChild->Render(verts);
	}
}

//-------------------------------------------------------------------
void SpatialHashingDisc::ShrinkToFit()
{
	float farthestDistance = 0.0f;
	Vec2 farthestVert;

	for(int i = 0; i < m_convexPoly2Ds.size(); ++i)
	{
		for(int j = 0; j < m_convexPoly2Ds[i]->m_verts.size(); ++j)
		{
			float distance = GetDistance(m_center, m_convexPoly2Ds[i]->m_verts[j]);
			if(distance > farthestDistance)
			{
				farthestDistance = distance;
				farthestVert = m_convexPoly2Ds[i]->m_verts[j];
			}
		}
	}

	if(farthestDistance > 0.0f)
	{
		m_radius = farthestDistance;
	}
}

//-------------------------------------------------------------------
void SpatialHashingDisc::GenerateChildren()
{
	Line verticalSplit = Line(m_top, m_bottom, 1.0f);

	std::vector<ConvexPoly2D*> bucket1;
	std::vector<ConvexPoly2D*> bucket2;

	// Divide our world into two sections. Currently, its left and right sides;

	// Go through all shapes we have, if any point is on left side, its whole self goes into left bucket.
	for(int i = 0; i < m_convexPoly2Ds.size(); ++i)
	{
		for(int j = 0; j < m_convexPoly2Ds[i]->m_verts.size(); ++j)
		{
			if (DotProductVec2((m_convexPoly2Ds[i]->m_verts[j] - m_center), verticalSplit.m_surfaceNormal) > 0.0f)
			{
				bucket1.push_back(m_convexPoly2Ds[i]);
			}
			else
			{
				bucket2.push_back(m_convexPoly2Ds[i]);
			}

			break;
		}
	}

	// If one bucket has no members then we will stop, it will just be a full other-bucket in the previous iteration;
	if(bucket1.size() == 0 || bucket2.size() == 0)
	{
		return;
	}
	else
	{
		m_convexPoly2Ds.clear();
	}

	// If we have any left side convex polys, then make a disc and repeat.
	if(bucket1.size() > 0)
	{
		Vec2 averageCenterOfAllConvexPolys = Vec2(0.0f, 0.0f);
		for(int i = 0; i < bucket1.size(); ++i)
		{
			Vec2 convexPolyAverageCenter = Vec2(0.0f, 0.0f);
			for(int j = 0; j < bucket1[i]->m_verts.size(); ++j)
			{
				convexPolyAverageCenter += bucket1[i]->m_verts[j];
			}
			convexPolyAverageCenter /= (float)bucket1[i]->m_verts.size();
			averageCenterOfAllConvexPolys += convexPolyAverageCenter;
		}
		averageCenterOfAllConvexPolys /= (float)bucket1.size();

		SpatialHashingDisc* left = new SpatialHashingDisc();
		left->SetCenter(averageCenterOfAllConvexPolys);
		left->SetRadius(Map::WIDTH);
		for (int i = 0; i < bucket1.size(); ++i)
		{
			left->m_convexPoly2Ds.push_back(bucket1[i]);
		}
		left->ShrinkToFit();
		left->m_top = left->m_center + (left->m_up * left->m_radius);
		left->m_bottom = left->m_center + (left->m_down * left->m_radius);
		m_leftChild = left;
		m_leftChild->m_parent = this;

		if(bucket1.size() > 1)
		{
			m_leftChild->GenerateChildren();
		}
		
	}

	if (bucket2.size() > 0)
	{
		Vec2 averageCenterOfAllConvexPolys = Vec2(0.0f, 0.0f);
		for (int i = 0; i < bucket2.size(); ++i)
		{
			Vec2 convexPolyAverageCenter = Vec2(0.0f, 0.0f);
			for (int j = 0; j < bucket2[i]->m_verts.size(); ++j)
			{
				convexPolyAverageCenter += bucket2[i]->m_verts[j];
			}
			convexPolyAverageCenter /= (float)bucket2[i]->m_verts.size();
			averageCenterOfAllConvexPolys += convexPolyAverageCenter;
		}
		averageCenterOfAllConvexPolys /= (float)bucket2.size();

		SpatialHashingDisc* right = new SpatialHashingDisc();
		right->SetCenter(averageCenterOfAllConvexPolys);
		right->SetRadius(Map::WIDTH);
		for (int i = 0; i < bucket2.size(); ++i)
		{
			right->m_convexPoly2Ds.push_back(bucket2[i]);
		}
		right->ShrinkToFit();
		right->m_top = right->m_center + (right->m_up * right->m_radius);
		right->m_bottom = right->m_center + (right->m_down * right->m_radius);
		m_rightChild = right;
		m_rightChild->m_parent = this;

		if (bucket2.size() > 1)
		{
			m_rightChild->GenerateChildren();
		}
	}
}

//-------------------------------------------------------------------
SpatialHashingDisc* SpatialHashingDisc::GetParent()
{
	return m_parent;
}

//-------------------------------------------------------------------
SpatialHashingDisc* SpatialHashingDisc::GetLeftChild()
{
	return m_leftChild;
}

//-------------------------------------------------------------------
SpatialHashingDisc* SpatialHashingDisc::GetRightChild()
{
	return m_rightChild;
}

//-------------------------------------------------------------------
Vec2 SpatialHashingDisc::GetCenter()
{
	return m_center;
}

//-------------------------------------------------------------------
float SpatialHashingDisc::GetRadius()
{
	return m_radius;
}

//-------------------------------------------------------------------
void SpatialHashingDisc::AddToRadius(float radiusAddition)
{
	m_radius += radiusAddition;
}

//-------------------------------------------------------------------
void SpatialHashingDisc::SetRadius(float radius)
{
	m_radius = radius;
}

//-------------------------------------------------------------------
void SpatialHashingDisc::SetCenter(Vec2 center)
{
	m_center = center;
}

//-------------------------------------------------------------------
void SpatialHashingDisc::RaycastAgainstMyselfAndChildren(std::vector<ConvexPoly2D*>* outPolys, const Vec2& rayStart, const Vec2& rayEnd)
{
	float time[2];
	Vec2 rayDirection = (rayEnd - rayStart);
	rayDirection.Normalize();
	Disc disc = Disc(m_center, m_radius);
	uint solutions = Raycast(time, rayDirection, rayStart, disc);

	// If we hit this then we need to see if we have children;
	// If we have children then we need to raycast against them.
	// If we fail a raycast against both our children then we don't do anything
	// If we raycast against a child and it has polys, add those polys to outpolys

	if(solutions == 1)
	{
		if(time[0] > 0.0f)
		{
			for (int i = 0; i < m_convexPoly2Ds.size(); ++i)
			{
				outPolys->push_back(m_convexPoly2Ds[i]);
			}

			if (m_leftChild)
			{
				m_leftChild->RaycastAgainstMyselfAndChildren(outPolys, rayStart, rayEnd);
			}
			if (m_rightChild)
			{
				m_rightChild->RaycastAgainstMyselfAndChildren(outPolys, rayStart, rayEnd);
			}
		}
	}
	else if(solutions == 2)
	{
		if (time[0] > 0.0f)
		{
			for (int i = 0; i < m_convexPoly2Ds.size(); ++i)
			{
				outPolys->push_back(m_convexPoly2Ds[i]);
			}

			if (m_leftChild)
			{
				m_leftChild->RaycastAgainstMyselfAndChildren(outPolys, rayStart, rayEnd);
			}
			if (m_rightChild)
			{
				m_rightChild->RaycastAgainstMyselfAndChildren(outPolys, rayStart, rayEnd);
			}
		}
		if(time[1] > 0.0f)
		{
			for (int i = 0; i < m_convexPoly2Ds.size(); ++i)
			{
				outPolys->push_back(m_convexPoly2Ds[i]);
			}

			if (m_leftChild)
			{
				m_leftChild->RaycastAgainstMyselfAndChildren(outPolys, rayStart, rayEnd);
			}
			if (m_rightChild)
			{
				m_rightChild->RaycastAgainstMyselfAndChildren(outPolys, rayStart, rayEnd);
			}
		}
	}
}
