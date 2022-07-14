#pragma once

#include <DirectXMath.h>

namespace dx = DirectX;

class Camera
{
public:
	Camera() noexcept;
	DirectX::XMMATRIX GetViewMatrix() const noexcept;
	void SpawnControlWindow() noexcept;
	void Reset() noexcept;
	void Rotate(float dx, float dy) noexcept;               // ȸ�� �Լ�
	void Translate(DirectX::XMFLOAT3 translation) noexcept; // �����̵� �Լ�
private:
	DirectX::XMFLOAT3 pos;
	float pitch; // x�� ȸ�� �� 
	float yaw;   // y�� ȸ�� ��
	static constexpr float travelSpeed = 12.0f;
	static constexpr float rotationSpeed = 0.004f;
};