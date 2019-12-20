#pragma once
#include "Game/Gameplay/Collider2D.hpp"

#include <vector>


class RigidBody2D;
class Collider2D;
class ColliderAABB2;
class ColliderRing;


class PhysicsSystem
{
	friend class RigidBody2D;

public:
	
	void BeginFrame();
	void PreRender();
	void Update(float deltaSeconds);
	void RunStep(float deltaSeconds);
	void MoveAllDynamicObjects(float deltaSeconds);

	// Collision Checks
	void ResolveDynamicVsStaticCollisions(bool resolutionCheck);
	void ResolveDynamicVsDynamicCollisions(bool resolutionCheck);


	// RigidBody
	RigidBody2D* CreateRigidBody(float mass, float restitution);
	void DestroyRigidBody(RigidBody2D* rigidBody);
	void UnregisterRigidBodyFromPhysicsSystem(RigidBody2D* rigidBody);

	// Collider
	void AddCollider(Collider2D* collider);
	void RemoveCollider(Collider2D* collider);

	// Get Manifolds
	bool GetManifold(Manifold2D* outManifold2D, AABB2* object1, AABB2* object2);
	bool GetManifold(Manifold2D* outManifold2D, AABB2* object1, Ring* object2);
	bool GetManifold(Manifold2D* outManifold2D, Ring* object1, Ring* object2);
	
	// Collision
	bool GetCollision2D(Collision2D& collision, Collider2D* object1, Collider2D* object2);
	

	// Gets
	Vec2 GetGravity() { return m_gravity; }

	// Sets
	void SetGravity(Vec2 gravity) { m_gravity = gravity; }

	// Store each RigidBody in the game
	std::vector<RigidBody2D*> m_rigidBodies;

	// Store each Collider in the game
	std::vector<Collider2D*> m_colliders;

	// Put the RigidBodyBucket here and store the above two

	// system info, like gravity
	Vec2 m_gravity = Vec2(0.0f, 0.0f);
};




