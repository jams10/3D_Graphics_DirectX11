#include "Light.h"
#include <imgui/imgui.h>

Light::Light()
{
    Reset();
}

Light::~Light()
{
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
		if (ImGui::Button("Reset"))
		{
			Reset();
		}
	}
	ImGui::End();
}

void Light::Reset() noexcept
{
    diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    lightDirection = { 0.0f, 0.0f, 1.0f };
}
