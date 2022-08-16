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

	// ���� �ð��� 1�ʸ� �Ѿ�� ������ ������ ���� �ð��� �ʱ�ȭ ����.
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
