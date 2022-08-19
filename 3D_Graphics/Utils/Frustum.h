#pragma once

#include <DirectXMath.h>
#include <vector>

class Frustum
{
public:
	Frustum();

	void ConstructFrustum(float screenDepth, DirectX::XMMATRIX& projMatrix, DirectX::XMMATRIX& viewMatrix);

	float GetDotResult(DirectX::XMFLOAT4 plane, DirectX::XMVECTOR v);
	bool CheckPoint(float x, float y , float z);
	bool CheckCube(float xCenter, float yCenter, float zCenter, float radius);
	bool CheckSphere(float xCenter, float yCenter, float zCenter, float radius);
	bool CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize);

private:
	std::vector<DirectX::XMFLOAT4> m_frustumPlanes;
};

