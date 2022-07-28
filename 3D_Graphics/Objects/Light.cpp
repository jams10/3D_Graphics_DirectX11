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

void Light::ResetAll() noexcept
{
	ResetAmbient();
	ResetDiffuse();
    lightDirection = { 0.0f, 0.0f, 1.0f };
}
