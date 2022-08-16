#include "CpuUsageCalculator.h"

#pragma comment(lib, "pdh.lib")

CpuUsageCalculator::CpuUsageCalculator()
{
}

void CpuUsageCalculator::Initialize()
{
	PDH_STATUS status;

	// 시스템 cpu 사용량을 읽어들일 수 있는 지 여부를 나타내는 플래그를 초기화함.
	m_canReadCpu = true;

	// 시스템 사용량 체크를 위한 쿼리 객체를 하나 만들어줌.
	status = PdhOpenQuery(NULL, 0, &m_queryHandle);
	if (status != ERROR_SUCCESS) // 쿼리 실패 = CPU 사용량을 읽을 수 없는 것이므로 플래그를 false로 설정.
	{
		m_canReadCpu = false;
	}

	// 시스템에 있는 모든 cpu들을 폴링하는 쿼리 객체를 하나 만들어줌.
	status = PdhAddCounter(m_queryHandle, TEXT("\\Processor(_Total)\\% processor time"), 0, &m_counterHandle);
	if (status != ERROR_SUCCESS)
	{
		m_canReadCpu = false;
	}

	m_lastSampleTime = GetTickCount();

	m_cpuUsage = 0;

	return;
}

void CpuUsageCalculator::Shutdown()
{
	if (m_canReadCpu)
	{
		PdhCloseQuery(m_queryHandle);
	}

	return;
}

void CpuUsageCalculator::Tick()
{
	PDH_FMT_COUNTERVALUE value;

	if (m_canReadCpu)
	{
		// 매 Tick 마다 CPU 사용량을 샘플링 하는 것은 무리가 있으므로, 1초마다 호출해 CPU 사용량을 저장함.
		if ((m_lastSampleTime + 1000) < GetTickCount())
		{
			m_lastSampleTime = GetTickCount();

			PdhCollectQueryData(m_queryHandle);

			PdhGetFormattedCounterValue(m_counterHandle, PDH_FMT_LONG, NULL, &value);

			m_cpuUsage = value.longValue;
		}
	}

	return;
}

int CpuUsageCalculator::GetCpuPercentage()
{
	int usage;

	// 쿼리 객체를 통해 CPU 값을 읽을 수 있을 때만 그 사용량을 리턴해줌.
	if (m_canReadCpu)
	{
		usage = (int)m_cpuUsage;
	}
	else
	{
		usage = 0;
	}

	return usage;
}
