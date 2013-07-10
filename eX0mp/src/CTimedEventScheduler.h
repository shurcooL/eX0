#ifndef __CTimedEventScheduler_H__
#define __CTimedEventScheduler_H__

class CTimedEventScheduler
{
public:
	CTimedEventScheduler(void);
	~CTimedEventScheduler(void);

	void ScheduleEvent(CTimedEvent oEvent);
	void RemoveEventById(u_int nId);

private:
	GLFWthread		m_oSchedulerThread;
	volatile bool	m_bSchedulerThreadRun;

	GLFWmutex		m_oSchedulerMutex;

	priority_queue<CTimedEvent>	m_oEvents;
	set<u_int>					m_oRemovedEventIds;

	static void GLFWCALL SchedulerThread(void *pArgument);
};

#endif // __CTimedEventScheduler_H__
