#pragma once
#ifndef __GameLogicThread_H__
#define __GameLogicThread_H__

class GameLogicThread
{
public:
	GameLogicThread();
	~GameLogicThread();

private:
	GLFWthread		m_oThread;
	volatile bool	m_bThreadRun;

	static void GLFWCALL Thread(void * pArgument);
};

#endif // __GameLogicThread_H__
