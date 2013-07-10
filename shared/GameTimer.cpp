// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

GameTimer::GameTimer()
	: m_dTimeOffset(0),
	  m_dCurrentTime(0),
	  m_dTimePassed(0)
{
}

GameTimer::~GameTimer()
{
}

double GameTimer::GetTime()
{
	return m_dCurrentTime;
}

double GameTimer::GetTimePassed()
{
	return m_dTimePassed;
}

double GameTimer::GetRealTime()
{
	return glfwGetTime() + m_dTimeOffset;
}

void GameTimer::UpdateTime()
{
	double dNewCurrentTime = glfwGetTime() + m_dTimeOffset;
	m_dTimePassed = dNewCurrentTime - m_dCurrentTime;
	m_dCurrentTime = dNewCurrentTime;
}

void GameTimer::Start()
{
	SetTime(0);
}

void GameTimer::SetTime(double dNewCurrentTime)
{
	m_dTimeOffset = dNewCurrentTime - glfwGetTime();
	m_dCurrentTime = dNewCurrentTime;
	m_dTimePassed = 0;
}
