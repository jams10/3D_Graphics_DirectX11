#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Shaders/LightShader.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <Objects/Model.h>
#include <Objects/Camera.h>
#include <Objects/Light.h>
#include <Input/Keyboard.h>
#include <imgui/imgui.h>
#include <stdexcept>

#define SAFE_RELEASE(p) if(p){delete p; p=nullptr;}

Graphics::Graphics()
{
    m_pD3D = nullptr;
    m_pCamera = nullptr;
    m_pModel = nullptr;
    m_pLightShader = nullptr;
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND Wnd)
{
    m_pD3D = new D3DGraphics();
    m_pD3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, Wnd, SCREEN_DEPTH, SCREEN_NEAR);

    m_pModel = new Model();
    m_pModel->Initialize(*m_pD3D, "Resources\\Models\\TestBox.obj", "Resources\\Images\\seafloor.png");

    m_pCamera = new Camera();

    m_pLight = new Light();

    m_pLightShader = new LightShader();
    m_pLightShader->Initialize(*m_pD3D);

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pD3D)
    SAFE_RELEASE(m_pModel)
    SAFE_RELEASE(m_pCamera)
    SAFE_RELEASE(m_pLightShader)
}

bool Graphics::Frame()
{
    if (!Render())
    {
        return false;
    }

    return true;
}

bool Graphics::Render()
{
    m_pD3D->BeginFrame(0.5f, 0.5f, 0.5f, 1.f);

    // 월드, 뷰, 투영 행렬을 얻어옴.
    dx::XMMATRIX world = m_pModel->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();

    // 모델을 구성하는 정점과 인덱스 버퍼를 파이프라인에 바인딩 함.
    m_pModel->Bind(*m_pD3D);

    // 정점 셰이더에 사용할 상수 버퍼를 각 행렬 데이터로 설정해주고, 셰이더 및 상수 버퍼를 파이프라인에 바인딩 해줌.
    m_pLightShader->Bind(*m_pD3D, m_pModel->GetIndexCount(), world, view, projection, m_pCamera->GetPosition(),
                          m_pModel->GetTexture(), m_pLight->GetAmbientColor(), m_pLight->GetDiffuseColor(), m_pLight->GetLightDirection(), 
                          m_pLight->GetSpecularColor(), m_pLight->GetSpecularPower());

    m_pModel->SpawnControlWindow();
    m_pCamera->SpawnControlWindow();
    m_pLight->SpawnControlWindow();

    // 렌더링된 씬을 화면에 표시.
    m_pD3D->EndFrame();

    return true;
}
