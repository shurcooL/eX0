#pragma once
#ifndef __CTimedEventScheduler_H__
#define __CTimedEventScheduler_H__

class CTimedEvent;

class CTimedEventScheduler
{
public:
	CTimedEventScheduler();
	~CTimedEventScheduler();

	void ScheduleEvent(CTimedEvent oEvent);
	bool CheckEventById(u_int nId);
	void RemoveEventById(u_int nId);
	void RemoveAllEvents();

private:
	std::multiset<CTimedEvent>	m_oEvents;

public:		// DEBUG: Hack, use friend or something
	GameTimer		m_oTimer;
private:

	GLFWmutex		m_oSchedulerMutex;

	Thread *		m_pThread;

	static void GLFWCALL ThreadFunction(void * pArgument);
};

#endif // __CTimedEventScheduler_H__
