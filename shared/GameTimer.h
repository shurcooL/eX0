#pragma once
#ifndef __GameTimer_H__
#define __GameTimer_H__

class GameTimer
{
public:
	GameTimer();
	~GameTimer();

	double GetTime(void);
	double GetTimePassed(void);
	double GetRealTime(void);
	void UpdateTime(void);

	void Start(void);
	void SetTime(double dNewCurrentTime);

private:
	GameTimer(const GameTimer &);
	GameTimer & operator =(const GameTimer &);

	double		m_dTimeOffset;

	double		m_dCurrentTime;
	double		m_dTimePassed;
};

#endif // __GameTimer_H__
