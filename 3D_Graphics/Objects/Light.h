#pragma once

#include <DirectXMath.h>

class Light
{
public:
	Light();
	~Light();
	DirectX::XMFLOAT4 GetAmbientColor() const noexcept;
	DirectX::XMFLOAT4 GetDiffuseColor() const noexcept;
	DirectX::XMFLOAT3 GetLightDirection() const noexcept;
	float			  GetSpecularPower() const noexcept;
	DirectX::XMFLOAT4 GetSpecularColor() const noexcept;
	DirectX::XMFLOAT4 GetPosition() const;
	DirectX::XMFLOAT3 GetPosition3() const;
	void SpawnControlWindow(float index) noexcept;
	void SetPosition(float x, float y, float z);
	void SetColor(float r, float g, float b);
	void ResetPosition() noexcept;
	void ResetRotation() noexcept;
	void ResetAmbient() noexcept;
	void ResetDiffuse() noexcept;
	void ResetSpecularPower() noexcept;
	void ResetSpecularColor () noexcept;
	void ResetAll() noexcept;
	void SetDirection(float x, float y, float z);

	void SetLookAt(float x, float y, float z);

	void GenerateViewMatrix();
	void GenerateProjectionMatrix(float screenDepth, float screenNear);

	void GetViewMatrix(DirectX::XMMATRIX& viewMatrix);
	void GetProjectionMatrix(DirectX::XMMATRIX& projectionMatrix);

private:
	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT4 diffuseColor;
	DirectX::XMFLOAT3 lightDirection;
	float			  specularPower;
	DirectX::XMFLOAT4 specularColor;
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT3 m_lookAt;
	DirectX::XMMATRIX m_viewMatrix;
	DirectX::XMMATRIX m_projectionMatrix;

	float pitch;
	float yaw;
};