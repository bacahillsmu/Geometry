#pragma once
#include "Engine/Math/Vec2.hpp"



#define UNUSED(x) (void)(x);

// Forward Declarations ---------------------------------------------------------------------------
struct AABB2;
struct Ring;
class Collider2D;
class ColliderAABB2;
class ColliderRing;

// ------------------------------------------------------------------------------------------------
enum ColliderType
{
	COLLIDER_TYPE_AABB2,
	COLLIDER_TYPE_RING,

	COLLIDER_TYPE_COUNTS
};

// ------------------------------------------------------------------------------------------------
struct Manifold2D
{
	Vec2 normal;
	float penetrationAmount;
};

// ------------------------------------------------------------------------------------------------
struct Collision2D
{
	Collider2D* me;
	Collider2D* them;
	Manifold2D manifold;
};


class Collider2D
{

public:
	Collider2D();
	~Collider2D();

	bool IsTouching(Collider2D* otherCollider);
	virtual void translate(const Vec2& amount) = 0;

	bool m_isTouching = false;
	ColliderType m_colliderType;
	Collider2D* m_collisionInformation = nullptr;
};

class ColliderAABB2 : public Collider2D
{
public:
	explicit ColliderAABB2( const Vec2& initialCenter, const Vec2& initialOffset );
	~ColliderAABB2();

	void UpdateShape();
	bool IsTouching(ColliderAABB2* otherCollider); // AABB2 vs. AABB2
	bool IsTouching(ColliderRing* otherCollider);  // AABB2 vs. Ring
	void translate(const Vec2& amount);

	AABB2* m_shape = nullptr;

};

class ColliderRing : public Collider2D
{
public:
	explicit ColliderRing( Vec2& initialRingCenter, float initialRingRadius, float initialRingThickness );
	~ColliderRing();

	bool IsTouching(ColliderAABB2* otherCollider); // Ring vs. AABB2
	bool IsTouching(ColliderRing* otherCollider);  // Ring vs. Ring
	void translate(const Vec2& amount);

	Ring* m_shape = nullptr;

};
