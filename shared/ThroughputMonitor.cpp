// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

ThroughputMonitor::ThroughputMonitor()
	: m_kdTimeSpan(1.0),
	  m_oSentTraffic(m_kdTimeSpan, 0),
	  m_oReceivedTraffic(m_kdTimeSpan, 0),
	  m_oAccessMutex(glfwCreateMutex())		// DEBUG: See if it's possible (and worth it) to rewrite this in a lock-free way
{
}

ThroughputMonitor::~ThroughputMonitor()
{
	glfwDestroyMutex(m_oAccessMutex);
}

void ThroughputMonitor::PushSentData(uint32 nBytes)
{
	glfwLockMutex(m_oAccessMutex);

	// DEBUG: See if glfwGetTime() should be replaced by some Timer
	m_oSentTraffic.push(nBytes, glfwGetTime());

	glfwUnlockMutex(m_oAccessMutex);
}

void ThroughputMonitor::PushReceivedData(uint32 nBytes)
{
	glfwLockMutex(m_oAccessMutex);

	// DEBUG: See if glfwGetTime() should be replaced by some Timer
	m_oReceivedTraffic.push(nBytes, glfwGetTime());

	glfwUnlockMutex(m_oAccessMutex);
}

double ThroughputMonitor::GetSentTraffic()
{
	glfwLockMutex(m_oAccessMutex);

	// DEBUG: See if glfwGetTime() should be replaced by some Timer
	m_oSentTraffic.trim(glfwGetTime());
	double dSentTraffic = m_oSentTraffic.Sum();

	glfwUnlockMutex(m_oAccessMutex);

	return dSentTraffic;
}

double ThroughputMonitor::GetReceivedTraffic()
{
	glfwLockMutex(m_oAccessMutex);

	// DEBUG: See if glfwGetTime() should be replaced by some Timer
	m_oReceivedTraffic.trim(glfwGetTime());
	double dReceivedTraffic = m_oReceivedTraffic.Sum();

	glfwUnlockMutex(m_oAccessMutex);

	return dReceivedTraffic;
}
