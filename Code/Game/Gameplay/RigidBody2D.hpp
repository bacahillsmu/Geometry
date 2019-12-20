#include "Engine/Math/Vec2.hpp"

// Forward Declarations ---------------------------------------------------------------------------
class PhysicsSystem;
class Collider2D;
class ColliderAABB2;
class ColliderRing;

struct Transform2D
{
	Vec2 m_position = Vec2(0.0f, 0.0f);
	Vec2 m_rotation = Vec2(0.0f, 0.0f);
	Vec2 m_scale    = Vec2(0.0f, 0.0f);
};

enum PhysicsSimulationType
{
	PHYSICS_SIMULATION_UNKNOWN = -1,

	PHYSICS_SIMULATION_TYPE_STATIC,
	PHYSICS_SIMULATION_TYPE_DYNAMIC,

	PHYSICS_SIMULATION_TYPE_COUNT
};

class RigidBody2D
{
	friend class PhysicsSystem;

public:
	RigidBody2D(float mass, float restitution);
	~RigidBody2D();

	void Move(float deltaSeconds);
	void MoveBy(Vec2 amountToMoveBy);


	// Possibly some form of debug rendering

	// Sets

	void SetSimulationType(PhysicsSimulationType physicsSimulationType);
	void SetGravityScale(Vec2 gravityScale);

// 	ColliderAABB2* SetAndReturnCollider(ColliderAABB2* collider);
// 	ColliderRing* SetAndReturnCollider(ColliderRing* collider);

	Collider2D* SetAndReturnCollider(ColliderAABB2* collider);
	Collider2D* SetAndReturnCollider(ColliderRing* collider);

private:



public:
	PhysicsSystem* m_physicsSystem = nullptr;
	ColliderAABB2* m_colliderAABB2 = nullptr;
	ColliderRing* m_colliderRing = nullptr;
	Collider2D* m_collider = nullptr;
	float m_restitution = 0.0f;
	PhysicsSimulationType m_physicsSimulationType;
	Transform2D m_transform;
	Transform2D* m_objectTransform = nullptr;
	float m_mass = 0.0f;
	Vec2 m_gravityScale = Vec2(0.0f, 0.0f);

	Vec2 m_velocity = Vec2(0.0f, 0.0f);
	
};