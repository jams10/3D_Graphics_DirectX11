#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Graphics/D2DGraphics.h>
#include <Graphics/DebugWindow.h>
#include <Graphics/RenderToTexture.h>
#include <Shaders/TextureShader.h>
#include <Shaders/PointLightShader.h>
#include <Shaders/LightShader.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <Objects/Model.h>
#include <Objects/Camera.h>
#include <Objects/Light.h>
#include <Objects/Bitmap.h>
#include <Objects/ModelList.h>
#include <Utils/Frustum.h>
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
    m_pD2D = nullptr;
    m_pCamera = nullptr;
    m_pModelGround = nullptr;
    m_pTextureShader = nullptr;
    m_pPointLightShader = nullptr;
    m_pLights = nullptr;
    m_pLightShader = nullptr;
    m_pBitmap = nullptr;
    m_pFrustum = nullptr;
    m_pModelList = nullptr;
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND Wnd)
{
    m_pD3D = new D3DGraphics();
    m_pD3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, Wnd, SCREEN_DEPTH, SCREEN_NEAR);

    m_pD2D = new D2DGraphics(*m_pD3D);
    m_pD2D->Initialize(*m_pD3D);

    m_pModelGround = new Model();
    m_pModelGround->Initialize(*m_pD3D, "Resources\\Models\\plane01.model", "Resources\\Images\\stone01.png", "Resources\\Images\\bump03.png", "Resources\\Images\\spec02.png");
    
    m_pCamera = new Camera();
    m_pFixedCamera = new Camera();

    m_pLights = new Light[4];

    m_pPointLightShader = new PointLightShader();
    m_pPointLightShader->Initialize(*m_pD3D);

    m_pLightShader = new LightShader();
    m_pLightShader->Initialize(*m_pD3D);

    m_pTextureShader = new TextureShader();
    m_pTextureShader->Initialize(*m_pD3D);

    m_pBitmap = new Bitmap();
    m_pBitmap->Initialize(*m_pD3D, screenWidth, screenHeight, screenWidth, screenHeight);

    m_pDebugWindow = new DebugWindow();
    m_pDebugWindow->Initialize(*m_pD3D, screenWidth, screenHeight, 100, 100);

    m_pModelList = new ModelList();
    m_pModelList->Initialize(50);

    m_pFrustum = new Frustum();

    m_pLights[0].SetColor(1.0f, 0.0f, 0.0f);
    m_pLights[0].SetPosition(-3.0f, 1.0f, 3.0f);
    m_pLights[1].SetColor(0.0f, 1.0f, 0.0f);
    m_pLights[1].SetPosition(3.0f, 1.0f, 3.0f);
    m_pLights[2].SetColor(0.0f, 0.0f, 1.0f);
    m_pLights[2].SetPosition(-3.0f, 1.0f, -3.0f);
    m_pLights[3].SetColor(1.0f, 1.0f, 1.0f);
    m_pLights[3].SetPosition(3.0f, 1.0f, -3.0f);

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pFrustum)
    SAFE_RELEASE(m_pModelList)
    SAFE_RELEASE(m_pDebugWindow)
    SAFE_RELEASE(m_pBitmap)
    SAFE_RELEASE(m_pTextureShader)
    SAFE_RELEASE(m_pPointLightShader)
    SAFE_RELEASE(m_pLightShader)
    if (m_pLights) delete[] m_pLights;
    SAFE_RELEASE(m_pCamera)
    SAFE_RELEASE(m_pModelGround)
    SAFE_RELEASE(m_pD2D)
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
    RenderScene(pSound);

    return true;
}

void Graphics::RenderScene(DXSound* pSound)
{
    m_pD3D->SetBackBufferRenderTarget();

    m_pD3D->BeginFrame(0.2f, 0.2f, 0.2f, 1.0f);
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();
    
    dx::XMFLOAT4 diffuseColors[4];
    dx::XMFLOAT4 lightPositions[4];

    for (int i = 0; i < 4; ++i)
    {
        diffuseColors[i] = m_pLights[i].GetDiffuseColor();
        lightPositions[i] = m_pLights[i].GetPosition();
    }

    m_pModelGround->Bind(*m_pD3D);

    m_pPointLightShader->Bind(*m_pD3D, m_pModelGround->GetIndexCount(), world, view, projection, m_pModelGround->GetTextureArray()[0],
        diffuseColors, lightPositions);

#pragma region UI
    m_pCamera->SpawnControlWindow();
    for (int i = 0; i < 4; ++i) m_pLights[i].SpawnControlWindow(i);
    pSound->SpawnControlWindow();
#pragma endregion

    m_pD3D->EndFrame();
    return;
}
