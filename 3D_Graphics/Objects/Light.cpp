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

DirectX::XMFLOAT3 Light::GetPosition3() const
{
	return DirectX::XMFLOAT3(position.x, position.y, position.z);
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
		ImGui::Text("Rotation");
		ImGui::SliderFloat("yaw", &yaw, -80.0f, 80.0f, "%.1f");
		ImGui::SliderFloat("pitch", &pitch, -80.0f, 80.0f, "%.1f");
		if (ImGui::Button("Reset Rotation"))
		{
			ResetRotation();
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

void Light::ResetRotation() noexcept
{
	yaw = 0.f;
	pitch = 0.f;
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
	yaw = 0.f;
	pitch = 0.f;
    lightDirection = { 0.0f, 0.0f, 1.0f };
}

void Light::SetDirection(float x, float y, float z)
{
	lightDirection = DirectX::XMFLOAT3(x, y, z);
}

void Light::SetLookAt(float x, float y, float z)
{
	m_lookAt.x = x;
	m_lookAt.y = y;
	m_lookAt.z = z;
}

void Light::GenerateViewMatrix()
{
	DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	DirectX::XMFLOAT3 lightPos = DirectX::XMFLOAT3(position.x, position.y, position.z);

	const auto vUp = DirectX::XMLoadFloat3(&up);
	const auto vLightPos = DirectX::XMLoadFloat3(&lightPos);
	const auto vLookAt = DirectX::XMLoadFloat3(&m_lookAt);

	m_viewMatrix = DirectX::XMMatrixLookAtLH(vLightPos, vLookAt, vUp);
}

void Light::GenerateProjectionMatrix(float screenDepth, float screenNear)
{
	float fieldOfView, screenAspect;
	fieldOfView = 3.141592654f / 2.0f;
	screenAspect = 1.f;

	// 투영 행렬을 생성.
	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);
}

void Light::GetViewMatrix(DirectX::XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}

void Light::GetProjectionMatrix(DirectX::XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
}
