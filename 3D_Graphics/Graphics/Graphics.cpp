#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Graphics/D2DGraphics.h>
#include <Graphics/DebugWindow.h>
#include <Graphics/RenderToTexture.h>
#include <Shaders/TextureShader.h>
#include <Shaders/LightShader.h>
#include <Shaders/RefractionShader.h>
#include <Shaders/WaterShader.h>
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
    m_pModelWall = nullptr;
    m_pModelGround = nullptr;
    m_pModelBath = nullptr;
    m_pModelWater = nullptr;
    m_pLightShader = nullptr;
    m_pRefractionTexture = nullptr;
    m_pReflectionTexture = nullptr;
    m_pTextureShader = nullptr;
    m_pRefractionShader = nullptr;
    m_pWaterShader = nullptr;
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

    m_pModelWall = new Model();
    m_pModelWall->Initialize(*m_pD3D, "Resources\\Models\\wall.model", "Resources\\Images\\wall01.png", "Resources\\Images\\bump03.png", "Resources\\Images\\spec02.png");
    
    m_pModelGround = new Model();
    m_pModelGround->Initialize(*m_pD3D, "Resources\\Models\\ground.model", "Resources\\Images\\ground01.png", "Resources\\Images\\bump03.png", "Resources\\Images\\spec02.png");

    m_pModelBath = new Model();
    m_pModelBath->Initialize(*m_pD3D, "Resources\\Models\\bath.model", "Resources\\Images\\marble01.png", "Resources\\Images\\bump03.png", "Resources\\Images\\spec02.png");

    m_pModelWater = new Model();
    m_pModelWater->Initialize(*m_pD3D, "Resources\\Models\\water.model", "Resources\\Images\\water01.png", "Resources\\Images\\bump03.png", "Resources\\Images\\spec02.png");

    m_pCamera = new Camera();
    m_pFixedCamera = new Camera();

    m_pLight = new Light();

    m_pLightShader = new LightShader();
    m_pLightShader->Initialize(*m_pD3D);

    m_pTextureShader = new TextureShader();
    m_pTextureShader->Initialize(*m_pD3D);

    m_pRefractionShader = new RefractionShader();
    m_pRefractionShader->Initialize(*m_pD3D);

    m_pWaterShader = new WaterShader();
    m_pWaterShader->Initialize(*m_pD3D);

    m_pBitmap = new Bitmap();
    m_pBitmap->Initialize(*m_pD3D, screenWidth, screenHeight, screenWidth, screenHeight);

    m_pRefractionTexture = new RenderToTexture();
    m_pRefractionTexture->Initialize(*m_pD3D, screenWidth, screenHeight);

    m_pReflectionTexture = new RenderToTexture();
    m_pReflectionTexture->Initialize(*m_pD3D, screenWidth, screenHeight);

    m_pDebugWindow = new DebugWindow();
    m_pDebugWindow->Initialize(*m_pD3D, screenWidth, screenHeight, 100, 100);

    m_pModelList = new ModelList();
    m_pModelList->Initialize(50);

    m_pFrustum = new Frustum();

    m_pLight->SetDirection(0.0f, -1.0f, 0.5f);
    m_waterTranslation = 0.f;
    m_waterHeight = 2.75f;

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pFrustum)
    SAFE_RELEASE(m_pModelList)
    SAFE_RELEASE(m_pDebugWindow)
    SAFE_RELEASE(m_pRefractionTexture)
    SAFE_RELEASE(m_pReflectionTexture)
    SAFE_RELEASE(m_pBitmap)
    SAFE_RELEASE(m_pRefractionShader)
    SAFE_RELEASE(m_pWaterShader)
    SAFE_RELEASE(m_pTextureShader)
    SAFE_RELEASE(m_pLightShader)
    SAFE_RELEASE(m_pLight)
    SAFE_RELEASE(m_pCamera)
    SAFE_RELEASE(m_pModelWall)
    SAFE_RELEASE(m_pModelGround)
    SAFE_RELEASE(m_pModelBath)
    SAFE_RELEASE(m_pModelWater)
    SAFE_RELEASE(m_pD2D)
    SAFE_RELEASE(m_pD3D)
}

bool Graphics::Frame(DXSound* pSound, int fps, int cpuUsage, float dt)
{
    m_waterTranslation += m_waterTranslationDelta;
    if (m_waterTranslation > 1.0f)
    {
        m_waterTranslation -= 1.0f;
    }

    if (!Render(pSound, fps, cpuUsage, dt))
    {
        return false;
    }

    return true;
}

bool Graphics::Render(DXSound* pSound, int fps, int cpuUsage, float dt)
{
    RenderRefractionTexture();
    RenderReflectionTexture();
    RenderScene();

    return true;
}

void Graphics::RenderRefractionTexture()
{
    XMFLOAT4 clipPlane;
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();

    // 클리핑 평면을 물의 높이 위쪽으로 설정해서 물 위에 있는 것을 모두 클리핑 해줌.
    clipPlane = XMFLOAT4(0.0f, -1.0f, 0.0f, m_waterHeight + 0.1f);

    // 굴절 효과 구현을 위한 굴절 텍스쳐를 그려주기 위해 Render to texture를 사용함.
    m_pRefractionTexture->SetRenderTarget(*m_pD3D);
    m_pRefractionTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);

    // 욕조 모델이 그려질 월드 위치를 정해줌.
    world = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

    m_pModelBath->Bind(*m_pD3D);

    m_pRefractionShader->Bind(*m_pD3D, m_pModelBath->GetIndexCount(), world, view, projection, m_pLight->GetLightDirection(),
        m_pLight->GetAmbientColor(), m_pLight->GetDiffuseColor(), clipPlane, m_pModelBath->GetTextureArray()[0]);

    // 다시 렌더 타겟을 백버퍼로 되돌려줌.
    m_pD3D->SetBackBufferRenderTarget();
}

void Graphics::RenderReflectionTexture()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX reflection = m_pCamera->GetReflectionMatrix(m_waterHeight);
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();

    // 반사 효과 구현을 위한 반사 텍스쳐를 그려주기 위해 Render to texture를 사용함.
    m_pReflectionTexture->SetRenderTarget(*m_pD3D);
    m_pReflectionTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);

    // 벽 모델이 그려질 월드 위치를 정해줌.
    world = XMMatrixTranslation(0.0f, 6.0f, 8.0f);

    m_pModelWall->Bind(*m_pD3D);

    m_pLightShader->Bind(*m_pD3D, m_pModelWall->GetIndexCount(), world, reflection, projection, m_pCamera->GetPosition(),
        m_pModelWall->GetTextureArray()[0], m_pLight->GetAmbientColor(), m_pLight->GetDiffuseColor(), m_pLight->GetLightDirection(),
        m_pLight->GetSpecularColor(), m_pLight->GetSpecularPower());

    // 다시 렌더 타겟을 백버퍼로 되돌려줌.
    m_pD3D->SetBackBufferRenderTarget();
}

void Graphics::RenderScene()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();
    dx::XMMATRIX reflection = m_pCamera->GetReflectionMatrix(m_waterHeight);

    m_pD3D->BeginFrame(0.0f, 0.0f, 0.0f, 1.0f);
    
    // 땅 모델 그리기
    world = XMMatrixTranslation(0.0f, 1.0f, 0.0f);

    m_pModelGround->Bind(*m_pD3D);

    m_pLightShader->Bind(*m_pD3D, m_pModelGround->GetIndexCount(), world, view, projection, m_pCamera->GetPosition(),
        m_pModelGround->GetTextureArray()[0], m_pLight->GetAmbientColor(), m_pLight->GetDiffuseColor(), m_pLight->GetLightDirection(),
        m_pLight->GetSpecularColor(), m_pLight->GetSpecularPower());

    // 벽 모델 그리기
    world = XMMatrixTranslation(0.0f, 6.0f, 8.0f);

    m_pModelWall->Bind(*m_pD3D);

    m_pLightShader->Bind(*m_pD3D, m_pModelWall->GetIndexCount(), world, view, projection, m_pCamera->GetPosition(),
        m_pModelWall->GetTextureArray()[0], m_pLight->GetAmbientColor(), m_pLight->GetDiffuseColor(), m_pLight->GetLightDirection(),
        m_pLight->GetSpecularColor(), m_pLight->GetSpecularPower());

    // 욕조 모델 그리기
    world = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

    m_pModelBath->Bind(*m_pD3D);

    m_pLightShader->Bind(*m_pD3D, m_pModelBath->GetIndexCount(), world, view, projection, m_pCamera->GetPosition(),
    m_pModelBath->GetTextureArray()[0], m_pLight->GetAmbientColor(), m_pLight->GetDiffuseColor(), m_pLight->GetLightDirection(),
    m_pLight->GetSpecularColor(), m_pLight->GetSpecularPower());

    // 물 모델 그리기
    world = XMMatrixTranslation(0.0f, m_waterHeight, 0.0f);

    m_pModelWater->Bind(*m_pD3D);

    m_pWaterShader->Bind(*m_pD3D, m_pModelWater->GetIndexCount(), world, view, projection, reflection, m_pReflectionTexture->GetShaderResourceView(),
        m_pRefractionTexture->GetShaderResourceView(), m_pModelWater->GetTextureArray()[0], m_waterTranslation, m_reflectRefractScale);

#pragma region UI
    if (ImGui::Begin("WaterParameter"))
    {
        ImGui::SliderFloat("ReflectRefractScale", &m_reflectRefractScale, -1.0f, 1.0f, "%.01f");
        if (ImGui::Button("Reset"))
        {
            m_reflectRefractScale = 0.01f;
        }
    }
    ImGui::End();
    m_pCamera->SpawnControlWindow();
    m_pLight->SpawnControlWindow();
#pragma endregion

    m_pD3D->EndFrame();
    return;
}
