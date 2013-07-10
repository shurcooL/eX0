#include "globals.h"

CTimedEventScheduler	*pTimedEventScheduler = NULL;

CTimedEventScheduler::CTimedEventScheduler()
{
	printf("CTimedEventScheduler() Constructor started.\n");

	m_oSchedulerMutex = glfwCreateMutex();

	m_bSchedulerThreadRun = true;
	m_oSchedulerThread = glfwCreateThread(&CTimedEventScheduler::SchedulerThread, this);

	if (m_oSchedulerThread >= 0)
		printf("Scheduler thread created.\n");
	else
		printf("Couldn't create the scheduler thread.\n");
}

CTimedEventScheduler::~CTimedEventScheduler()
{
	if (m_oSchedulerThread >= 0)
	{
		m_bSchedulerThreadRun = false;

		glfwWaitThread(m_oSchedulerThread, GLFW_WAIT);
		//glfwDestroyThread(oSchedulerThread);
		m_oSchedulerThread = -1;

		printf("Scheduler thread has been destroyed.\n");
	}

	glfwDestroyMutex(m_oSchedulerMutex);

	printf("CTimedEventScheduler() ~Destructor done.\n");
}

void GLFWCALL CTimedEventScheduler::SchedulerThread(void *pArgument)
{
	CTimedEventScheduler *pScheduler = (CTimedEventScheduler *)pArgument;

	CTimedEvent oEvent;

	// Main scheduler loop
	while (pScheduler->m_bSchedulerThreadRun)
	{
		glfwLockMutex(pScheduler->m_oSchedulerMutex);

		while (!pScheduler->m_oEvents.empty()
			&& glfwGetTime() >= (oEvent = pScheduler->m_oEvents.top()).GetTime())
		{
			// Ignore the event if it was removed
			if (pScheduler->m_oRemovedEventIds.find(oEvent.GetId()) != pScheduler->m_oRemovedEventIds.end())
			{
				pScheduler->m_oEvents.pop();
				pScheduler->m_oRemovedEventIds.erase(oEvent.GetId());
			}
			else
			{
				pScheduler->m_oEvents.pop();
				oEvent.Execute();
				if (oEvent.m_dInterval > 0) {
					oEvent.SetTime(oEvent.GetTime() + oEvent.m_dInterval);
					pScheduler->m_oEvents.push(oEvent);
				}
			}
		}

		glfwUnlockMutex(pScheduler->m_oSchedulerMutex);

		// Sleep
		/*if (!pScheduler->m_oEvents.empty())
			glfwSleep(pScheduler->m_oEvents.top().GetTime() - glfwGetTime());
		else
			glfwSleep(0.0);*/
		glfwSleep(0.01);
	}

	printf("Scheduler thread has ended.\n");
}

void CTimedEventScheduler::ScheduleEvent(CTimedEvent oEvent)
{
	glfwLockMutex(m_oSchedulerMutex);

	// Add the event
	if (oEvent.GetId() != 0)
		m_oEvents.push(oEvent);

	glfwUnlockMutex(m_oSchedulerMutex);
}

void CTimedEventScheduler::RemoveEventById(u_int nId)
{
	glfwLockMutex(m_oSchedulerMutex);

	if (nId != 0)
		m_oRemovedEventIds.insert(nId);

	glfwUnlockMutex(m_oSchedulerMutex);
}
