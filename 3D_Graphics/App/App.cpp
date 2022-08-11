#include "App.h"
#include <Objects/Camera.h>

Application::Application(std::wstring title, int screenWidth, int screenHeight)
	:
	screenWidth(screenWidth),
	screenHeight(screenHeight),
	m_Wnd(title.c_str(), screenWidth, screenHeight)
{
}

Application::~Application()
{
}

bool Application::Initialize()
{
	m_pGfx = new Graphics();

	if (!m_pGfx->Initialize(screenWidth, screenHeight, m_Wnd.GetWindowHandle()))
	{
		// Cannot initialize the graphics instance.
		return false;
	}

	m_pSound = new DXSound();
	if (m_pSound)
	{
		m_pSound->Initialize(m_Wnd.GetWindowHandle());
	}

	return true;
}

int Application::Run()
{
	while (true)
	{
		if (const auto ecode = Window::ProcessMessages())
		{
			// ProcessMessages() 함수를 통해 값이 들어가 있는 optional 객체를 받는 경우,
			// 종료할 것임을 의미하므로 exit code를 리턴 해줌.
			return *ecode; // optional 객체가 들고 있는 값에 접근할 때 *연산자를 사용.
		}
		Frame();
	}
}

void Application::ShutDown()
{
	if (m_pSound)
	{
		m_pSound->Shutdown();
		delete m_pSound;
		m_pSound = nullptr;
	}
	if (m_pGfx)
	{
		m_pGfx->Shutdown();
		delete m_pGfx;
		m_pGfx = nullptr;
	}
}

void Application::Frame()
{
	timer.Tick();
	const auto dt = timer.GetDeltaTime();

	while (const auto e = m_Wnd.kbd.ReadKey())
	{
		if (!e->IsPress())
		{
			continue;
		}

		switch (e->GetCode())
		{
		case VK_TAB:
			if (m_Wnd.CursorEnabled())
			{
				m_Wnd.DisableCursor();
				m_Wnd.mouse.EnableRaw();
			}
			else
			{
				m_Wnd.EnableCursor();
				m_Wnd.mouse.DisableRaw();
			}
			break;
		}
	}

#pragma region CameraMove
	if (!m_Wnd.CursorEnabled())
	{
		if (m_Wnd.kbd.KeyIsPressed('W'))
		{
			m_pGfx->GetCamera()->Translate({ 0.0f,0.0f,dt });
		}
		if (m_Wnd.kbd.KeyIsPressed('A'))
		{
			m_pGfx->GetCamera()->Translate({ -dt,0.0f,0.0f });
		}
		if (m_Wnd.kbd.KeyIsPressed('S'))
		{
			m_pGfx->GetCamera()->Translate({ 0.0f,0.0f,-dt });
		}
		if (m_Wnd.kbd.KeyIsPressed('D'))
		{
			m_pGfx->GetCamera()->Translate({ dt,0.0f,0.0f });
		}
		if (m_Wnd.kbd.KeyIsPressed('R'))
		{
			m_pGfx->GetCamera()->Translate({ 0.0f,dt,0.0f });
		}
		if (m_Wnd.kbd.KeyIsPressed('F'))
		{
			m_pGfx->GetCamera()->Translate({ 0.0f,-dt,0.0f });
		}
	}

	while (const auto delta = m_Wnd.mouse.ReadRawDelta())
	{
		if (!m_Wnd.CursorEnabled())
		{
			m_pGfx->GetCamera()->Rotate(delta->x, delta->y);
		}
	}
#pragma endregion

	m_pGfx->Frame(m_pSound);
}