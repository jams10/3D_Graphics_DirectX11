#include "Light.h"
#include <imgui/imgui.h>

Light::Light()
{
	ResetAll();
}

Light::~Light()
{
}

DirectX::XMFLOAT4 Light::GetAmbientColor() const noexcept
{
	return ambientColor;
}

DirectX::XMFLOAT4 Light::GetDiffuseColor() const noexcept
{
    return diffuseColor;
}

DirectX::XMFLOAT3 Light::GetLightDirection() const noexcept
{
    return lightDirection;
}

float Light::GetSpecularPower() const noexcept
{
	return specularPower;
}

DirectX::XMFLOAT4 Light::GetSpecularColor() const noexcept
{
	return specularColor;
}

void Light::SpawnControlWindow() noexcept
{
	if (ImGui::Begin("Light"))
	{
		ImGui::Text("Diffuse Color");
		ImGui::ColorPicker3("Diffuse Color", &diffuseColor.x);
		if (ImGui::Button("Reset Diffuse"))
		{
			ResetDiffuse();
		}
		ImGui::Text("Ambient Color");
		ImGui::ColorPicker3("Ambient Color", &ambientColor.x);
		if (ImGui::Button("Reset Ambient"))
		{
			ResetAmbient();
		}
		ImGui::Text("Specular Color");
		ImGui::ColorPicker3("Specular Color", &specularColor.x);
		if (ImGui::Button("Reset Specular Color"))
		{
			ResetSpecularColor();
		}
		ImGui::Text("Specular Power");
		ImGui::SliderFloat("Power", &specularPower, 16.0f, 64.0f, "%.1f");
		if (ImGui::Button("Reset Specular Power"))
		{
			ResetSpecularPower();
		}
	}
	ImGui::End();
}

void Light::ResetAmbient() noexcept
{
	ambientColor = { 0.2f, 0.2f, 0.2f, 1.0f };
}

void Light::ResetDiffuse() noexcept
{
	diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void Light::ResetSpecularPower() noexcept
{
	specularPower = 16.0f;
}

void Light::ResetSpecularColor() noexcept
{
	specularColor = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void Light::ResetAll() noexcept
{
	ResetAmbient();
	ResetDiffuse();
	ResetSpecularPower();
	ResetSpecularColor();
    lightDirection = { 0.0f, 0.0f, 1.0f };
}

void Light::SetDirection(float x, float y, float z)
{
	lightDirection = DirectX::XMFLOAT3(x, y, z);
}
