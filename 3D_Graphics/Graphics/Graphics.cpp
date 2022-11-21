#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Shaders/TSColorShader.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <Objects/TS_TestModel.h>
#include <Objects/Camera.h>
#include <Objects/Light.h>
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
    m_pModel = nullptr;
    m_pColorShader = nullptr;
    tessellationAmount = 12.0f;
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND Wnd)
{
    m_pD3D = new D3DGraphics();
    m_pD3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, Wnd, SCREEN_DEPTH, SCREEN_NEAR);

    m_pModel = new TS_TestModel();
    m_pModel->Initialize(*m_pD3D);
    
    m_pCamera = new Camera();

    m_pColorShader = new TSColorShader();
    m_pColorShader->Initialize(*m_pD3D);


    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pColorShader)
    SAFE_RELEASE(m_pModel)
    SAFE_RELEASE(m_pCamera)
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
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX proj = m_pD3D->GetProjectionMatrix();

    m_pD3D->BeginFrame(0.2f, 0.2f, 0.2f, 1.0f);

    m_pModel->Bind(*m_pD3D);

    m_pColorShader->Bind(*m_pD3D, m_pModel->GetIndexCount(), world, view, proj, tessellationAmount);

#pragma region UI
    //m_pCamera->SpawnControlWindow();
    if (ImGui::Begin("Tessellation"))
    {
        ImGui::Text("Amount");
        ImGui::SliderFloat("X", &tessellationAmount, 6.0f, 20.0f, "%.1f");
    }
    ImGui::End();
#pragma endregion

    m_pD3D->EndFrame();
    return true;
}



