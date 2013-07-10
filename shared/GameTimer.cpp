// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

GameTimer::GameTimer()
	: m_dTimeOffset(0),
	  m_dCurrentTime(0),
	  m_dTimePassed(0),
	  m_dSyncTimeOffset(0),
	  m_bSyncTime(false),
	  m_bIsStarted(false),
	  m_oSyncMutex(glfwCreateMutex()),
	  m_pSyncOtherTimer(nullptr)
{
}

GameTimer::~GameTimer()
{
	glfwDestroyMutex(m_oSyncMutex);
}

double GameTimer::GetTime()
{
	eX0_assert(m_bIsStarted, "GetTime() called on an unstarted timer");

	return m_dCurrentTime;
}

double GameTimer::GetTimePassed()
{
	eX0_assert(m_bIsStarted, "GetTimePassed() called on an unstarted timer");

	return m_dTimePassed;
}

double GameTimer::GetRealTime()
{
	eX0_assert(m_bIsStarted, "GetRealTime() called on an unstarted timer");

	return (glfwGetTime() + m_dTimeOffset);
}

void GameTimer::UpdateTime()
{
	eX0_assert(m_bIsStarted);

	if (false == m_bSyncTime)
	{
		double dNewCurrentTime = glfwGetTime() + m_dTimeOffset;
		m_dTimePassed = dNewCurrentTime - m_dCurrentTime;
		m_dCurrentTime = dNewCurrentTime;
	}
	else
	{
		glfwLockMutex(m_oSyncMutex);

		m_dTimePassed = (glfwGetTime() + m_dTimeOffset) - m_dCurrentTime;
		m_dTimeOffset = m_dSyncTimeOffset;
		m_dCurrentTime = glfwGetTime() + m_dTimeOffset;

		m_bSyncTime = false;

		glfwUnlockMutex(m_oSyncMutex);
	}
}

void GameTimer::Start()
{
	eX0_assert(!m_bIsStarted);

	m_bIsStarted = true;
	if (false == m_bSyncTime)
		SetTime(0);
	else
		UpdateTime();
}

bool GameTimer::IsStarted()
{
	return m_bIsStarted;
}

void GameTimer::SetTime(double dNewCurrentTime)
{
	eX0_assert(m_bIsStarted);

	m_dTimeOffset = dNewCurrentTime - glfwGetTime();
	m_dCurrentTime = dNewCurrentTime;
	m_dTimePassed = 0;

	SyncOtherTimer();
}

// Starts syncing the other timer to this one
void GameTimer::StartSyncingTimer(GameTimer * pTimer)
{
	eX0_assert(nullptr != pTimer);
	eX0_assert(this != pTimer);
	eX0_assert(nullptr == m_pSyncOtherTimer);

	m_pSyncOtherTimer = pTimer;

	SyncOtherTimer();
}

void GameTimer::StopSyncingTimer(GameTimer * pTimer)
{
	eX0_assert(nullptr != m_pSyncOtherTimer);

	if (pTimer == m_pSyncOtherTimer)
		m_pSyncOtherTimer = nullptr;
}

// Syncs the other timer to this one
void GameTimer::SyncOtherTimer()
{
	if (nullptr != m_pSyncOtherTimer)
	{
		glfwLockMutex(m_pSyncOtherTimer->m_oSyncMutex);

		m_pSyncOtherTimer->m_dSyncTimeOffset = this->m_dTimeOffset;
		m_pSyncOtherTimer->m_bSyncTime = true;

		glfwUnlockMutex(m_pSyncOtherTimer->m_oSyncMutex);
	}
}
