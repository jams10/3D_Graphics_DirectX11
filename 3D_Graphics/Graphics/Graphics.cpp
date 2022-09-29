#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Graphics/D2DGraphics.h>
#include <Graphics/DebugWindow.h>
#include <Graphics/RenderToTexture.h>
#include <Shaders/TextureShader.h>
#include <Shaders/LightShader.h>
#include <Shaders/ReflectionShader.h>
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
    m_pModelCube = nullptr;
    m_pModelFloor = nullptr;
    m_pLightShader = nullptr;
    m_pRenderToTexture = nullptr;
    m_pTextureShader = nullptr;
    m_pReflectionShader = nullptr;
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

    m_pReflectionShader = new ReflectionShader();
    m_pReflectionShader->Initialize(*m_pD3D);

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
    SAFE_RELEASE(m_pReflectionShader)
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
    if (!Render(pSound, fps, cpuUsage, dt))
    {
        return false;
    }

    return true;
}

bool Graphics::Render(DXSound* pSound, int fps, int cpuUsage, float dt)
{
    RenderToTextureFunc();
    RenderScene();

    return true;
}

void Graphics::RenderToTextureFunc()
{
    dx::XMMATRIX world = m_pModelCube->GetWorldMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();

    // 렌더 타겟을 텍스쳐로 설정하고 초기화 해줌.
    m_pRenderToTexture->SetRenderTarget(*m_pD3D);
    m_pRenderToTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);

    // 반사 뷰 행렬 얻어오기.
    dx::XMMATRIX reflection = m_pCamera->GetReflectionMatrix(-1.5f);

    m_pModelCube->Bind(*m_pD3D);
    m_pTextureShader->Bind(*m_pD3D, m_pModelCube->GetIndexCount(), world, reflection, projection, m_pModelCube->GetTextureArray()[0]);

    m_pD3D->SetBackBufferRenderTarget();

    return;
}

void Graphics::RenderScene()
{
    dx::XMMATRIX world1 = m_pModelCube->GetWorldMatrix();
    dx::XMMATRIX world2 = m_pModelFloor->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();
    dx::XMMATRIX reflection = m_pCamera->GetReflectionMatrix(-1.5f);

    m_pD3D->BeginFrame(0.0f, 0.0f, 0.0f, 1.0f);

    m_pModelCube->Bind(*m_pD3D);
    m_pTextureShader->Bind(*m_pD3D, m_pModelCube->GetIndexCount(), world1, view, projection, m_pModelCube->GetTextureArray()[0]);

    m_pModelFloor->Bind(*m_pD3D);
    m_pReflectionShader->Bind(*m_pD3D, m_pModelFloor->GetIndexCount(), world2, view, projection, m_pModelFloor->GetTextureArray()[0],
        m_pRenderToTexture->GetShaderResourceView(), reflection);

#pragma region UI
    m_pModelCube->SpawnControlWindow();
    m_pCamera->SpawnControlWindow();
    m_pLight->SpawnControlWindow();
#pragma endregion

    m_pD3D->EndFrame();
    return;
}
