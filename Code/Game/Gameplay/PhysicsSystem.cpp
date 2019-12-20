#include "Game/Framework/GameCommon.hpp"
#include "Game/Gameplay/PhysicsSystem.hpp"
#include "Game/Gameplay/RigidBody2D.hpp"
#include "Engine/Math/Ring.hpp"
#include "Engine/Math/AABB2.hpp"

#include <cmath>

// ----------------------------------------------------------------------------
PhysicsSystem* g_thePhysicsSystem = nullptr;


// Processes ------------------------------------------------------------------
void PhysicsSystem::BeginFrame()
{
	// Clean collisions by saying none are touching so we get a fresh check on touching
	for(int i = 0; i < m_colliders.size(); i ++)
	{
		m_colliders[i]->m_isTouching = false;
	}

	for (RigidBody2D* rb : m_rigidBodies)
	{
		rb->m_transform = *rb->m_objectTransform;
	}
}

void PhysicsSystem::PreRender()
{
	// figure out movement and apply to actual game object
	for (RigidBody2D* rb : m_rigidBodies)
	{
		*rb->m_objectTransform = rb->m_transform;
	}
}

void PhysicsSystem::Update(float deltaSeconds)
{
	BeginFrame();

	RunStep(deltaSeconds);

	PreRender();
}

// Public ---------------------------------------------------------------------
RigidBody2D* PhysicsSystem::CreateRigidBody(float mass, float restitution)
{
	RigidBody2D* rigidBody = new RigidBody2D(mass, restitution);
	m_rigidBodies.push_back(rigidBody);
	return rigidBody;
}

void PhysicsSystem::DestroyRigidBody( RigidBody2D* rigidBody )
{
	UNUSED(rigidBody);
}

void PhysicsSystem::UnregisterRigidBodyFromPhysicsSystem( RigidBody2D* rigidBody )
{
	for(int rigidBodyIndex = 0; rigidBodyIndex < m_rigidBodies.size(); rigidBodyIndex++)
	{
		if(m_rigidBodies[rigidBodyIndex] == rigidBody)
		{
			m_rigidBodies.erase(m_rigidBodies.begin() + rigidBodyIndex);
		}
	}
}

void PhysicsSystem::AddCollider(Collider2D* collider)
{
	m_colliders.push_back(collider);
}

void PhysicsSystem::RemoveCollider(Collider2D* collider)
{
	for(int colliderIndex = 0; colliderIndex < m_colliders.size(); colliderIndex++)
	{
		if(m_colliders[colliderIndex] == collider)
		{
			m_colliders.erase(m_colliders.begin() + colliderIndex);
		}
	}
}

bool PhysicsSystem::GetCollision2D( Collision2D& collision, Collider2D* object1, Collider2D* object2 )
{
	bool isColliding = false;

	collision.me = object1;
	collision.them = object2;

	if(object1->m_colliderType == COLLIDER_TYPE_AABB2)
	{
		ColliderAABB2* meColliderAABB2 = static_cast<ColliderAABB2*>(object1);

		if(object2->m_colliderType == COLLIDER_TYPE_AABB2)
		{
			ColliderAABB2* themColliderAABB2 = static_cast<ColliderAABB2*>(object2);
			isColliding = GetManifold(&collision.manifold, meColliderAABB2->m_shape, themColliderAABB2->m_shape);

		}
		else if(object2->m_colliderType == COLLIDER_TYPE_RING)
		{
			ColliderRing* themColliderRing = static_cast<ColliderRing*>(object2);
			isColliding = GetManifold(&collision.manifold, meColliderAABB2->m_shape, themColliderRing->m_shape);
			collision.manifold.normal *= -1;
		}
	}
	else if(object1->m_colliderType == COLLIDER_TYPE_RING)
	{
		ColliderRing* meColliderRing = static_cast<ColliderRing*>(object1);

		if(object2->m_colliderType == COLLIDER_TYPE_AABB2)
		{
			ColliderAABB2* themColliderAABB2 = static_cast<ColliderAABB2*>(object2);
			isColliding = GetManifold(&collision.manifold, themColliderAABB2->m_shape, meColliderRing->m_shape);			
		}
		else if(object2->m_colliderType == COLLIDER_TYPE_RING)
		{
			ColliderRing* themColliderRing = static_cast<ColliderRing*>(object2);
			isColliding = GetManifold(&collision.manifold, meColliderRing->m_shape, themColliderRing->m_shape);
		}
	}

	return isColliding;
}

void PhysicsSystem::RunStep( float deltaSeconds )
{
	
	MoveAllDynamicObjects(deltaSeconds);

	ResolveDynamicVsStaticCollisions(true);  // Collision & Resolution Check
	ResolveDynamicVsDynamicCollisions(true); // Collision & Resolution Check
	ResolveDynamicVsStaticCollisions(false); // JUST Collision Check

	for(int colliderIndex = 0; colliderIndex < m_colliders.size(); colliderIndex++)
	{
		for(int otherColliderIndex = 0; otherColliderIndex < m_colliders.size(); otherColliderIndex++)
		{
			if(colliderIndex != otherColliderIndex)
			{
				Collision2D collision2D;
				bool isTouching = GetCollision2D(collision2D, m_colliders[colliderIndex], m_colliders[otherColliderIndex]);
				if(isTouching)
				{
					m_colliders[colliderIndex]->m_isTouching = isTouching;
					m_colliders[otherColliderIndex]->m_isTouching = isTouching;
				}				
			}
		}
	}
}

void PhysicsSystem::MoveAllDynamicObjects( float deltaSeconds )
{
	// Check each RigidBody
	for(int rigidBodyIndex = 0; rigidBodyIndex < m_rigidBodies.size(); rigidBodyIndex++)
	{
		// Only apply physics to Dynamic objects
		if(m_rigidBodies[rigidBodyIndex]->m_physicsSimulationType == PHYSICS_SIMULATION_TYPE_DYNAMIC)
		{
			m_rigidBodies[rigidBodyIndex]->Move(deltaSeconds);
		}
	}
}

void PhysicsSystem::ResolveDynamicVsStaticCollisions( bool resolutionCheck )
{
	// Collect all DYNAMIC vs. STATIC Colliders
	std::vector<RigidBody2D*> dynamicRigidBodies;
	std::vector<RigidBody2D*> staticRigidBodies;
	for(int rigidbodyIndex = 0; rigidbodyIndex < m_rigidBodies.size(); rigidbodyIndex++)
	{
		switch(m_rigidBodies[rigidbodyIndex]->m_physicsSimulationType)
		{
		case PHYSICS_SIMULATION_TYPE_DYNAMIC:
			dynamicRigidBodies.push_back(m_rigidBodies[rigidbodyIndex]);
			break;

		case PHYSICS_SIMULATION_TYPE_STATIC:
			staticRigidBodies.push_back(m_rigidBodies[rigidbodyIndex]);
			break;
		};
	}

	// Run through each dynamic collider, checking to see if it collided with a static collider
	if(dynamicRigidBodies.size() && staticRigidBodies.size())
	{
		for(int dynamicIndex = 0; dynamicIndex < dynamicRigidBodies.size(); dynamicIndex++)
		{
			for(int staticIndex = 0; staticIndex < staticRigidBodies.size(); staticIndex++)
			{
				Collision2D collision2D;
				bool isColliding = GetCollision2D(collision2D, dynamicRigidBodies[dynamicIndex]->m_collider, staticRigidBodies[staticIndex]->m_collider);
				
				RigidBody2D* rigidBody = dynamicRigidBodies[dynamicIndex];
				RigidBody2D* staticBody = staticRigidBodies[staticIndex];

				if(isColliding)
				{
					rigidBody->MoveBy(collision2D.manifold.normal * collision2D.manifold.penetrationAmount);

					if(resolutionCheck)
					{
						Vec2 projectedVelocity = ProjectedVector(rigidBody->m_velocity, collision2D.manifold.normal);
						float movingAwayValue = DotProductVec2(collision2D.manifold.normal, rigidBody->m_velocity);

						if(movingAwayValue < 0)
						{
							Vec2 projectedVelocityTangent = rigidBody->m_velocity - projectedVelocity;
							rigidBody->m_velocity = projectedVelocityTangent - (projectedVelocity * (rigidBody->m_restitution * staticBody->m_restitution));
						}						
					}
				}
			}
		}
	}
}

void PhysicsSystem::ResolveDynamicVsDynamicCollisions( bool resolutionCheck )
{
	// Collect all DYNAMIC vs. STATIC Colliders
	std::vector<RigidBody2D*> dynamicRigidBodies;
	for(int rigidbodyIndex = 0; rigidbodyIndex < m_rigidBodies.size(); rigidbodyIndex++)
	{
		switch(m_rigidBodies[rigidbodyIndex]->m_physicsSimulationType)
		{
			case PHYSICS_SIMULATION_TYPE_DYNAMIC:
			dynamicRigidBodies.push_back(m_rigidBodies[rigidbodyIndex]);
			break;
		};
	}

	// Run through each dynamic collider, checking to see if it collided with a static collider
	if(dynamicRigidBodies.size())
	{
		for(int dynamicIndex = 0; dynamicIndex < dynamicRigidBodies.size(); dynamicIndex++)
		{
			for(int dynamicIndex2 = 0; dynamicIndex2 < dynamicRigidBodies.size(); dynamicIndex2++)
			{
				if(dynamicIndex != dynamicIndex2)
				{
					Collision2D collision2D;
					bool isColliding = GetCollision2D(collision2D, dynamicRigidBodies[dynamicIndex]->m_collider, dynamicRigidBodies[dynamicIndex2]->m_collider);
					
					RigidBody2D* rigidBody = dynamicRigidBodies[dynamicIndex];
					RigidBody2D* rigidBody2 = dynamicRigidBodies[dynamicIndex2];

					if(isColliding)
					{						
						float totalMass = rigidBody->m_mass + rigidBody2->m_mass;
						
						rigidBody->MoveBy(collision2D.manifold.normal * collision2D.manifold.penetrationAmount * (rigidBody2->m_mass / totalMass) );
						rigidBody2->MoveBy((collision2D.manifold.normal * -1.0f) * collision2D.manifold.penetrationAmount * (rigidBody->m_mass / totalMass) );

						if(resolutionCheck)
						{
							float movingAwayValue = DotProductVec2(collision2D.manifold.normal, rigidBody->m_velocity);
							if(movingAwayValue < 0)
							{
								float middle1 = DotProductVec2(rigidBody->m_velocity, collision2D.manifold.normal);
								float middle2 = DotProductVec2(rigidBody2->m_velocity, collision2D.manifold.normal);

								float restitution = rigidBody->m_restitution * rigidBody2->m_restitution;

								float final1 = (restitution * rigidBody2->m_mass * (middle2 - middle1) + rigidBody->m_mass * middle1 + rigidBody2->m_mass * middle2) / totalMass;
								float final2 = (restitution * rigidBody->m_mass * (middle1 - middle2) + rigidBody->m_mass * middle1 + rigidBody2->m_mass * middle2) / totalMass;

								rigidBody->m_velocity = (final1 * collision2D.manifold.normal) + (rigidBody->m_velocity - ProjectedVector(rigidBody->m_velocity, collision2D.manifold.normal));
								rigidBody2->m_velocity = (final2 * collision2D.manifold.normal) + (rigidBody2->m_velocity - ProjectedVector(rigidBody2->m_velocity, collision2D.manifold.normal));
							}
						}
					}
				}				
			}
		}
	}
}

bool PhysicsSystem::GetManifold( Manifold2D* outManifold2D, AABB2* object1, AABB2* object2 )
{
	UNUSED(outManifold2D);
	bool isColliding = false;

	Vec2 min = object1->mins.GetMax(object2->mins);
	Vec2 max = object1->maxs.GetMin(object2->maxs);

	Vec2 difference = max - min;

	if(difference.x > 0 && difference.y > 0)
	{
		isColliding = true;

		if(difference.x < difference.y)
		{
			outManifold2D->penetrationAmount = difference.x;
			outManifold2D->normal = Vec2(std::copysign(1.0f, object1->mins.x - object2->mins.x), 0.0f);			
		}
		else
		{
			outManifold2D->penetrationAmount = difference.y;
			outManifold2D->normal = Vec2(0.0f, std::copysign(1.0f, object1->mins.y - object2->mins.y));
		}
	}

	return isColliding;
}

bool PhysicsSystem::GetManifold( Manifold2D* outManifold2D, AABB2* object1, Ring* object2 )
{
	bool isColliding = false;

	Vec2 closestPoint = GetClosestPointOnAABB2(object2->ringCenter, object1);

	if(closestPoint != object2->ringCenter)
	{
		isColliding = IsPointInDisc(closestPoint, object2->ringCenter, object2->ringRadius);
		if(isColliding)
		{
			outManifold2D->penetrationAmount = GetOverLapDistance(closestPoint, object2->ringCenter, object2->ringRadius);
			outManifold2D->normal = (object2->ringCenter - closestPoint).GetNormalized();
		}
	}
	else
	{
		closestPoint = GetClosestPointOnAABB2Edge(object2->ringCenter, object1);
		isColliding = IsPointInDisc(closestPoint, object2->ringCenter, object2->ringRadius);
		if(isColliding)
		{
			outManifold2D->penetrationAmount = GetOverLapDistance(closestPoint, object2->ringCenter, object2->ringRadius);
			outManifold2D->normal = (object2->ringCenter - closestPoint).GetNormalized();
		}
	}	

	return isColliding;
}

bool PhysicsSystem::GetManifold( Manifold2D* outManifold2D, Ring* object1, Ring* object2 )
{
	bool isColliding = false;

	isColliding = DoDiscsOverlap(object1->ringCenter, object1->ringRadius, object2->ringCenter, object2->ringRadius);
	if(isColliding)
	{
		outManifold2D->penetrationAmount = GetDiscOverlapAmount(object1->ringCenter, object1->ringRadius, object2->ringCenter, object2->ringRadius);
		outManifold2D->normal = (object1->ringCenter - object2->ringCenter).GetNormalized();
	}

	return isColliding;
}
