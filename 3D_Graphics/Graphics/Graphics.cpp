#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Graphics/TextureShader.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <Objects/Model.h>
#include <Objects/Camera.h>
#include <Input/Keyboard.h>
#include <imgui/imgui.h>
#include <stdexcept>

#define SAFE_RELEASE(p) if(p){delete p; p=nullptr;}

Graphics::Graphics()
{
    m_pD3D = nullptr;
    m_pCamera = nullptr;
    m_pModel = nullptr;
    m_pTextureShader = nullptr;
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND Wnd)
{
    m_pD3D = new D3DGraphics();
    m_pD3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, Wnd, SCREEN_DEPTH, SCREEN_NEAR);

    m_pModel = new Model();
    m_pModel->Initialize(*m_pD3D, "Images\\seafloor.png");

    m_pCamera = new Camera();
    m_pCamera->SetLocation(0.0f, 0.0f, -5.0f);

    m_pTextureShader = new TextureShader();
    m_pTextureShader->Initialize(*m_pD3D);

    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pD3D)
    SAFE_RELEASE(m_pModel)
    SAFE_RELEASE(m_pCamera)
    SAFE_RELEASE(m_pTextureShader)
}

bool Graphics::Frame(Keyboard* kbd, float dt)
{
    dx::XMFLOAT3 pos = m_pCamera->GetLocation();
    dx::XMFLOAT3 rot = m_pCamera->GetRotation();
    if (kbd->KeyIsPressed('W'))
    {
        m_pCamera->SetLocation(pos.x, pos.y, pos.z + 20.0f * dt);
    }
    if (kbd->KeyIsPressed('S'))
    {
        m_pCamera->SetLocation(pos.x, pos.y, pos.z - 20.0f * dt);
    }
    if (kbd->KeyIsPressed('A'))
    {
        m_pCamera->SetLocation(pos.x - 20.0f * dt, pos.y, pos.z);
    }
    if (kbd->KeyIsPressed('D'))
    {
        m_pCamera->SetLocation(pos.x + 20.0f * dt, pos.y, pos.z);
    }
    if (kbd->KeyIsPressed('Q'))
    {
        m_pCamera->SetRotation(rot.x, rot.y, rot.z - 20.0f * dt);
    }
    if (kbd->KeyIsPressed('E'))
    {
        m_pCamera->SetRotation(rot.x, rot.y, rot.z + 20.0f * dt);
    }

    if (!Render())
    {
        return false;
    }

    return true;
}

bool Graphics::Render()
{
    m_pD3D->BeginFrame(0.5f, 0.5f, 0.5f, 1.f);

    // ī�޶��� ���� ��ġ�� ���� �� ��ȯ ����� ����ؼ� ����.
    m_pCamera->Update();

    // ����, ��, ���� ����� ����.
    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX projection = m_pD3D->GetProjectionMatrix();

    // ���� �����ϴ� ������ �ε��� ���۸� ���������ο� ���ε� ��.
    m_pModel->Bind(*m_pD3D);

    // ���� ���̴��� ����� ��� ���۸� �� ��� �����ͷ� �������ְ�, ���̴� �� ��� ���۸� ���������ο� ���ε� ����.
    m_pTextureShader->Bind(*m_pD3D, m_pModel->GetIndexCount(), world, view, projection, m_pModel->GetTexture());

    // �������� ���� ȭ�鿡 ǥ��.
    m_pD3D->EndFrame();

    return true;
}
