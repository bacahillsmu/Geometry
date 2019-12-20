#include "Game/Framework/GameCommon.hpp"
#include "Game/Gameplay/PhysicsObject.hpp"
#include "Game/Gameplay/PhysicsSystem.hpp"
#include "Engine/Core/RandomNumberGenerator.hpp"


PhysicsObject::PhysicsObject( Vec2 position, ColliderType colliderType, PhysicsSimulationType physicsSimulationType, Vec2 offset, float mass, float restitution )
{
	// We need to set the transform
	m_transform.m_position = position;
	m_transform.m_rotation = Vec2(0.0f, 0.0f);
	m_transform.m_scale = Vec2(0.0f, 0.0f);
	m_colliderType = colliderType;
	m_physicsSimulationType = physicsSimulationType;

	// Give this physics object a rigidbody to represent itself
	m_rigidBody = g_thePhysicsSystem->CreateRigidBody(mass, restitution);
	m_rigidBody->SetGravityScale(Vec2(1.0f, 1.0f));
	m_rigidBody->SetSimulationType(physicsSimulationType);
	m_rigidBody->m_transform = m_transform;
	m_rigidBody->m_objectTransform = &m_transform;
	m_rigidBody->m_physicsSystem = g_thePhysicsSystem;

	// Give it a shape
	if(colliderType == COLLIDER_TYPE_AABB2)
	{
		//Vec2 offset(g_theRandomNumberGenerator->GetRandomFloatInRange(3.0f, 6.0f), g_theRandomNumberGenerator->GetRandomFloatInRange(3.0f, 6.0f));
		
		//m_colliderAABB2 = m_rigidBody->SetAndReturnCollider(new ColliderAABB2(position, offset));
		//g_thePhysicsSystem->AddCollider(m_colliderAABB2);
		m_collider = m_rigidBody->SetAndReturnCollider(new ColliderAABB2(position, offset));
		g_thePhysicsSystem->AddCollider(m_collider);
	}
	else if(colliderType == COLLIDER_TYPE_RING)
	{
		float ringRadius = g_theRandomNumberGenerator->GetRandomFloatInRange(3.0f, 6.0f);
		float ringThiccness = g_theRandomNumberGenerator->GetRandomFloatInRange(0.2f, 1.2f);

		//m_colliderRing = m_rigidBody->SetAndReturnCollider(new ColliderRing(position, ringRadius, ringThiccness));
		//g_thePhysicsSystem->AddCollider(m_colliderRing);
		m_collider = m_rigidBody->SetAndReturnCollider(new ColliderRing(position, ringRadius, ringThiccness));
		g_thePhysicsSystem->AddCollider(m_collider);
	}
}

PhysicsObject::~PhysicsObject()
{
	delete m_rigidBody;
	m_rigidBody = nullptr;
}

bool PhysicsObject::IsPhysicsObjectOutOfBounds()
{
	Vec2 minBounds = Vec2(0.0f, 0.0f);
	Vec2 maxBounds = Vec2(WORLD_WIDTH, WORLD_HEIGHT);

	if(m_transform.m_position.x < minBounds.x || 
		m_transform.m_position.x > maxBounds.x || 
		m_transform.m_position.y < minBounds.y || 
		m_transform.m_position.y > maxBounds.y)
	{
		return true;
	}

	return false;
}
