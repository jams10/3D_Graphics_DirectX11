#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Graphics/D2DGraphics.h>
#include <Graphics/DebugWindow.h>
#include <Graphics/RenderToTexture.h>
#include <Shaders/TextureShader.h>
#include <Shaders/LightShader.h>
#include <Shaders/FadeShader.h>
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
    accumulatedTime = 0.f;
    blendAmount = 0.5f;
    m_pD3D = nullptr;
    m_pD2D = nullptr;
    m_pCamera = nullptr;
    m_pModelCube = nullptr;
    m_pModelFloor = nullptr;
    m_pLightShader = nullptr;
    m_pRenderToTexture = nullptr;
    m_pTextureShader = nullptr;
    m_pFadeShader = nullptr;
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

    m_pModelCube = new Model();
    m_pModelCube->Initialize(*m_pD3D, "Resources\\Models\\Cube.model", "Resources\\Images\\dirt01.png", "Resources\\Images\\bump03.png", "Resources\\Images\\spec02.png");
    
    m_pModelFloor = new Model();
    m_pModelFloor->Initialize(*m_pD3D, "Resources\\Models\\Floor.model", "Resources\\Images\\blue01.png", "Resources\\Images\\bump03.png", "Resources\\Images\\spec02.png");

    m_pCamera = new Camera();
    m_pFixedCamera = new Camera();

    m_pLight = new Light();

    m_pLightShader = new LightShader();
    m_pLightShader->Initialize(*m_pD3D);

    m_pTextureShader = new TextureShader();
    m_pTextureShader->Initialize(*m_pD3D);

    m_pFadeShader = new FadeShader();
    m_pFadeShader->Initialize(*m_pD3D);

    m_pBitmap = new Bitmap();
    m_pBitmap->Initialize(*m_pD3D, screenWidth, screenHeight, screenWidth, screenHeight);

    m_pRenderToTexture = new RenderToTexture();
    m_pRenderToTexture->Initialize(*m_pD3D, screenWidth, screenHeight);

    m_pDebugWindow = new DebugWindow();
    m_pDebugWindow->Initialize(*m_pD3D, screenWidth, screenHeight, 100, 100);

    m_pModelList = new ModelList();
    m_pModelList->Initialize(50);

    m_pFrustum = new Frustum();

    m_fadeInTime = 10.f;   // fade되는 시간을 10 초로 초기화.
    m_accumulatedTime = 0; // 누적 시간을 0초로 초기화.
    m_fadePercentage = 0;  // fade 퍼센티지를 0으로 만들어 씬을 완전히 검은색으로 만들어줌.
    m_fadeDone = false;

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pFrustum)
    SAFE_RELEASE(m_pModelList)
    SAFE_RELEASE(m_pDebugWindow)
    SAFE_RELEASE(m_pRenderToTexture)
    SAFE_RELEASE(m_pBitmap)
    SAFE_RELEASE(m_pFadeShader)
    SAFE_RELEASE(m_pTextureShader)
    SAFE_RELEASE(m_pLightShader)
    SAFE_RELEASE(m_pLight)
    SAFE_RELEASE(m_pCamera)
    SAFE_RELEASE(m_pModelFloor)
    SAFE_RELEASE(m_pModelCube)
    SAFE_RELEASE(m_pD2D)
    SAFE_RELEASE(m_pD3D)
}

bool Graphics::Frame(DXSound* pSound, int fps, int cpuUsage, float dt)
{
    if (!m_fadeDone)
    {
        m_accumulatedTime += dt;
        std::cout << m_accumulatedTime << '\n';
        if (m_accumulatedTime < m_fadeInTime)
        {
            m_fadePercentage = m_accumulatedTime / m_fadeInTime;
        }
        else
        {
            m_fadeDone = true;
            m_fadePercentage = 1.0f;
        }
    }

    if (!Render(pSound, fps, cpuUsage, dt))
    {
        return false;
    }

    return true;
}

bool Graphics::Render(DXSound* pSound, int fps, int cpuUsage, float dt)
{
    static float rotYaw = 0.f;
    rotYaw += PI * 0.005f;

    if (rotYaw > 360.0f)
    {
        rotYaw -= 360.0f;
    }

    if (m_fadeDone)
    {
        // fade 효과 시간이 끝나면 일반 씬을 그려줌.
        RenderScene(rotYaw);
    }
    else
    {
        RenderToTextureFunc(rotYaw);
        RenderFadingScene();
    }
    return true;
}

void Graphics::RenderToTextureFunc(float yaw)
{
    dx::XMMATRIX world = m_pModelCube->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();

    // 렌더 타겟을 텍스쳐로 설정하고 초기화 해줌.
    m_pRenderToTexture->SetRenderTarget(*m_pD3D);
    m_pRenderToTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);

    m_pModelCube->SetYaw(yaw);
    m_pModelCube->Bind(*m_pD3D);
    m_pTextureShader->Bind(*m_pD3D, m_pModelCube->GetIndexCount(), world, view, projection, m_pModelCube->GetTextureArray()[0]);

    m_pD3D->SetBackBufferRenderTarget();

    return;
}

void Graphics::RenderFadingScene()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetOrthMatrix();

    m_pD3D->BeginFrame(0.0f, 0.0f, 0.0f, 1.0f);
    m_pD3D->TurnZBufferOff();

    m_pBitmap->Bind(*m_pD3D, 0, 0);
    m_pFadeShader->Bind(*m_pD3D, m_pBitmap->GetIndexCount(), world, view, projection, m_pRenderToTexture->GetShaderResourceView(), m_fadePercentage);

    m_pD3D->TurnZBufferOn();
    m_pD3D->EndFrame();
}

void Graphics::RenderScene(float yaw)
{
    dx::XMMATRIX world = m_pModelCube->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();
    dx::XMMATRIX reflection = m_pCamera->GetReflectionMatrix(-1.5f);

    m_pD3D->BeginFrame(0.0f, 0.0f, 0.0f, 1.0f);
    
    m_pModelCube->SetYaw(yaw);
    m_pModelCube->Bind(*m_pD3D);
    m_pTextureShader->Bind(*m_pD3D, m_pModelCube->GetIndexCount(), world, view, projection, m_pModelCube->GetTextureArray()[0]);

#pragma region UI
    m_pModelCube->SpawnControlWindow();
    m_pCamera->SpawnControlWindow();
    m_pLight->SpawnControlWindow();
#pragma endregion

    m_pD3D->EndFrame();
    return;
}
