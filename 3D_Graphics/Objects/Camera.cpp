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

	// ���� ������ ����Ű�� up ���͸� ����.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	upVector = XMLoadFloat3(&up); // XMFLOAT3 Ÿ���� XMVECTOR ����ü�� �ε���.

	// ���� ���������� ī�޶� ��ġ�� ��Ÿ���� ���͸� ����.
	position.x = m_locationX;
	position.y = m_locationY;
	position.z = m_locationZ;
	positionVector = XMLoadFloat3(&position);

	// ī�޶� �ٶ󺸴� ����� ��ġ�� ��Ÿ���� ���͸� ����.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;
	lookAtVector = XMLoadFloat3(&lookAt);

	// 60�й��� ���� ���� ���� ������ �ٲ���.
	pitch = m_rotationX * 0.0174532925f;
	yaw = m_rotationY * 0.0174532925f;
	roll = m_rotationZ * 0.0174532925f;

	// pitch, yaw, roll ���� ���� ȸ���ϴ� ȸ�� ����� ����.
	rotationMatrix = dx::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// ī�޶� �������� �ùٸ��� ȸ�� �ǵ��� ������ ������ ȸ�� ��ķ� lookAt �� up ���͸� ��ȯ��.
	lookAtVector = dx::XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = dx::XMVector3TransformCoord(upVector, rotationMatrix);

	// ī�޶� �ٶ󺸴� ����� �Ǵ� ���͸� ī�޶� ��ġ �������� �̵���.
	lookAtVector = dx::XMVectorAdd(positionVector, lookAtVector);

	// �� ���͸� ���� view ��ȯ ����� �������.
	m_viewMatrix = dx::XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}

dx::XMMATRIX Camera::GetViewMatrix()
{
	return m_viewMatrix;
}
