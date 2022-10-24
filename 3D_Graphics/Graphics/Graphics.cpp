#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Graphics/D2DGraphics.h>
#include <Graphics/DebugWindow.h>
#include <Shaders/TextureShader.h>
#include <Shaders/FireShader.h>
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
    m_pModel = nullptr;
    m_pTextureShader = nullptr;
    m_pFireShader = nullptr;
    m_pLight = nullptr;
    m_pLightShader = nullptr;
    m_pBitmap = nullptr;
    m_pFrustum = nullptr;
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

    m_pModel = new Model();
    m_pModel->Initialize(*m_pD3D, "Resources\\Models\\Square.model", "Resources\\Images\\fire01.png", "Resources\\Images\\noise01.png", "Resources\\Images\\alpha01.png");
    
    m_pCamera = new Camera();
    m_pFixedCamera = new Camera();

    m_pLight = new Light();

    m_pFireShader = new FireShader();
    m_pFireShader->Initialize(*m_pD3D);

    m_pLightShader = new LightShader();
    m_pLightShader->Initialize(*m_pD3D);

    m_pTextureShader = new TextureShader();
    m_pTextureShader->Initialize(*m_pD3D);

    m_pBitmap = new Bitmap();
    m_pBitmap->Initialize(*m_pD3D, screenWidth, screenHeight, screenWidth, screenHeight);

    m_pDebugWindow = new DebugWindow();
    m_pDebugWindow->Initialize(*m_pD3D, screenWidth, screenHeight, 100, 100);

    m_pFrustum = new Frustum();

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pFrustum)
    SAFE_RELEASE(m_pDebugWindow)
    SAFE_RELEASE(m_pBitmap)
    SAFE_RELEASE(m_pTextureShader)
    SAFE_RELEASE(m_pFireShader)
    SAFE_RELEASE(m_pLightShader)
    SAFE_RELEASE(m_pLight)
    SAFE_RELEASE(m_pCamera)
    SAFE_RELEASE(m_pModel)
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
    m_pD3D->BeginFrame(0.2f, 0.2f, 0.2f, 1.0f);
    dx::XMMATRIX world = m_pModel->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();

    static float frameTime = 0.0f;
    float distortionScale, distortionBias;
    XMFLOAT3 scrollSpeeds, scales;
    XMFLOAT2 distortion1, distortion2, distortion3;

    frameTime += dt;
    if (frameTime > 1000.f) frameTime = 0.0f;

    scrollSpeeds = XMFLOAT3(1.3f, 2.1f, 2.3f);
    scales = XMFLOAT3(1.0f, 2.0f, 3.0f);
    distortion1 = XMFLOAT2(0.1f, 0.2f);
    distortion2 = XMFLOAT2(0.1f, 0.3f);
    distortion3 = XMFLOAT2(0.1f, 0.2f);
    distortionScale = 0.8f;
    distortionBias = 0.5f;

    m_pD3D->TurnOnAlphaBlending();

    m_pModel->Bind(*m_pD3D);
    m_pFireShader->Bind(*m_pD3D, m_pModel->GetIndexCount(), world, view, projection,
        m_pModel->GetTextureArray()[0], m_pModel->GetTextureArray()[1], m_pModel->GetTextureArray()[2],
        frameTime, scrollSpeeds, scales, distortion1, distortion2, distortion3, distortionScale, distortionBias);

    m_pD3D->TurnOffAlphaBlending();

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
