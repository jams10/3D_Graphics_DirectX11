#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Graphics/RenderToTexture.h>
#include <Shaders/ShadowShader.h>
#include <Shaders/DepthShader.h>
#include <Shaders/LightShader.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <Objects/Camera.h>
#include <Objects/Model.h>
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
    m_pCubeModel = nullptr;
    m_pGroundModel = nullptr;
    m_pSphereModel = nullptr;
    m_pLight = nullptr;
    m_pRenderTexture = nullptr;
    m_pDepthShader = nullptr;
    m_pShadowShader = nullptr;
    m_pLightShader = nullptr;
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND Wnd)
{
    m_pD3D = new D3DGraphics();
    m_pD3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, Wnd, SCREEN_DEPTH, SCREEN_NEAR);
    
    m_pCamera = new Camera();

    m_pCubeModel = new Model();
    m_pCubeModel->Initialize(*m_pD3D, "Resources\\Models\\Cube.model", "Resources\\Images\\wall01.png", "Resources\\Images\\noise01.png", "Resources\\Images\\alpha01.png");
    m_pCubeModel->SetPosition(-2.0f, 2.0f, 0.0f);

    m_pSphereModel = new Model();
    m_pSphereModel->Initialize(*m_pD3D, "Resources\\Models\\Sphere.model", "Resources\\Images\\ice01.png", "Resources\\Images\\noise01.png", "Resources\\Images\\alpha01.png");
    m_pSphereModel->SetPosition(2.0f, 2.0f, 0.0f);

    m_pGroundModel = new Model();
    m_pGroundModel->Initialize(*m_pD3D, "Resources\\Models\\plane01.model", "Resources\\Images\\metal001.png", "Resources\\Images\\noise01.png", "Resources\\Images\\alpha01.png");
    m_pGroundModel->SetPosition(0.0f, 1.0f, 0.0f);

    m_pLight = new Light();
    m_pLight->SetLookAt(0.0f, 0.0f, 0.f);
    m_pLight->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);

    m_pRenderTexture = new RenderToTexture();
    m_pRenderTexture->Initialize(*m_pD3D, 1280, 720, SCREEN_DEPTH, SCREEN_NEAR);

    m_pDepthShader = new DepthShader();
    m_pDepthShader->Initialize(*m_pD3D);

    m_pShadowShader = new ShadowShader();
    m_pShadowShader->Initialize(*m_pD3D);

    m_pLightShader = new LightShader();
    m_pLightShader->Initialize(*m_pD3D);

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pLightShader)
    SAFE_RELEASE(m_pShadowShader)
    SAFE_RELEASE(m_pDepthShader)
    SAFE_RELEASE(m_pRenderTexture)
    SAFE_RELEASE(m_pLight)
    SAFE_RELEASE(m_pGroundModel)
    SAFE_RELEASE(m_pSphereModel)
    SAFE_RELEASE(m_pCubeModel)
    SAFE_RELEASE(m_pCamera)
    SAFE_RELEASE(m_pD3D)
}

bool Graphics::Frame(DXSound* pSound, int fps, int cpuUsage, float dt)
{
    static float lightPositionX = -5.0f;

    lightPositionX += 0.05f;
    if (lightPositionX > 5.0f)
    {
        lightPositionX = -5.0f;
    }

    m_pLight->SetPosition(lightPositionX, 8.0f, -5.0f);

    if (!Render(pSound, fps, cpuUsage, dt))
    {
        return false;
    }

    return true;
}

bool Graphics::Render(DXSound* pSound, int fps, int cpuUsage, float dt)
{
   RenderSceneToTexture();

    m_pD3D->BeginFrame(0.1f, 0.1f, 0.1f, 1.0f);

    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX proj = m_pD3D->GetProjectionMatrix();
    dx::XMMATRIX lightView = dx::XMMatrixIdentity();
    dx::XMMATRIX lightProjection = dx::XMMatrixIdentity();

    m_pLight->GetViewMatrix(lightView);
    m_pLight->GetProjectionMatrix(lightProjection);

    world = m_pCubeModel->GetWorldMatrix();
    m_pCubeModel->Bind(*m_pD3D);
    m_pShadowShader->Bind(*m_pD3D, m_pCubeModel->GetIndexCount(), world, view, proj, lightView, lightProjection,
        m_pCubeModel->GetTextureArray()[0], m_pRenderTexture->GetShaderResourceView(), m_pLight->GetPosition3(),
        m_pLight->GetAmbientColor(), m_pLight->GetDiffuseColor());

    world = m_pSphereModel->GetWorldMatrix();
    m_pSphereModel->Bind(*m_pD3D);
    m_pShadowShader->Bind(*m_pD3D, m_pSphereModel->GetIndexCount(), world, view, proj, lightView, lightProjection,
        m_pSphereModel->GetTextureArray()[0], m_pRenderTexture->GetShaderResourceView(), m_pLight->GetPosition3(),
        m_pLight->GetAmbientColor(), m_pLight->GetDiffuseColor());

    world = m_pGroundModel->GetWorldMatrix();
    m_pGroundModel->Bind(*m_pD3D);
    m_pShadowShader->Bind(*m_pD3D, m_pGroundModel->GetIndexCount(), world, view, proj, lightView, lightProjection,
        m_pGroundModel->GetTextureArray()[0], m_pRenderTexture->GetShaderResourceView(), m_pLight->GetPosition3(),
        m_pLight->GetAmbientColor(), m_pLight->GetDiffuseColor());

#pragma region UI
#pragma endregion

    m_pD3D->EndFrame();
    return true;
}

void Graphics::RenderSceneToTexture()
{
    dx::XMMATRIX world = dx::XMMatrixIdentity();
    dx::XMMATRIX lightView = dx::XMMatrixIdentity();
    dx::XMMATRIX lightProjection = dx::XMMatrixIdentity();

    m_pRenderTexture->SetRenderTarget(*m_pD3D);
    m_pRenderTexture->ClearRenderTarget(*m_pD3D, 0.1f, 0.1f, 0.1f, 1.0f);

    m_pLight->GenerateViewMatrix();
    m_pLight->GetViewMatrix(lightView);
    m_pLight->GetProjectionMatrix(lightProjection);

    world = m_pCubeModel->GetWorldMatrix();
    m_pCubeModel->Bind(*m_pD3D);
    m_pDepthShader->Bind(*m_pD3D, m_pCubeModel->GetIndexCount(), world, lightView, lightProjection);

    world = m_pSphereModel->GetWorldMatrix();
    m_pSphereModel->Bind(*m_pD3D);
    m_pDepthShader->Bind(*m_pD3D, m_pSphereModel->GetIndexCount(), world, lightView, lightProjection);

    world = m_pGroundModel->GetWorldMatrix();
    m_pGroundModel->Bind(*m_pD3D);
    m_pDepthShader->Bind(*m_pD3D, m_pGroundModel->GetIndexCount(), world, lightView, lightProjection);

    m_pD3D->SetBackBufferRenderTarget();

    m_pD3D->ResetViewport();
}



