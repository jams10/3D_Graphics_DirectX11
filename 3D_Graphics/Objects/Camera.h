#pragma once

#include <DirectXMath.h>

namespace dx = DirectX;

class Camera
{
public:
	Camera();
	~Camera();

	void SetLocation(float x, float y, float z);
	void SetRotation(float x, float y, float z);

	dx::XMFLOAT3 GetLocation();
	dx::XMFLOAT3 GetRotation();

	void Update();
	dx::XMMATRIX GetViewMatrix();

private:
	float m_locationX, m_locationY, m_locationZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	dx::XMMATRIX m_viewMatrix;
};