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
	void SpawnControlWindow() noexcept;
	void ResetAmbient() noexcept;
	void ResetDiffuse() noexcept;
	void ResetAll() noexcept;
private:
	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT4 diffuseColor;
	DirectX::XMFLOAT3 lightDirection;

};