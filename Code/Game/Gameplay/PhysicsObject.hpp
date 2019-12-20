#include "Game/Gameplay/RigidBody2D.hpp"
#include "Game/Gameplay/Collider2D.hpp"
#include "Engine/Core/Rgba.hpp"

class PhysicsObject
{

public:
	PhysicsObject(){}
	~PhysicsObject();

	PhysicsObject(Vec2 position, 
		ColliderType colliderType, 
		PhysicsSimulationType physicsSimulationType,
		Vec2 offset,
		float mass,
		float restitution);

	bool IsPhysicsObjectOutOfBounds();

	Transform2D m_transform;
	Rgba m_renderColor = Rgba::GREEN;
	RigidBody2D* m_rigidBody = nullptr;
	Collider2D* m_collider = nullptr;
	ColliderType m_colliderType;
	PhysicsSimulationType m_physicsSimulationType;
	bool m_isSelected = false;

};
