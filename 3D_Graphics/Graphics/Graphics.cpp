#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Graphics/D2DGraphics.h>
#include <Shaders/TextureShader.h>
#include <Shaders/LightShader.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <Objects/Model.h>
#include <Objects/Camera.h>
#include <Objects/Light.h>
#include <Objects/Bitmap.h>
#include <Input/Keyboard.h>
#include <imgui/imgui.h>
#include <stdexcept>

#define SAFE_RELEASE(p) if(p){delete p; p=nullptr;}

Graphics::Graphics()
{
    m_pD3D = nullptr;
    m_pD2D = nullptr;
    m_pCamera = nullptr;
    m_pModel = nullptr;
    m_pLightShader = nullptr;
    m_pBitmap = nullptr;
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
    m_pModel->Initialize(*m_pD3D, "Resources\\Models\\TestBox.obj", "Resources\\Images\\seafloor.png");

    m_pCamera = new Camera();
    m_pFixedCamera = new Camera();

    m_pLight = new Light();

    m_pTextureShader = new TextureShader();
    m_pTextureShader->Initialize(*m_pD3D);

    m_pLightShader = new LightShader();
    m_pLightShader->Initialize(*m_pD3D);

    m_pBitmap = new Bitmap();
    m_pBitmap->Initialize(*m_pD3D, screenWidth, screenHeight, "Resources\\Images\\seafloor.png", 256, 256);

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pD3D)
    SAFE_RELEASE(m_pD2D)
    SAFE_RELEASE(m_pModel)
    SAFE_RELEASE(m_pCamera)
    SAFE_RELEASE(m_pTextureShader)
    SAFE_RELEASE(m_pLightShader)
    SAFE_RELEASE(m_pBitmap)
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
    m_pD2D->BeginFrame();

    m_pD2D->DrawBox(0, 720 - 256, 256, 720);
    m_pD2D->WriteText(L"Direct2D 그리기 테스트!");

    // 월드, 뷰, 원근 투영, 정사영 투영 행렬을 얻어옴.
    dx::XMMATRIX world = m_pModel->GetWorldMatrix();
    dx::XMMATRIX worldFor2D = dx::XMMatrixIdentity();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX viewFor2D = m_pFixedCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();
    dx::XMMATRIX orth = m_pD3D->GetOrthMatrix();

    // 2D 렌더링을 위해 깊이 버퍼를 꺼줌.
    m_pD3D->TurnZBufferOff();

    // 2D 이미지 렌더링을 위한 bitmap 객체가 가진 정점과 인덱스 버퍼를 파이프라인에 바인딩.
    m_pBitmap->Bind(*m_pD3D, (1280 / 2) * -1 + (256 / 2), 720 / 2 - (256/ 2));

    // 2D 이미지는 texture shader로 그려줌.
    m_pTextureShader->Bind(*m_pD3D, m_pBitmap->GetIndexCount(), worldFor2D, viewFor2D, orth, m_pBitmap->GetTexture());

    m_pD3D->TurnZBufferOn();

    // 모델을 구성하는 정점과 인덱스 버퍼를 파이프라인에 바인딩 함.
    m_pModel->Bind(*m_pD3D);

    // 3D 모델은 light shader로 그려줌.
    m_pLightShader->Bind(*m_pD3D, m_pModel->GetIndexCount(), world, view, projection, m_pCamera->GetPosition(),
                          m_pModel->GetTexture(), m_pLight->GetAmbientColor(), m_pLight->GetDiffuseColor(), m_pLight->GetLightDirection(), 
                          m_pLight->GetSpecularColor(), m_pLight->GetSpecularPower());

    m_pModel->SpawnControlWindow();
    m_pCamera->SpawnControlWindow();
    m_pLight->SpawnControlWindow();

    m_pD2D->EndFrame();
    // 렌더링된 씬을 화면에 표시.
    m_pD3D->EndFrame();

    return true;
}
