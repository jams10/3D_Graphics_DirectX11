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
	void SpawnControlWindow(float index) noexcept;
	void SetPosition(float x, float y, float z);
	void SetColor(float r, float g, float b);
	void ResetPosition() noexcept;
	void ResetAmbient() noexcept;
	void ResetDiffuse() noexcept;
	void ResetSpecularPower() noexcept;
	void ResetSpecularColor () noexcept;
	void ResetAll() noexcept;
	void SetDirection(float x, float y, float z);
private:
	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT4 diffuseColor;
	DirectX::XMFLOAT3 lightDirection;
	float			  specularPower;
	DirectX::XMFLOAT4 specularColor;
	DirectX::XMFLOAT4 position;
};