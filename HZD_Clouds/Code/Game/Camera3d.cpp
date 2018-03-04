#include "Game/Camera3d.hpp"

#include "Engine/engine.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

const float	CAMERA_NO_CLIP_FLY_SPEED = 10000.0f;
const float	MOUSE_SENSITIVITY_X = 0.05f;
const float	MOUSE_SENSITIVITY_Y = 0.05f;

Camera3d::Camera3d(const Vector3& initialPosition)
	:m_position(initialPosition)
	,m_yawDegreesAboutY(0.0f)
	,m_rollDegreesAboutZ(0.0f)
	,m_pitchDegreesAboutX(0.0f)
{
}

Vector3 Camera3d::CalcForwardXZ() const{
	return Vector3(-SinDegrees(m_yawDegreesAboutY), 0.0f, -CosDegrees(m_yawDegreesAboutY));
}

Vector3 Camera3d::CalcLeftXZ() const{
	Vector3 forward = CalcForwardXZ();
	return Vector3(forward.z, 0.f, -forward.x);
}

void Camera3d::Update(float deltaSeconds)
{
	float moveDistance = CAMERA_NO_CLIP_FLY_SPEED * deltaSeconds;

	if(g_theInputSystem->IsKeyDown(KEYCODE_SHIFT)){
		moveDistance *= 0.1f;
	}

	if(g_theInputSystem->WasKeyJustPressed('O')){
		m_position = Vector3::ZERO;
		m_rollDegreesAboutZ = 0.0f;
		m_pitchDegreesAboutX = 0.0f;
		m_yawDegreesAboutY = 0.0f;
	}

	// Keyboard position
	if(g_theInputSystem->IsKeyDown('W'))
		m_position += CalcForwardXZ() * -moveDistance;

	if(g_theInputSystem->IsKeyDown('S'))
		m_position += CalcForwardXZ() * moveDistance;

	if(g_theInputSystem->IsKeyDown('D'))
		m_position += CalcLeftXZ() * -moveDistance;

	if(g_theInputSystem->IsKeyDown('A'))
		m_position += CalcLeftXZ() * moveDistance;

	if(g_theInputSystem->IsKeyDown(KEYCODE_SPACE))
		m_position.y += moveDistance;

	if(g_theInputSystem->IsKeyDown('Z'))
		m_position.y -= moveDistance;

	// Mouse orientation
	IntVector2 mouseDeltaMove = g_theInputSystem->GetMouseMoveDelta();

	float mouseMoveDx = (float)mouseDeltaMove.x;
	m_yawDegreesAboutY += mouseMoveDx * MOUSE_SENSITIVITY_X;

	float mouseMoveDy = (float)mouseDeltaMove.y;
	m_pitchDegreesAboutX += mouseMoveDy * MOUSE_SENSITIVITY_Y;

	m_pitchDegreesAboutX = Clamp(m_pitchDegreesAboutX, -89.9f, 89.9f);
	KeepDegrees0To360(m_yawDegreesAboutY);

	Matrix4 x_rot = Matrix4::make_rotation_x_degrees(m_pitchDegreesAboutX);
	Matrix4 y_rot = Matrix4::make_rotation_y_degrees(m_yawDegreesAboutY);
	Matrix4 z_rot = Matrix4::make_rotation_z_degrees(m_rollDegreesAboutZ);

	m_rotation = x_rot * y_rot;
}

void Camera3d::LookAt(const Vector3& lookAtPosition, const Vector3& cameraUp){
	Vector3 k_basis = (m_position - lookAtPosition).Normalized();
	Vector3 i_basis = CrossProduct(cameraUp, k_basis);
	Vector3 j_basis = CrossProduct(k_basis, i_basis);

	// Set basis vectors of transform matrix
	m_rotation.set_i_basis(i_basis, 0.0f);
	m_rotation.set_j_basis(j_basis, 0.0f);
	m_rotation.set_k_basis(k_basis, 0.0f);
}

Matrix4 Camera3d::GetViewTransform() const{
	Matrix4 inverse_trans = Matrix4::make_translation(-m_position);
	return inverse_trans * m_rotation.transposed();
}

Matrix4 Camera3d::GetWorldTransform() const
{
    return m_rotation * Matrix4::make_translation(m_position);
}