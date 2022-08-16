#pragma once

#include <Windows/WindowsHeaders.h>
#include <mmsystem.h>

class FpsCalculator
{
public:
	FpsCalculator();

	void Initialize();
	void Tick();
	int GetFps();

private:
	int m_fps, m_count;
	unsigned long m_startTime;
};