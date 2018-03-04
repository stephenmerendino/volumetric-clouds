#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix4.hpp"

class Camera3d{
public:
	Vector3			m_position;
	Matrix4			m_rotation;

public:
	float			m_yawDegreesAboutY;
	float			m_rollDegreesAboutZ;
	float			m_pitchDegreesAboutX;

public:
	Camera3d(const Vector3& initialPosition = Vector3::ZERO);

	void Update(float deltaSeconds);
	void LookAt(const Vector3& lookAtPosition, const Vector3& cameraUp = Vector3::Y_AXIS);

	Matrix4 GetViewTransform() const;
	Matrix4 GetWorldTransform() const;

	Vector3 CalcForwardXZ() const;
	Vector3 CalcLeftXZ() const;
};