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
#include <Sound/DXSound.h>
#include <imgui/imgui.h>
#include <stdexcept>
#include <sstream>

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

bool Graphics::Frame(DXSound* pSound, int fps, int cpuUsage)
{
    if (!Render(pSound, fps, cpuUsage))
    {
        return false;
    }

    return true;
}

bool Graphics::Render(DXSound* pSound, int fps, int cpuUsage)
{
    m_pD3D->BeginFrame(0.5f, 0.5f, 0.5f, 1.f);
    m_pD2D->BeginFrame();
    
    std::wstringstream wss;
    wss << L"FPS : " << fps << L"\nCPU : " << cpuUsage << L"%";
    m_pD2D->DrawBox(1280 - 100, 0, 1280, 100);
    m_pD2D->WriteText(wss.str(), 1280 - 100, 0, 1280, 100);

    // ����, ��, ���� ����, ���翵 ���� ����� ����.
    dx::XMMATRIX world = m_pModel->GetWorldMatrix();
    dx::XMMATRIX worldFor2D = dx::XMMatrixIdentity();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX viewFor2D = m_pFixedCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();
    dx::XMMATRIX orth = m_pD3D->GetOrthMatrix();

    // 2D �������� ���� ���� ���۸� ����.
    m_pD3D->TurnZBufferOff();

    // 2D �̹��� �������� ���� bitmap ��ü�� ���� ������ �ε��� ���۸� ���������ο� ���ε�.
    m_pBitmap->Bind(*m_pD3D, (1280 / 2) * -1 + (256 / 2), 720 / 2 - (256/ 2));

    // 2D �̹����� texture shader�� �׷���.
    m_pTextureShader->Bind(*m_pD3D, m_pBitmap->GetIndexCount(), worldFor2D, viewFor2D, orth, m_pBitmap->GetTexture());

    m_pD3D->TurnZBufferOn();

    // ���� �����ϴ� ������ �ε��� ���۸� ���������ο� ���ε� ��.
    m_pModel->Bind(*m_pD3D);

    // 3D ���� light shader�� �׷���.
    m_pLightShader->Bind(*m_pD3D, m_pModel->GetIndexCount(), world, view, projection, m_pCamera->GetPosition(),
                          m_pModel->GetTexture(), m_pLight->GetAmbientColor(), m_pLight->GetDiffuseColor(), m_pLight->GetLightDirection(), 
                          m_pLight->GetSpecularColor(), m_pLight->GetSpecularPower());

    m_pModel->SpawnControlWindow();
    m_pCamera->SpawnControlWindow();
    m_pLight->SpawnControlWindow();
    pSound->SpawnControlWindow();

    m_pD2D->EndFrame();
    // �������� ���� ȭ�鿡 ǥ��.
    m_pD3D->EndFrame();

    return true;
}
