#pragma once

#include <Windows/WindowsHeaders.h>
#include <Windows/Window.h>
#include <Graphics/Graphics.h>
#include <Sound/DXSound.h>
#include <Utils/FpsCalculator.h>
#include <Utils/CpuUsageCalculator.h>
#include <Utils/GameTimer.h>
#include <Utils/ImguiManager.h>
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
	ImguiManager imgui;
	Window m_Wnd;
	Graphics* m_pGfx;
	DXSound* m_pSound;
	FpsCalculator fpsCounter;
	CpuUsageCalculator cpuCounter;
	GameTimer timer;
	int screenWidth;
	int screenHeight;
};