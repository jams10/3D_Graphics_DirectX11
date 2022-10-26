#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Shaders/TextureShader.h>
#include <Shaders/LightShader.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <Objects/Model.h>
#include <Objects/Camera.h>
#include <Objects/Light.h>
#include <Objects/ModelList.h>
#include <Input/Keyboard.h>
#include <Sound/DXSound.h>
#include <imgui/imgui.h>
#include <stdexcept>
#include <sstream>

#include <iostream>

#define SAFE_RELEASE(p) if(p){delete p; p=nullptr;}
#define PI 3.141592

Graphics::Graphics()
{
    m_pD3D = nullptr;
    m_pCamera = nullptr;
    m_pTextureShader = nullptr;
    m_pLight = nullptr;
    m_pLightShader = nullptr;
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND Wnd)
{
    m_pD3D = new D3DGraphics();
    m_pD3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, Wnd, SCREEN_DEPTH, SCREEN_NEAR);


    m_pFloorModel = new Model();
    m_pFloorModel->Initialize(*m_pD3D, "Resources\\Models\\Floor.model", "Resources\\Images\\grid01.png", "Resources\\Images\\noise01.png", "Resources\\Images\\alpha01.png");
    
    m_pBillboardModel = new Model();
    m_pBillboardModel->Initialize(*m_pD3D, "Resources\\Models\\Square.model", "Resources\\Images\\seafloor.png", "Resources\\Images\\noise01.png", "Resources\\Images\\alpha01.png");

    m_pCamera = new Camera();

    m_pLight = new Light();

    m_pLightShader = new LightShader();
    m_pLightShader->Initialize(*m_pD3D);

    m_pTextureShader = new TextureShader();
    m_pTextureShader->Initialize(*m_pD3D);

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pTextureShader)
    SAFE_RELEASE(m_pLightShader)
    SAFE_RELEASE(m_pLight)
    SAFE_RELEASE(m_pCamera)
    SAFE_RELEASE(m_pFloorModel)
    SAFE_RELEASE(m_pBillboardModel)
    SAFE_RELEASE(m_pD3D)
}

bool Graphics::Frame(DXSound* pSound, int fps, int cpuUsage, float dt)
{
    if (!Render(pSound, fps, cpuUsage, dt))
    {
        return false;
    }

    return true;
}

bool Graphics::Render(DXSound* pSound, int fps, int cpuUsage, float dt)
{
    m_pD3D->BeginFrame(0.2f, 0.2f, 0.2f, 1.0f);
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();
    dx::XMMATRIX translate;
    dx::XMFLOAT3 camPos, modelPos;
    float angle, rotation;

    m_pFloorModel->Bind(*m_pD3D);
    m_pTextureShader->Bind(*m_pD3D, m_pFloorModel->GetIndexCount(), world, view, projection, m_pFloorModel->GetTextureArray()[0]);

    camPos = m_pCamera->GetPosition(); // ī�޶� ��ġ
    modelPos.x = 0.0f;                 // �� ��ġ
    modelPos.y = 1.5f;
    modelPos.z = 0.0f;

    // atan2 �Լ��� ���� ī�޶� ��ġ ���� ���� ȸ�� ������ ����.
    angle = atan2(modelPos.x - camPos.x, modelPos.z - camPos.z) * (180.0 / 3.141592f);
    // dx �Լ��� ����ϱ� ���� ���� ������ 60�й� �������� ���� ������ ��������.
    rotation = (float)angle * 0.0174532925f;
    // ȸ�� ���, �����̵� ����� ���� ������ ������ �𵨿� ����.
    world = dx::XMMatrixRotationY(rotation);
    translate = dx::XMMatrixTranslation(modelPos.x, modelPos.y, modelPos.z);
    world = dx::XMMatrixMultiply(world, translate);

    m_pBillboardModel->Bind(*m_pD3D);
    m_pTextureShader->Bind(*m_pD3D, m_pBillboardModel->GetIndexCount(), world, view, projection, m_pBillboardModel->GetTextureArray()[0]);

#pragma region UI
    //m_pCamera->SpawnControlWindow();
    //m_pModel->SpawnControlWindow();
#pragma endregion

    m_pD3D->EndFrame();

    return true;
}

void Graphics::RenderScene()
{

}
