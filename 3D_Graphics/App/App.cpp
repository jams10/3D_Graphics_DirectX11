#include "App.h"

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

	return true;
}

int Application::Run()
{
	while (true)
	{
		if (const auto ecode = Window::ProcessMessages())
		{
			// ProcessMessages() �Լ��� ���� ���� �� �ִ� optional ��ü�� �޴� ���,
			// ������ ������ �ǹ��ϹǷ� exit code�� ���� ����.
			return *ecode; // optional ��ü�� ��� �ִ� ���� ������ �� *�����ڸ� ���.
		}
		Frame();
	}
}

void Application::ShutDown()
{
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

	m_pGfx->Frame(&m_Wnd.kbd, dt);
}