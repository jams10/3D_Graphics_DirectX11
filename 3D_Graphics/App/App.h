#pragma once

#include <Windows/WindowsHeaders.h>
#include <Windows/Window.h>
#include <Graphics/Graphics.h>
#include <string>

class Application
{
public:
	Application(std::wstring title, int screenWidth, int screenHeight);
	~Application();

	bool Initialize();
	int Run();
	void ShutDown();

private:
	void Frame();

private:
	Window m_Wnd;
	Graphics* m_pGfx;
	int screenWidth;
	int screenHeight;
};