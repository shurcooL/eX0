#pragma once
#ifndef __ThroughputMonitor_H__
#define __ThroughputMonitor_H__

class ThroughputMonitor
{
public:
	ThroughputMonitor();
	~ThroughputMonitor();

	void PushSentData(uint32 nBytes);
	void PushReceivedData(uint32 nBytes);

	double GetSentTraffic();
	double GetReceivedTraffic();

private:
	ThroughputMonitor(const ThroughputMonitor &);
	ThroughputMonitor & operator =(const ThroughputMonitor &);

	const double	m_kdTimeSpan;			// Amount of time to cover, in seconds

	MovingAverage	m_oSentTraffic;
	MovingAverage	m_oReceivedTraffic;

	GLFWmutex	m_oAccessMutex;
};

#endif // __ThroughputMonitor_H__
