#pragma once
#ifndef __GameSession_H__
#define __GameSession_H__

class GameSession
{
public:
	GameSession();
	~GameSession();

	GameTimer & MainTimer();
	GameTimer & LogicTimer();

	ThroughputMonitor * GetNetworkMonitor();

	u_char		GlobalStateSequenceNumberTEST;

private:
	GameSession(const GameSession &);
	GameSession & operator =(const GameSession &);

	GameTimer	m_oMainTimer;		// Main thread game timer
	GameTimer	m_oLogicTimer;		// Logic thread game timer

	ThroughputMonitor	*m_pNetworkMonitor;
};

#endif // __GameSession_H__
