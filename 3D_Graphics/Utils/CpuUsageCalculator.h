#pragma once

#include <pdh.h>

class CpuUsageCalculator
{
public:
	CpuUsageCalculator();

	void Initialize();
	void Shutdown();
	void Tick();
	int GetCpuPercentage();

private:
	bool m_canReadCpu;
	HQUERY m_queryHandle;
	HCOUNTER m_counterHandle;
	unsigned long m_lastSampleTime;
	long m_cpuUsage;
};