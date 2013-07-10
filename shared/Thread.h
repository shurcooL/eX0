#pragma once
#ifndef __Thread_H__
#define __Thread_H__

class Thread
{
public:
	Thread(GLFWthreadfun oFunction, void * pArgument, string sName);
	~Thread();

	inline bool ShouldBeRunning() { return m_bShouldBeRunning; }
	inline void RequestStop() { m_bShouldBeRunning = false; }
	void ThreadEnded();

private:
	volatile GLFWthread	m_oThread;
	volatile bool		m_bShouldBeRunning;
	string				m_sName;
};

#endif // __Thread_H__
