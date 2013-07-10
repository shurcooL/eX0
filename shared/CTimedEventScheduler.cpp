#include "NetworkIncludes.h"
#include <GL/glfw.h>
#include <set>
#include <string>
using std::multiset;

#include "CTimedEventScheduler.h"

#include "CTimedEvent.h"
void eX0_assert(bool expression, std::string message = ""); // TODO: Create a centralized 'common stuff' file, and include it there instead

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
			&& glfwGetTime() >= (oEvent = *pScheduler->m_oEvents.begin()).GetTime())
		{
			pScheduler->m_oEvents.erase(pScheduler->m_oEvents.begin());
			oEvent.Execute();
			if (oEvent.m_dInterval > 0) {
				oEvent.SetTime(oEvent.GetTime() + oEvent.m_dInterval);
				pScheduler->m_oEvents.insert(oEvent);
			}
		}

		glfwUnlockMutex(pScheduler->m_oSchedulerMutex);

		// Sleep
		/*if (!pScheduler->m_oEvents.empty())
			glfwSleep(pScheduler->m_oEvents.top().GetTime() - glfwGetTime());
		else
			glfwSleep(0.0);*/
		glfwSleep(0.0001);
	}

	printf("Scheduler thread has ended.\n");
}

void CTimedEventScheduler::ScheduleEvent(CTimedEvent oEvent)
{
	eX0_assert(oEvent.GetId() != 0, "CTimedEventScheduler::ScheduleEvent: oEvent.GetId() != 0");
	if (oEvent.GetId() == 0) return;

	glfwLockMutex(m_oSchedulerMutex);

	// Add the event
	m_oEvents.insert(oEvent);

	glfwUnlockMutex(m_oSchedulerMutex);
}

bool CTimedEventScheduler::CheckEventById(u_int nId)
{
	if (nId == 0) return false;

	glfwLockMutex(m_oSchedulerMutex);

	for (multiset<CTimedEvent>::iterator it1 = m_oEvents.begin(); it1 != m_oEvents.end(); ++it1)
	{
		if (it1->GetId() == nId)
		{
			glfwUnlockMutex(m_oSchedulerMutex);
			return true;
		}
	}

	glfwUnlockMutex(m_oSchedulerMutex);

	return false;
}

void CTimedEventScheduler::RemoveEventById(u_int nId)
{
	if (nId == 0) return;

	glfwLockMutex(m_oSchedulerMutex);

	for (multiset<CTimedEvent>::iterator it1 = m_oEvents.begin(); it1 != m_oEvents.end(); ++it1)
	{
		if (it1->GetId() == nId)
		{
			m_oEvents.erase(it1);
			break;
		}
	}

	glfwUnlockMutex(m_oSchedulerMutex);
}

void CTimedEventScheduler::RemoveAllEvents()
{
	glfwLockMutex(m_oSchedulerMutex);

	m_oEvents.clear();

	glfwUnlockMutex(m_oSchedulerMutex);
}
