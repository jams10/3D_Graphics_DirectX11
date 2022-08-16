#include "CpuUsageCalculator.h"

#pragma comment(lib, "pdh.lib")

CpuUsageCalculator::CpuUsageCalculator()
{
}

void CpuUsageCalculator::Initialize()
{
	PDH_STATUS status;

	// �ý��� cpu ��뷮�� �о���� �� �ִ� �� ���θ� ��Ÿ���� �÷��׸� �ʱ�ȭ��.
	m_canReadCpu = true;

	// �ý��� ��뷮 üũ�� ���� ���� ��ü�� �ϳ� �������.
	status = PdhOpenQuery(NULL, 0, &m_queryHandle);
	if (status != ERROR_SUCCESS) // ���� ���� = CPU ��뷮�� ���� �� ���� ���̹Ƿ� �÷��׸� false�� ����.
	{
		m_canReadCpu = false;
	}

	// �ý��ۿ� �ִ� ��� cpu���� �����ϴ� ���� ��ü�� �ϳ� �������.
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
		// �� Tick ���� CPU ��뷮�� ���ø� �ϴ� ���� ������ �����Ƿ�, 1�ʸ��� ȣ���� CPU ��뷮�� ������.
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

	// ���� ��ü�� ���� CPU ���� ���� �� ���� ���� �� ��뷮�� ��������.
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
