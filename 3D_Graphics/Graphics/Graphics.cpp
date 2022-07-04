#include "Graphics.h"
#include <Graphics/D3DGraphics.h>

Graphics::Graphics()
{
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize(int screenWidth, int screenHeight, HWND Wnd)
{
    m_pD3D = new D3DGraphics();
    
    if (!m_pD3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, Wnd, SCREEN_DEPTH, SCREEN_NEAR))
    {
        // Cannot initialize the D3DGraphics instance.
        return false;
    }

    return true;
}

void Graphics::Shutdown()
{
    if (m_pD3D)
    {
        delete m_pD3D;
        m_pD3D = nullptr;
    }
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
    m_pD3D->ClearBuffer(0.5f, 0.5f, 0.5f, 1.f);

    m_pD3D->EndFrame();

    return true;
}
