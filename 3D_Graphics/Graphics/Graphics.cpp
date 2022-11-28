#include "Graphics.h"
#include <Graphics/D3DGraphics.h>
#include <Graphics/ParticleSystem.h>
#include <Shaders/ParticleShader.h>
#include <ErrorHandle/DxgiInfoManager.h>
#include <ErrorHandle/CustomException.h>
#include <ErrorHandle/D3DGraphicsExceptionMacros.h>
#include <Objects/Camera.h>
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
    m_pParticleSystem = nullptr;
    m_pParticleShader = nullptr;
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND Wnd)
{
    m_pD3D = new D3DGraphics();
    m_pD3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, Wnd, SCREEN_DEPTH, SCREEN_NEAR);

    m_pParticleSystem = new ParticleSystem;
    m_pParticleSystem->Initialize(*m_pD3D, "Resources\\Images\\star.png");
    
    m_pCamera = new Camera();

    m_pParticleShader = new ParticleShader;
    m_pParticleShader->Initialize(*m_pD3D);


    return true;
}

void Graphics::Shutdown()
{
    SAFE_RELEASE(m_pParticleShader)
    m_pParticleSystem->Shutdown();
    SAFE_RELEASE(m_pParticleSystem)
    SAFE_RELEASE(m_pCamera)
    SAFE_RELEASE(m_pD3D)
}

bool Graphics::Frame(DXSound* pSound, int fps, int cpuUsage, float dt)
{

    m_pParticleSystem->Frame(dt, *m_pD3D);

    if (!Render(pSound, fps, cpuUsage, dt))
    {
        return false;
    }

    return true;
}

bool Graphics::Render(DXSound* pSound, int fps, int cpuUsage, float dt)
{
    m_pD3D->BeginFrame(0.2f, 0.2f, 0.2f, 1.0f);

    dx::XMMATRIX world = m_pD3D->GetWorldMatrix();
    dx::XMMATRIX view = m_pCamera->GetViewMatrix();
    dx::XMMATRIX proj = m_pD3D->GetProjectionMatrix();

    m_pD3D->TurnOnAlphaBlending();

    m_pParticleSystem->Bind(*m_pD3D);

    m_pParticleShader->Bind(*m_pD3D, m_pParticleSystem->GetIndexCount(), world, view, proj, m_pParticleSystem->GetTexture());

#pragma region UI
#pragma endregion

    m_pD3D->TurnOffAlphaBlending();

    m_pD3D->EndFrame();
    return true;
}



