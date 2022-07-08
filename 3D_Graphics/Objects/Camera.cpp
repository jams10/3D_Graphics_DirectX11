#include "Camera.h"

Camera::Camera()
{
	m_locationX = 0.0f;
	m_locationY = 0.0f;
	m_locationZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;

	m_viewMatrix = dx::XMMatrixIdentity();
}

Camera::~Camera()
{
}

void Camera::SetLocation(float x, float y, float z)
{
	m_locationX = x;
	m_locationY = y;
	m_locationZ = z;
	return;
}

void Camera::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}

dx::XMFLOAT3 Camera::GetLocation()
{
	return dx::XMFLOAT3(m_locationX, m_locationY, m_locationZ);
}

dx::XMFLOAT3 Camera::GetRotation()
{
	return dx::XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}

void Camera::Update()
{
	dx::XMFLOAT3 up, position, lookAt;
	dx::XMVECTOR upVector, positionVector, lookAtVector;
	float yaw, pitch, roll;
	dx::XMMATRIX rotationMatrix;

	// 위쪽 방향을 가리키는 up 벡터를 설정.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	upVector = XMLoadFloat3(&up); // XMFLOAT3 타입을 XMVECTOR 구조체에 로드함.

	// 월드 공간에서의 카메라 위치를 나타내는 벡터를 설정.
	position.x = m_locationX;
	position.y = m_locationY;
	position.z = m_locationZ;
	positionVector = XMLoadFloat3(&position);

	// 카메라가 바라보는 대상의 위치를 나타내는 벡터를 설정.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;
	lookAtVector = XMLoadFloat3(&lookAt);

	// 60분법의 각도 값을 라디안 값으로 바꿔줌.
	pitch = m_rotationX * 0.0174532925f;
	yaw = m_rotationY * 0.0174532925f;
	roll = m_rotationZ * 0.0174532925f;

	// pitch, yaw, roll 값을 따라 회전하는 회전 행렬을 생성.
	rotationMatrix = dx::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// 카메라가 원점에서 올바르게 회전 되도록 위에서 구해준 회전 행렬로 lookAt 및 up 벡터를 변환함.
	lookAtVector = dx::XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = dx::XMVector3TransformCoord(upVector, rotationMatrix);

	// 카메라가 바라보는 대상이 되는 벡터를 카메라 위치 기준으로 이동함.
	lookAtVector = dx::XMVectorAdd(positionVector, lookAtVector);

	// 세 벡터를 통해 view 변환 행렬을 만들어줌.
	m_viewMatrix = dx::XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}

dx::XMMATRIX Camera::GetViewMatrix()
{
	return m_viewMatrix;
}
