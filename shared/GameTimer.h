#pragma once
#ifndef __GameTimer_H__
#define __GameTimer_H__

class GameTimer
{
public:
	GameTimer();
	~GameTimer();

	double GetTime();
	double GetTimePassed();
	double GetRealTime();
	void UpdateTime();

	void Start();
	bool IsStarted();
	void SetTime(double dNewCurrentTime);

	void StartSyncingTimer(GameTimer * pTimer);
	void StopSyncingTimer(GameTimer * pTimer);

private:
	GameTimer(const GameTimer &);
	GameTimer & operator =(const GameTimer &);

	void SyncOtherTimer();

	double			m_dTimeOffset;

	double			m_dCurrentTime;
	double			m_dTimePassed;

	volatile double		m_dSyncTimeOffset;
	volatile bool		m_bSyncTime;

	bool			m_bIsStarted;
	GLFWmutex		m_oSyncMutex;
	GameTimer *		m_pSyncOtherTimer;
};

#endif // __GameTimer_H__
