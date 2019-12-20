#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Vec3.hpp"


class OrbitCamera : public Camera
{

public:

	OrbitCamera();
	~OrbitCamera();

	// Called each frame to update the underlying camera with the RTSCamera's options; 
	void Update();

	void SetFocalPoint( const Vec3& position );
	void ZoomByDelta( float deltaZoom );
	void SetDistance( float distance );
	void SetAngle( float angleDegrees );

	void SetRestingRotation( float angleDegrees );
	void SetRotationOffset( float angleDegrees );
	void UpdateCameraPan( float panAmount, float deltaSeconds );

public:

	Vec3 m_focalPoint = Vec3( -1.0f, -1.0f, -1.0f );

	float m_distance = 13.0f;
	float m_minDistance = 1.0f;
	float m_maxDistance = 13.0f;

	float m_defaultAngle = -135.0f;
	float m_defaultTilt = 40.0f;
	Vec2 m_tiltBounds = Vec2(10.0f, 40.0f);

	float m_angleOffset = 0.0f;
	float m_minAngleOffset = -55.0f;
	float m_maxAngleOffset = 55.0f;


};