#pragma once
#ifndef __Thread_H__
#define __Thread_H__

class Thread
{
public:
	Thread(GLFWthreadfun pFunction, void * pArgument, std::string sName);
	~Thread();

	inline bool ShouldBeRunning(void) { return m_bShouldBeRunning; }
	inline void RequestStop(void) { m_bShouldBeRunning = false; }
	inline bool IsAlive(void) { return m_oThread >= 0 && !m_bThreadEnded; }
	void ThreadEnded(void);
void *				m_pArguments[2];

	FpsCounter * GetFpsCounter(void);

	static Thread * GetThisThreadAndRevertArgument(void *& pArgument);

private:
	Thread(const Thread &);
	Thread & operator =(const Thread &);

	volatile bool		m_bCanStart;
	volatile bool		m_bShouldBeRunning;
	volatile bool		m_bThreadEnded;
	std::string			m_sName;
	FpsCounter *		m_pFpsCounter;
	//void *				m_pArguments[2];
	volatile GLFWthread	m_oThread;
};

#endif // __Thread_H__
