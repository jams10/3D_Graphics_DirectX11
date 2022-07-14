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
	void Rotate(float dx, float dy) noexcept;               // 회전 함수
	void Translate(DirectX::XMFLOAT3 translation) noexcept; // 평행이동 함수
private:
	DirectX::XMFLOAT3 pos;
	float pitch; // x축 회전 값 
	float yaw;   // y축 회전 값
	static constexpr float travelSpeed = 12.0f;
	static constexpr float rotationSpeed = 0.004f;
};