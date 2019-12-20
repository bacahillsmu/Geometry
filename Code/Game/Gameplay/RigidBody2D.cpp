#include "Game/Gameplay/RigidBody2D.hpp"
#include "Game/Gameplay/PhysicsSystem.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Gameplay/Collider2D.hpp"

RigidBody2D::RigidBody2D( float mass, float restitution )
	:m_mass(mass)
	,m_restitution(restitution)
{
}

RigidBody2D::~RigidBody2D()
{
	m_physicsSystem->UnregisterRigidBodyFromPhysicsSystem(this); 
	m_physicsSystem->RemoveCollider(m_colliderAABB2);
	m_physicsSystem->RemoveCollider(m_colliderRing);
}

// Public ---------------------------------------------------------------------
void RigidBody2D::Move( float deltaSeconds )
{
	Vec2 acceleration = g_thePhysicsSystem->GetGravity() * m_gravityScale;
	m_velocity += acceleration * deltaSeconds;
	m_transform.m_position += m_velocity * deltaSeconds;
	m_collider->translate(m_velocity * deltaSeconds);
}

void RigidBody2D::MoveBy( Vec2 amountToMoveBy )
{
	m_transform.m_position += amountToMoveBy;
	m_collider->translate(amountToMoveBy);
}

void RigidBody2D::SetSimulationType( PhysicsSimulationType physicsSimulationType )
{
	m_physicsSimulationType = physicsSimulationType;
}

void RigidBody2D::SetGravityScale( Vec2 gravityScale )
{
	m_gravityScale = gravityScale;
}

Collider2D* RigidBody2D::SetAndReturnCollider( ColliderAABB2* collider )
{
	m_collider = dynamic_cast<Collider2D*>(collider);
	return m_collider;
}

Collider2D* RigidBody2D::SetAndReturnCollider( ColliderRing* collider )
{
	m_collider = dynamic_cast<Collider2D*>(collider);
	return m_collider;
}

// ColliderAABB2* RigidBody2D::SetAndReturnCollider( ColliderAABB2* collider )
// {
// 	m_colliderAABB2 = collider;
// 	return m_colliderAABB2;
// }
// 
// ColliderRing* RigidBody2D::SetAndReturnCollider( ColliderRing* collider )
// {
// 	m_colliderRing = collider;
// 	return m_colliderRing;
// }

// Private --------------------------------------------------------------------

// ----------------------------------------------------------------------------
