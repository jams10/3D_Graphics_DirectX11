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

DirectX::XMFLOAT4 Light::GetPosition() const
{
	return position;
}

void Light::SpawnControlWindow(float index) noexcept
{
	if (ImGui::Begin("Light"))
	{
		ImGui::PushID(index);
		ImGui::Text("Diffuse Color");
		ImGui::ColorPicker3("Diffuse Color", &diffuseColor.x);
		if (ImGui::Button("Reset Diffuse"))
		{
			ResetDiffuse();
		}
		ImGui::Text("Position");
		ImGui::SliderFloat("X", &position.x, -80.0f, 80.0f, "%.1f");
		ImGui::SliderFloat("Y", &position.y, -80.0f, 80.0f, "%.1f");
		ImGui::SliderFloat("Z", &position.z, -80.0f, 80.0f, "%.1f");
		if (ImGui::Button("Reset Position"))
		{
			ResetPosition();
		}
		ImGui::PopID();
	}
	ImGui::End();
}

void Light::SetPosition(float x, float y, float z)
{
	position = DirectX::XMFLOAT4(x, y, z, 1.0f);
}

void Light::SetColor(float r, float g, float b)
{
	diffuseColor = DirectX::XMFLOAT4(r, g, b, 1.0f);
}

void Light::ResetPosition() noexcept
{
	position = { 0.0f, 1.0f, 0.0f, 1.0f };
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
	ResetPosition();
    lightDirection = { 0.0f, 0.0f, 1.0f };
}

void Light::SetDirection(float x, float y, float z)
{
	lightDirection = DirectX::XMFLOAT3(x, y, z);
}
