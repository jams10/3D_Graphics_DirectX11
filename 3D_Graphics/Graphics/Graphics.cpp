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

    // ��ü ȭ�� ���� �� �ؽ���.
    m_pRenderToTexture = new RenderToTexture();
    m_pRenderToTexture->Initialize(*m_pD3D, screenWidth, screenHeight, SCREEN_DEPTH, SCREEN_NEAR);

    // �ٿ� ���ø��� ����� ���� �� �ؽ���.
    m_pDownSampleTexture = new RenderToTexture();
    m_pDownSampleTexture->Initialize(*m_pD3D, downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);

    // ���� ����, ���� ���� ����� �ؽ��ĸ� �׸��� �� ����� ���� �� �ؽ���.
    m_pHorizontalBlurTexture = new RenderToTexture();
    m_pHorizontalBlurTexture->Initialize(*m_pD3D, downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);
    m_pVerticalBlurTexture = new RenderToTexture();
    m_pVerticalBlurTexture->Initialize(*m_pD3D, downSampleWidth, downSampleHeight, SCREEN_DEPTH, SCREEN_NEAR);

    // �� ���ø��� ����� ���� �� �ؽ���.
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
    // ��ü ����� �ؽ��Ŀ� �׷���.
    RenderSceneToTexture(dt);

    // ����� �׷��� �ؽ��ĸ� �ٿ� ���ø� ��.
    DownSampleTexture();

    // �ٿ� ���ø��� �ؽ��Ŀ� ���� ���� ������.
    RenderHorizontalBlurToTexture();

    // ���� ���� ����� �ؽ��Ŀ� ���� ���� ������.
    RenderVerticalBlurToTexture();

    // ����, ���� ���� ����� �ؽ��ĸ� ���� ũ��� �ٽ� �� ���ø� ����.
    UpSampleTexture();

    // �����ø��� �� �ؽ��ĸ� ȭ�鿡 �׷���.
    Render2DTextureScene();

    return true;
}

void Graphics::RenderSceneToTexture(float dt)
{
    dx::XMMATRIX world = m_pModel->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();

    // ���� Ÿ�� ����.
    m_pRenderToTexture->SetRenderTarget(*m_pD3D);

    // ���� Ÿ���� �ʱ�ȭ.
    m_pRenderToTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);

    // �� �ڿ� ���������ο� ���ε�.
    m_pModel->Bind(*m_pD3D);

    // �ؽ��� ���̴��� ���� �� �׸���.
    m_pTextureShader->Bind(*m_pD3D, m_pModel->GetIndexCount(), world, view, projection, m_pModel->GetTextureArray()[0]);

    // ���� Ÿ���� �ؽ��Ŀ��� �ٽ� �� �۹��� �ǵ�����.
    m_pD3D->SetBackBufferRenderTarget();

    // ����Ʈ�� ������� �ǵ���.
    m_pD3D->ResetViewport();
}

void Graphics::DownSampleTexture()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pFixedCamera->GetViewMatrix(); // 2D �̹��� �۾��� �����ϴ� ���, ī�޶�� ȭ�� ���� ���ϴ� ������ ���� ī�޶� ���.
    dx::XMMATRIX ortho;

    // ���� Ÿ���� �� ���� �� �ؽ��ķ� ����.
    m_pDownSampleTexture->SetRenderTarget(*m_pD3D);

    // ���� Ÿ�� �ʱ�ȭ.
    m_pDownSampleTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);
    // �ܼ��� 2D �ؽ��ĸ� �ٿ� ���ø� �ϱ� ���� �۾��̱� ������ ���� ���� ����� ������.
    m_pDownSampleTexture->GetOrthoMatrix(ortho);

    // 2D �������� ���� Z ���� ����� ��.
    m_pD3D->TurnZBufferOff();

    // �簢�� ���� �ڿ��� ���������ο� ���ε�.
    m_pSmallWindow->Bind(*m_pD3D);

    // �ؽ��� ���̴��� ���� ���� �簢�� �𵨿� ���� �������� �ؽ��ĸ� �׷���.
    // �̷��� �ϸ� ���� ũ�� ���� �ؽ��� ũ�Ⱑ �� ũ�� ������ ���÷��� �˾Ƽ� ���� ũ��� �ؽ��ĸ� ���ø� �ϸ鼭 �ٿ� ���ø��� ��.
    m_pTextureShader->Bind(*m_pD3D, m_pSmallWindow->GetIndexCount(), world, view, ortho, m_pRenderToTexture->GetShaderResourceView());

    // Z ���۸� ����.
    m_pD3D->TurnZBufferOn();

    // ���� Ÿ���� �ٽ� �� ���۷� �ǵ�����.
    m_pD3D->SetBackBufferRenderTarget();

    // �� ��Ʈ�� ������� �ǵ���.
    m_pD3D->ResetViewport();
}

void Graphics::RenderHorizontalBlurToTexture()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pFixedCamera->GetViewMatrix();
    dx::XMMATRIX ortho;
    float screenSizeX;

    // ���� �� ���̴����� ����� ȭ���� �ʺ� ���� �̸� �����ص�.
    screenSizeX = (float)m_pHorizontalBlurTexture->GetTextureWidth();

    // ���� Ÿ���� �� ���� �� �ؽ��ķ� ����.
    m_pHorizontalBlurTexture->SetRenderTarget(*m_pD3D);

    // ���� Ÿ�� �ʱ�ȭ.
    m_pHorizontalBlurTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);
    // �ܼ��� 2D �ؽ��ĸ� �ٿ� ���ø� �ϱ� ���� �۾��̱� ������ ���� ���� ����� ������.
    m_pHorizontalBlurTexture->GetOrthoMatrix(ortho);

    // 2D �������� ���� Z ���� ����� ��.
    m_pD3D->TurnZBufferOff();

    // �簢�� ���� �ڿ��� ���������ο� ���ε�.
    m_pSmallWindow->Bind(*m_pD3D);

    // ���� �� ���̴��� ���� ���� �簢�� �𵨿� �ٿ� ���ø��� �ؽ��ĸ� ���� ���� ������ ������ ����.
    m_pHorizontalBlurShader->Bind(*m_pD3D, m_pSmallWindow->GetIndexCount(), world, view, ortho,
        m_pDownSampleTexture->GetShaderResourceView(), screenSizeX);

    // Z ���۸� ����.
    m_pD3D->TurnZBufferOn();

    // ���� Ÿ���� �ٽ� �� ���۷� �ǵ�����.
    m_pD3D->SetBackBufferRenderTarget();

    // �� ��Ʈ�� ������� �ǵ���.
    m_pD3D->ResetViewport();

}

void Graphics::RenderVerticalBlurToTexture()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pFixedCamera->GetViewMatrix();
    dx::XMMATRIX ortho;
    float screenSizeY;

    // ���� �� ���̴����� ����� ȭ���� �ʺ� ���� �̸� �����ص�.
    screenSizeY = (float)m_pVerticalBlurTexture->GetTextureHeight();

    // ���� Ÿ���� �� ���� �� �ؽ��ķ� ����.
    m_pVerticalBlurTexture->SetRenderTarget(*m_pD3D);

    // ���� Ÿ�� �ʱ�ȭ.
    m_pVerticalBlurTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);
    // �ܼ��� 2D �ؽ��ĸ� �ٿ� ���ø� �ϱ� ���� �۾��̱� ������ ���� ���� ����� ������.
    m_pVerticalBlurTexture->GetOrthoMatrix(ortho);

    // 2D �������� ���� Z ���� ����� ��.
    m_pD3D->TurnZBufferOff();

    // �簢�� ���� �ڿ��� ���������ο� ���ε�.
    m_pSmallWindow->Bind(*m_pD3D);

    // ���� �� ���̴��� ���� ���� �簢�� �𵨿� ���� ���� ����� �ؽ��ĸ� ������ ����.
    m_pVerticalBlurShader->Bind(*m_pD3D, m_pSmallWindow->GetIndexCount(), world, view, ortho,
        m_pHorizontalBlurTexture->GetShaderResourceView(), screenSizeY);

    // Z ���۸� ����.
    m_pD3D->TurnZBufferOn();

    // ���� Ÿ���� �ٽ� �� ���۷� �ǵ�����.
    m_pD3D->SetBackBufferRenderTarget();

    // �� ��Ʈ�� ������� �ǵ���.
    m_pD3D->ResetViewport();
}

void Graphics::UpSampleTexture()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pFixedCamera->GetViewMatrix();
    dx::XMMATRIX ortho;

    // ���� Ÿ���� �� ���� �� �ؽ��ķ� ����.
    m_pUpSampleTexture->SetRenderTarget(*m_pD3D);

    // ���� Ÿ�� �ʱ�ȭ.
    m_pUpSampleTexture->ClearRenderTarget(*m_pD3D, 0.0f, 0.0f, 0.0f, 1.0f);
    // �ܼ��� 2D �ؽ��ĸ� �ٿ� ���ø� �ϱ� ���� �۾��̱� ������ ���� ���� ����� ������. (�� �� ���� ���� ����� �� ���� �� �ؽ��Ŀ� �ٸ� ũ�⸦ ����.)
    m_pUpSampleTexture->GetOrthoMatrix(ortho);

    // 2D �������� ���� Z ���� ����� ��.
    m_pD3D->TurnZBufferOff();

    // ��ü ȭ�� ũ���� �簢�� ���� �ڿ��� ���������ο� ���ε�.
    m_pFullscreenWindow->Bind(*m_pD3D);

    // �ؽ��� ���̴��� ���� ��ü ȭ�� ũ���� �簢�� �𵨿� ���� ���� ����� �ؽ��ĸ� ������ ����.
    // �ؽ��� ũ�� ���� �� ũ�Ⱑ ũ�� ������ ���÷��� �˾Ƽ� �����ø��� ����.
    m_pTextureShader->Bind(*m_pD3D, m_pFullscreenWindow->GetIndexCount(), world, view, ortho,
        m_pVerticalBlurTexture->GetShaderResourceView());

    // Z ���۸� ����.
    m_pD3D->TurnZBufferOn();

    // ���� Ÿ���� �ٽ� �� ���۷� �ǵ�����.
    m_pD3D->SetBackBufferRenderTarget();

    // �� ��Ʈ�� ������� �ǵ���.
    m_pD3D->ResetViewport();
}

void Graphics::Render2DTextureScene()
{
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pFixedCamera->GetViewMatrix();
    dx::XMMATRIX ortho = m_pD3D->GetOrthMatrix();

    m_pD3D->BeginFrame(0.2f, 0.2f, 0.2f, 1.0f);

    // 2D �������� �ϱ� ���� Z ���� ����� ��.
    m_pD3D->TurnZBufferOff();

    // ��ü ȭ�� ũ�� �簢���� �ڿ��� ���������ο� ���ε�.
    m_pFullscreenWindow->Bind(*m_pD3D);

    // �ؽ��� ���̴��� ���� �����ø��� �ؽ��ĸ� ��ü ȭ�� ũ�� �簢���� ������ ����.
    m_pTextureShader->Bind(*m_pD3D, m_pFullscreenWindow->GetIndexCount(), world, view, ortho, m_pUpSampleTexture->GetShaderResourceView());

    // 2D �������� ������ Z ���۸� �ٽ� ����.
    m_pD3D->TurnZBufferOn();

#pragma region UI
    //m_pCamera->SpawnControlWindow();
    m_pModel->SpawnControlWindow();
#pragma endregion

    m_pD3D->EndFrame();
}
