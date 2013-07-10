#pragma once
#ifndef __GameSession_H__
#define __GameSession_H__

class GameSession
{
public:
	GameSession();
	~GameSession();

	GameTimer & LogicTimer(void);
	GameTimer & RenderTimer(void);

private:
	GameSession(const GameSession &);
	GameSession & operator =(const GameSession &);

	GameTimer	m_oGameTimer;
	GameTimer	m_oRenderTimer;
};

#endif // __GameSession_H__
