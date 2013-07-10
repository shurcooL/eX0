#pragma once
#ifndef __GameLogicThread_H__
#define __GameLogicThread_H__

class GameLogicThread
{
public:
	GameLogicThread();
	~GameLogicThread();

private:
	Thread *		m_pThread;

	static void GLFWCALL ThreadFunction(void * pArgument);
};

#endif // __GameLogicThread_H__
