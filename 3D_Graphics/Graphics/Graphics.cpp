#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Graphics/RenderToTexture.h>
#include <Shaders/TextureShader.h>
#include <Shaders/HorizontalBlurShader.h>
#include <Shaders/VerticalBlurShader.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <Objects/Model.h>
#include <Objects/Camera.h>
#include <Objects/Light.h>
#include <Objects/OrthoWindow.h>
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
    m_pFixedCamera = nullptr;
    m_pModel = nullptr;
    m_pSmallWindow = nullptr;
    m_pFullscreenWindow = nullptr;
    m_pHorizontalBlurShader = nullptr;
    m_pVerticalBlurShader = nullptr;
    m_pRenderToTexture = nullptr;
    m_pDownSampleTexture = nullptr;
    m_pHorizontalBlurTexture = nullptr;
    m_pVerticalBlurTexture = nullptr;
    m_pUpSampleTexture = nullptr;
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND Wnd)
{
    m_pD3D = new D3DGraphics();
    m_pD3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, Wnd, SCREEN_DEPTH, SCREEN_NEAR);

    m_pModel = new Model();
    m_pModel->Initialize(*m_pD3D, "Resources\\Models\\Cube.model", "Resources\\Images\\seafloor.png", "Resources\\Images\\noise01.png", "Resources\\Images\\alpha01.png");
    
    m_pCamera = new Camera();
    m_pFixedCamera = new Camera();

    m_pTextureShader = new TextureShader();
    m_pTextureShader->Initialize(*m_pD3D);

    int downSampleWidth = 0, downSampleHeight = 0;
    downSampleWidth = screenWidth / 2;
    downSampleHeight = screenHeight / 2;

    m_pSmallWindow = new OrthoWindow();
    m_pSmallWindow->Initialize(*m_pD3D, downSampleWidth, downSampleHeight);

    m_pFullscreenWindow = new OrthoWindow();
    m_pFullscreenWindow->Initialize(*m_pD3D, screenWidth, screenHeight);

    m_pHorizontalBlurShader = new HorizontalBlurShader();
    m_pHorizontalBlurShader->Initialize(*m_pD3D);

    m_pVerticalBlurShader = new VerticalBlurShader();
    m_pVerticalBlurShader->Initialize(*m_pD3D);

    // 전체 화면 렌더 투 텍스쳐.
    m_pRenderToTexture = new RenderToTexture();
    m_pRenderToTexture->Initialize(*m_pD3D, screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);

    // 다운 샘플링에 사용할 렌더 투 텍스쳐.
    m_pDownSampleTexture = new RenderToTexture();
    m_pDownSampleTexture->Initialize(*m_pD3D, downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);

    // 각각 수평, 수직 블러가 적용된 텍스쳐를 그리는 데 사용할 렌더 투 텍스쳐.
    m_pHorizontalBlurTexture = new RenderToTexture();
    m_pHorizontalBlurTexture->Initialize(*m_pD3D, downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
    m_pVerticalBlurTexture = new RenderToTexture();
    m_pVerticalBlurTexture->Initialize(*m_pD3D, downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);

    // 업 샘플링에 사용할 렌더 투 텍스쳐.
    m_pUpSampleTexture = new RenderToTexture();
    m_pUpSampleTexture->Initialize(*m_pD3D, screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pUpSampleTexture)
    SAFE_RELEASE(m_pHorizontalBlurTexture) 
    SAFE_RELEASE(m_pVerticalBlurTexture)
    SAFE_RELEASE(m_pDownSampleTexture) 
    SAFE_RELEASE(m_pRenderToTexture) 
    SAFE_RELEASE(m_pVerticalBlurShader)
    SAFE_RELEASE(m_pHorizontalBlurShader)
    SAFE_RELEASE(m_pFullscreenWindow)
    SAFE_RELEASE(m_pSmallWindow)
    SAFE_RELEASE(m_pModel)
    SAFE_RELEASE(m_pFixedCamera)
    SAFE_RELEASE(m_pCamera)
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
    // 전체 장면을 텍스쳐에 그려줌.
    RenderSceneToTexture(dt);

    // 장면을 그려준 텍스쳐를 다운 샘플링 함.
    DownSampleTexture();

    // 다운 샘플링한 텍스쳐에 수평 블러를 수행함.
    RenderHorizontalBlurToTexture();

    // 수평 블러가 적용된 텍스쳐에 수직 블러를 수행함.
    RenderVerticalBlurToTexture();

    // 수평, 수직 블러가 적용된 텍스쳐를 원래 크기로 다시 업 샘플링 해줌.
    UpSampleTexture();

    // 업샘플링한 블러 텍스쳐를 화면에 그려줌.
    Render2DTextureScene();

    return true;
}

void Graphics::RenderSceneToTexture(float dt)
{
    dx::XMMATRIX world = m_pModel->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();

    // 렌더 타겟 설정.
    m_pRenderToTexture->SetRenderTarget(*m_pD3D);

    // 렌더 타겟을 초기화.
    m_pRenderToTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);

    // 모델 자원 파이프라인에 바인딩.
    m_pModel->Bind(*m_pD3D);

    // 텍스쳐 셰이더를 통해 모델 그리기.
    m_pTextureShader->Bind(*m_pD3D, m_pModel->GetIndexCount(), world, view, projection, m_pModel->GetTextureArray()[0]);

    // 렌더 타겟을 텍스쳐에서 다시 백 퍼버로 되돌려줌.
    m_pD3D->SetBackBufferRenderTarget();

    // 뷰포트를 원래대로 되돌림.
    m_pD3D->ResetViewport();
}

void Graphics::DownSampleTexture()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pFixedCamera->GetViewMatrix(); // 2D 이미지 작업을 수행하는 경우, 카메라는 화면 속을 향하는 고정된 정면 카메라를 사용.
    dx::XMMATRIX ortho;

    // 렌더 타겟을 이 렌더 투 텍스쳐로 설정.
    m_pDownSampleTexture->SetRenderTarget(*m_pD3D);

    // 렌더 타겟 초기화.
    m_pDownSampleTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);
    // 단순히 2D 텍스쳐를 다운 샘플링 하기 위한 작업이기 때문에 직교 투영 행렬을 가져옴.
    m_pDownSampleTexture->GetOrthoMatrix(ortho);

    // 2D 렌더링을 위해 Z 버퍼 기능을 끔.
    m_pD3D->TurnZBufferOff();

    // 사각형 모델의 자원을 파이프라인에 바인딩.
    m_pSmallWindow->Bind(*m_pD3D);

    // 텍스쳐 셰이더를 통해 작은 사각형 모델에 씬을 렌더링한 텍스쳐를 그려줌.
    // 이렇게 하면 모델의 크기 보다 텍스쳐 크기가 더 크기 때문에 샘플러가 알아서 모델의 크기로 텍스쳐를 샘플링 하면서 다운 샘플링이 됨.
    m_pTextureShader->Bind(*m_pD3D, m_pSmallWindow->GetIndexCount(), world, view, ortho, m_pRenderToTexture->GetShaderResourceView());

    // Z 버퍼를 켜줌.
    m_pD3D->TurnZBufferOn();

    // 렌더 타겟을 다시 백 버퍼로 되돌려줌.
    m_pD3D->SetBackBufferRenderTarget();

    // 뷰 포트를 원래대로 되돌림.
    m_pD3D->ResetViewport();
}

void Graphics::RenderHorizontalBlurToTexture()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pFixedCamera->GetViewMatrix();
    dx::XMMATRIX ortho;
    float screenSizeX;

    // 수평 블러 셰이더에서 사용할 화면의 너비 값을 미리 저장해둠.
    screenSizeX = (float)m_pHorizontalBlurTexture->GetTextureWidth();

    // 렌더 타겟을 이 렌더 투 텍스쳐로 설정.
    m_pHorizontalBlurTexture->SetRenderTarget(*m_pD3D);

    // 렌더 타겟 초기화.
    m_pHorizontalBlurTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);
    // 단순히 2D 텍스쳐를 다운 샘플링 하기 위한 작업이기 때문에 직교 투영 행렬을 가져옴.
    m_pHorizontalBlurTexture->GetOrthoMatrix(ortho);

    // 2D 렌더링을 위해 Z 버퍼 기능을 끔.
    m_pD3D->TurnZBufferOff();

    // 사각형 모델의 자원을 파이프라인에 바인딩.
    m_pSmallWindow->Bind(*m_pD3D);

    // 수평 블러 셰이더를 통해 작은 사각형 모델에 다운 샘플링한 텍스쳐를 수평 블러를 적용해 렌더링 해줌.
    m_pHorizontalBlurShader->Bind(*m_pD3D, m_pSmallWindow->GetIndexCount(), world, view, ortho,
        m_pDownSampleTexture->GetShaderResourceView(), screenSizeX);

    // Z 버퍼를 켜줌.
    m_pD3D->TurnZBufferOn();

    // 렌더 타겟을 다시 백 버퍼로 되돌려줌.
    m_pD3D->SetBackBufferRenderTarget();

    // 뷰 포트를 원래대로 되돌림.
    m_pD3D->ResetViewport();

}

void Graphics::RenderVerticalBlurToTexture()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pFixedCamera->GetViewMatrix();
    dx::XMMATRIX ortho;
    float screenSizeY;

    // 수직 블러 셰이더에서 사용할 화면의 너비 값을 미리 저장해둠.
    screenSizeY = (float)m_pVerticalBlurTexture->GetTextureHeight();

    // 렌더 타겟을 이 렌더 투 텍스쳐로 설정.
    m_pVerticalBlurTexture->SetRenderTarget(*m_pD3D);

    // 렌더 타겟 초기화.
    m_pVerticalBlurTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);
    // 단순히 2D 텍스쳐를 다운 샘플링 하기 위한 작업이기 때문에 직교 투영 행렬을 가져옴.
    m_pVerticalBlurTexture->GetOrthoMatrix(ortho);

    // 2D 렌더링을 위해 Z 버퍼 기능을 끔.
    m_pD3D->TurnZBufferOff();

    // 사각형 모델의 자원을 파이프라인에 바인딩.
    m_pSmallWindow->Bind(*m_pD3D);

    // 수직 블러 셰이더를 통해 작은 사각형 모델에 수평 블러가 적용된 텍스쳐를 렌더링 해줌.
    m_pVerticalBlurShader->Bind(*m_pD3D, m_pSmallWindow->GetIndexCount(), world, view, ortho,
        m_pHorizontalBlurTexture->GetShaderResourceView(), screenSizeY);

    // Z 버퍼를 켜줌.
    m_pD3D->TurnZBufferOn();

    // 렌더 타겟을 다시 백 버퍼로 되돌려줌.
    m_pD3D->SetBackBufferRenderTarget();

    // 뷰 포트를 원래대로 되돌림.
    m_pD3D->ResetViewport();
}

void Graphics::UpSampleTexture()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pFixedCamera->GetViewMatrix();
    dx::XMMATRIX ortho;

    // 렌더 타겟을 이 렌더 투 텍스쳐로 설정.
    m_pUpSampleTexture->SetRenderTarget(*m_pD3D);

    // 렌더 타겟 초기화.
    m_pUpSampleTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);
    // 단순히 2D 텍스쳐를 다운 샘플링 하기 위한 작업이기 때문에 직교 투영 행렬을 가져옴. (이 때 직교 투영 행렬은 블러 렌더 투 텍스쳐와 다른 크기를 가짐.)
    m_pUpSampleTexture->GetOrthoMatrix(ortho);

    // 2D 렌더링을 위해 Z 버퍼 기능을 끔.
    m_pD3D->TurnZBufferOff();

    // 전체 화면 크기의 사각형 모델의 자원을 파이프라인에 바인딩.
    m_pFullscreenWindow->Bind(*m_pD3D);

    // 텍스쳐 셰이더를 통해 전체 화면 크기의 사각형 모델에 수평 블러가 적용된 텍스쳐를 렌더링 해줌.
    // 텍스쳐 크기 보다 모델 크기가 크기 때문에 샘플러가 알아서 업샘플링을 해줌.
    m_pTextureShader->Bind(*m_pD3D, m_pFullscreenWindow->GetIndexCount(), world, view, ortho,
        m_pVerticalBlurTexture->GetShaderResourceView());

    // Z 버퍼를 켜줌.
    m_pD3D->TurnZBufferOn();

    // 렌더 타겟을 다시 백 버퍼로 되돌려줌.
    m_pD3D->SetBackBufferRenderTarget();

    // 뷰 포트를 원래대로 되돌림.
    m_pD3D->ResetViewport();
}

void Graphics::Render2DTextureScene()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pFixedCamera->GetViewMatrix();
    dx::XMMATRIX ortho = m_pD3D->GetOrthMatrix();

    m_pD3D->BeginFrame(0.2f, 0.2f, 0.2f, 1.0f);

    // 2D 렌더링을 하기 위해 Z 버퍼 기능을 끔.
    m_pD3D->TurnZBufferOff();

    // 전체 화면 크기 사각형의 자원플 파이프라인에 바인딩.
    m_pFullscreenWindow->Bind(*m_pD3D);

    // 텍스쳐 셰이더를 통해 업샘플링된 텍스쳐를 전체 화면 크기 사각형에 렌더링 해줌.
    m_pTextureShader->Bind(*m_pD3D, m_pFullscreenWindow->GetIndexCount(), world, view, ortho, m_pUpSampleTexture->GetShaderResourceView());

    // 2D 렌더링이 끝나면 Z 버퍼를 다시 켜줌.
    m_pD3D->TurnZBufferOn();

#pragma region UI
    //m_pCamera->SpawnControlWindow();
    m_pModel->SpawnControlWindow();
#pragma endregion

    m_pD3D->EndFrame();
}
