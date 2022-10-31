#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Shaders/DepthShader.h>
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
    m_pDepthShader = nullptr;
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

    m_pDepthShader = new DepthShader();
    m_pDepthShader->Initialize(*m_pD3D);

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pDepthShader)
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

    m_pFloorModel->Bind(*m_pD3D);
    m_pDepthShader->Bind(*m_pD3D, m_pFloorModel->GetIndexCount(), world, view, projection);
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
