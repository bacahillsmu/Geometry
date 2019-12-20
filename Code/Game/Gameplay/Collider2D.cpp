#include "Game/Gameplay/Collider2D.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Ring.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Gameplay/PhysicsSystem.hpp"
#include "Game/Framework/GameCommon.hpp"

#include <algorithm>

Collider2D::Collider2D()
{

}

Collider2D::~Collider2D()
{
	for(int i = 0; i < g_thePhysicsSystem->m_colliders.size(); i++)
	{
		if(g_thePhysicsSystem->m_colliders[i] == this)
		{
			g_thePhysicsSystem->m_colliders.erase(g_thePhysicsSystem->m_colliders.begin() + i);
		}
	}
}

ColliderAABB2::ColliderAABB2( const Vec2& center, const Vec2& offset )
{
	m_shape = new AABB2(center, offset);
	m_colliderType = COLLIDER_TYPE_AABB2;
}

ColliderAABB2::~ColliderAABB2()
{
	
}

void ColliderAABB2::UpdateShape()
{
	m_shape->mins.x = m_shape->center.x - m_shape->offset.x;
	m_shape->mins.y = m_shape->center.y - m_shape->offset.y;
	m_shape->maxs.x = m_shape->center.x + m_shape->offset.x;
	m_shape->maxs.y = m_shape->center.y + m_shape->offset.y;
}

bool Collider2D::IsTouching( Collider2D* otherCollider )
{
	bool isTouching = false;

	if(m_colliderType == COLLIDER_TYPE_AABB2)
	{
		ColliderAABB2* thisCollider = static_cast<ColliderAABB2*>(this);
		if(otherCollider->m_colliderType == COLLIDER_TYPE_AABB2)
		{
			isTouching = thisCollider->IsTouching(static_cast<ColliderAABB2*>(otherCollider));
		}
		else if(otherCollider->m_colliderType == COLLIDER_TYPE_RING)
		{
			isTouching = thisCollider->IsTouching(static_cast<ColliderRing*>(otherCollider));
		}
	}
	else if(m_colliderType == COLLIDER_TYPE_RING)
	{
		ColliderRing* thisCollider = static_cast<ColliderRing*>(this);
		if(otherCollider->m_colliderType == COLLIDER_TYPE_AABB2)
		{
			isTouching = thisCollider->IsTouching(static_cast<ColliderAABB2*>(otherCollider));
		}
		else if(otherCollider->m_colliderType == COLLIDER_TYPE_RING)
		{
			isTouching = thisCollider->IsTouching(static_cast<ColliderRing*>(otherCollider));
		}
	}

	return isTouching;
}

bool ColliderAABB2::IsTouching( ColliderAABB2* otherCollider )
{
	AABB2* thisAABB2 = m_shape;
	AABB2* otherAABB2 = otherCollider->m_shape;

	Vec2 min = thisAABB2->mins.GetMax(otherAABB2->mins);
	Vec2 max = thisAABB2->maxs.GetMin(otherAABB2->maxs);

	if(min.x < max.x && min.y < max.y)
	{
		return true;
	}
	else 
	{
		return false;
	}

}

bool ColliderAABB2::IsTouching( ColliderRing* otherCollider )
{
	AABB2* thisAABB2 = m_shape;
	Ring* otherRing = otherCollider->m_shape;

	float closestX = Clamp(otherRing->ringCenter.x, thisAABB2->mins.x, thisAABB2->maxs.x);
	float closestY = Clamp(otherRing->ringCenter.y, thisAABB2->mins.y, thisAABB2->maxs.y);
	Vec2 closestPoint = Vec2(closestX, closestY);

	float distanceSquared = GetDistanceSquared(otherRing->ringCenter, closestPoint);
	float radiusSquared = otherRing->ringRadius * otherRing->ringRadius;

	if(distanceSquared <= radiusSquared)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ColliderAABB2::translate( const Vec2& amount )
{
	m_shape->center += amount; 
	UpdateShape();
}

ColliderRing::ColliderRing( Vec2& initialRingCenter, float initialRingRadius, float initialRingThickness )
{
	m_shape = new Ring(initialRingCenter, initialRingRadius, initialRingThickness);
	m_colliderType = COLLIDER_TYPE_RING;
}

ColliderRing::~ColliderRing()
{
	
}

bool ColliderRing::IsTouching( ColliderAABB2* otherCollider )
{
	Ring* thisRing = m_shape;
	AABB2* otherAABB2 = otherCollider->m_shape;

	float closestX = Clamp(thisRing->ringCenter.x, otherAABB2->mins.x, otherAABB2->maxs.x);
	float closestY = Clamp(thisRing->ringCenter.y, otherAABB2->mins.y, otherAABB2->maxs.y);
	Vec2 closestPoint = Vec2(closestX, closestY);

	float distanceSquared = GetDistanceSquared(thisRing->ringCenter, closestPoint);
	float radiusSquared = thisRing->ringRadius * thisRing->ringRadius;

	if(distanceSquared <= radiusSquared)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ColliderRing::IsTouching( ColliderRing* otherCollider )
{
	Ring* thisRing = m_shape;
	Ring* otherRing = otherCollider->m_shape;

	float distanceBetweenCenters = GetDistance( thisRing->ringCenter, otherRing->ringCenter );

	if(distanceBetweenCenters <= (thisRing->ringRadius + otherRing->ringRadius))
	{
		return true;
	}
	else
	{
		return false;
	}	
}

void ColliderRing::translate( const Vec2& amount )
{
	m_shape->ringCenter += amount;
}
