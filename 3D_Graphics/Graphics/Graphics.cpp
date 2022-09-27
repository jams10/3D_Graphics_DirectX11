#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Graphics/D2DGraphics.h>
#include <Graphics/DebugWindow.h>
#include <Graphics/RenderToTexture.h>
#include <Shaders/TextureShader.h>
#include <Shaders/LightShader.h>
#include <Shaders/TransparentShader.h>
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

#define SAFE_RELEASE(p) if(p){delete p; p=nullptr;}

Graphics::Graphics()
{
    accumulatedTime = 0.f;
    blendAmount = 0.5f;
    m_pD3D = nullptr;
    m_pD2D = nullptr;
    m_pCamera = nullptr;
    m_pModel1 = nullptr;
    m_pModel2 = nullptr;
    m_pLightShader = nullptr;
    m_pTextureShader = nullptr;
    m_pTransparentShader = nullptr;
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

    m_pModel1 = new Model();
    m_pModel1->Initialize(*m_pD3D, "Resources\\Models\\Cube.model", "Resources\\Images\\dirt01.png", "Resources\\Images\\bump03.png", "Resources\\Images\\spec02.png");
    
    m_pModel2 = new Model();
    m_pModel2->Initialize(*m_pD3D, "Resources\\Models\\Cube.model", "Resources\\Images\\stone01.png", "Resources\\Images\\bump03.png", "Resources\\Images\\spec02.png");

    m_pCamera = new Camera();
    m_pFixedCamera = new Camera();

    m_pLight = new Light();

    m_pLightShader = new LightShader();
    m_pLightShader->Initialize(*m_pD3D);

    m_pTextureShader = new TextureShader();
    m_pTextureShader->Initialize(*m_pD3D);

    m_pTransparentShader = new TransparentShader();
    m_pTransparentShader->Initialize(*m_pD3D);

    m_pBitmap = new Bitmap();
    m_pBitmap->Initialize(*m_pD3D, screenWidth, screenHeight, "Resources\\Images\\seafloor.png", 256, 256);

    m_pRenderToTexture = new RenderToTexture();
    m_pRenderToTexture->Initialize(*m_pD3D, screenWidth, screenHeight);

    m_pDebugWindow = new DebugWindow();
    m_pDebugWindow->Initialize(*m_pD3D, screenWidth, screenHeight, 100, 100);

    m_pModelList = new ModelList();
    m_pModelList->Initialize(50);

    m_pFrustum = new Frustum();

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pFrustum)
    SAFE_RELEASE(m_pModelList)
    SAFE_RELEASE(m_pDebugWindow)
    SAFE_RELEASE(m_pRenderToTexture)
    SAFE_RELEASE(m_pBitmap)
    SAFE_RELEASE(m_pTransparentShader)
    SAFE_RELEASE(m_pTextureShader)
    SAFE_RELEASE(m_pLightShader)
    SAFE_RELEASE(m_pLight)
    SAFE_RELEASE(m_pCamera)
    SAFE_RELEASE(m_pModel2)
    SAFE_RELEASE(m_pModel1)
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
    XMFLOAT3 clipNormal(0.0f, 0.0f, -1.0f);
    XMFLOAT4 clipPlane(clipNormal.x, clipNormal.y, clipNormal.z, 0.f);

    m_pD3D->BeginFrame(0.3f, 0.3f, 0.3f, 1.0f);

    dx::XMMATRIX world1 = m_pModel1->GetWorldMatrix();
    dx::XMMATRIX world2 = m_pModel2->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();


    accumulatedTime += dt;

#pragma region TranslationUI
    if (ImGui::Begin("TransParent"))
    {
        ImGui::SliderFloat("BlendAmount", &blendAmount, 0.f, 1.0f, "%.1f");
        if (ImGui::Button("Reset"))
        {
            blendAmount = 0.f;
        }
    }
    ImGui::End();
#pragma endregion

    m_pModel1->Bind(*m_pD3D);
    m_pTextureShader->Bind(*m_pD3D, m_pModel1->GetIndexCount(), world1, view, projection, m_pModel1->GetTextureArray()[0]);
    
    m_pD3D->TurnOnAlphaBlending();

    m_pModel2->Bind(*m_pD3D);
    m_pTransparentShader->Bind(*m_pD3D, m_pModel2->GetIndexCount(), world2, view, projection, m_pModel2->GetTextureArray()[0], blendAmount);

    m_pD3D->TurnOffAlphaBlending();

#pragma region UI
    m_pModel1->SpawnControlWindow();
    m_pCamera->SpawnControlWindow();
    m_pLight->SpawnControlWindow();
    pSound->SpawnControlWindow();
#pragma endregion

    m_pD3D->EndFrame();

    return true;
}

void Graphics::RenderToTextureFunc()
{
  
}

void Graphics::RenderScene()
{
}
