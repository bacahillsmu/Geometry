#include "Game/Gameplay/Camera/OrbitCamera.hpp"


// -----------------------------------------------------------------------
OrbitCamera::OrbitCamera()
{

}

// -----------------------------------------------------------------------
OrbitCamera::~OrbitCamera()
{

}

// -----------------------------------------------------------------------
void OrbitCamera::Update()
{
	float currentAngle = m_defaultAngle + m_angleOffset;
	float h = SinDegrees(m_defaultTilt) * m_distance;
	float xyDistance = CosDegrees(m_defaultTilt) * m_distance;

	m_defaultTilt = RangeMap(m_distance, m_maxDistance, m_minDistance, m_tiltBounds.y, m_tiltBounds.x);

	Vec3 offset = Vec3(CosDegrees(currentAngle), SinDegrees(currentAngle), 0.0f) * xyDistance + Vec3(0.0f, 0.0f, -h);

	Vec3 cameraPosition = m_focalPoint + offset;

	Matrix4x4 cameraModel = Matrix4x4::LookAt(cameraPosition, m_focalPoint, Vec3(0.0f, 0.0f, -1.0f));
	SetModelMatrix( cameraModel );
}

// -----------------------------------------------------------------------
void OrbitCamera::SetFocalPoint( const Vec3& position )
{
	m_focalPoint = position;
}

// -----------------------------------------------------------------------
void OrbitCamera::ZoomByDelta( float deltaZoom )
{
	m_distance -= deltaZoom;

	m_distance = m_distance < m_minDistance ? m_minDistance : m_distance;
	m_distance = m_distance > m_maxDistance ? m_maxDistance : m_distance;
}

// -----------------------------------------------------------------------
void OrbitCamera::SetDistance( float distance )
{
	m_distance = distance;

	m_distance = m_distance < m_minDistance ? m_minDistance : m_distance;
	m_distance = m_distance > m_maxDistance ? m_maxDistance : m_distance;
}

// -----------------------------------------------------------------------
void OrbitCamera::SetAngle( float angleDegrees )
{
	UNUSED(angleDegrees);
}

// -----------------------------------------------------------------------
void OrbitCamera::SetRestingRotation( float angleDegrees )
{
	m_defaultAngle = angleDegrees;
}

// -----------------------------------------------------------------------
void OrbitCamera::SetRotationOffset( float angleDegrees )
{
	m_angleOffset = angleDegrees;
}

// -----------------------------------------------------------------------
void OrbitCamera::UpdateCameraPan( float panAmount, float deltaSeconds )
{
	if(m_angleOffset > 0.0f && panAmount == 0)
	{
		m_angleOffset += -0.5f * deltaSeconds * 60.0f;
		if(m_angleOffset > -0.4 && m_angleOffset < 0.4)
		{
			m_angleOffset = 0.0f;
		}
	}

	if(m_angleOffset < 0.0f && panAmount == 0)
	{
		m_angleOffset += 0.5f * deltaSeconds * 60.0f;
		if(m_angleOffset > -0.4 && m_angleOffset < 0.4)
		{
			m_angleOffset = 0.0f;
		}
	}

	m_angleOffset += panAmount * deltaSeconds;

	m_angleOffset = m_angleOffset < m_minAngleOffset ? m_minAngleOffset : m_angleOffset;
	m_angleOffset = m_angleOffset > m_maxAngleOffset ? m_maxAngleOffset : m_angleOffset;
}
