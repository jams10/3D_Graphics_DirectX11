#include "FpsCalculator.h"

FpsCalculator::FpsCalculator()
{
}

void FpsCalculator::Initialize()
{
	m_fps = 0;
	m_count = 0;
	m_startTime = timeGetTime();
}

void FpsCalculator::Tick()
{
	m_count++;

	// 누적 시간이 1초를 넘어가면 프레임 개수와 시작 시간을 초기화 해줌.
	if (timeGetTime() >= (m_startTime + 1000))
	{
		m_fps = m_count;
		m_count = 0;

		m_startTime = timeGetTime();
	}
}

int FpsCalculator::GetFps()
{
    return m_fps;
}
