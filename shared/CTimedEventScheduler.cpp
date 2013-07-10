#include "NetworkIncludes.h"
#include <GL/glfw.h>
#include <set>
#include <list>
#include <string>

/*#ifdef EX0_CLIENT
#	include "../eX0mp/src/mmgr/mmgr.h"
#else
#	include "../eX0ds/src/mmgr/mmgr.h"
#endif // EX0_CLIENT*/
#include "../shared/FpsCounter.h"
#include "../shared/Thread.h"
#include "../shared/GameTimer.h"
#include "CTimedEvent.h"

#include "CTimedEventScheduler.h"

void eX0_assert(bool expression, std::string message = "", bool fatal = false); // TODO: Create a centralized 'common stuff' file, and include it there instead

CTimedEventScheduler * pTimedEventScheduler = NULL;

CTimedEventScheduler::CTimedEventScheduler()
	: m_oEvents(),
	  m_oTimer(),
	  m_oSchedulerMutex(glfwCreateMutex())
{
	m_oTimer.Start();

	m_pThread = new Thread(&CTimedEventScheduler::ThreadFunction, this, "Scheduler");
}

CTimedEventScheduler::~CTimedEventScheduler()
{
	delete m_pThread;

	glfwDestroyMutex(m_oSchedulerMutex);
}

void GLFWCALL CTimedEventScheduler::ThreadFunction(void * pArgument)
{
	Thread * pThread = Thread::GetThisThreadAndRevertArgument(pArgument);
	FpsCounter * pFpsCounter = pThread->GetFpsCounter();

	CTimedEventScheduler * pScheduler = static_cast<CTimedEventScheduler *>(pArgument);

	CTimedEvent oEvent;

	// Main scheduler loop
	while (pThread->ShouldBeRunning())
	{
		pFpsCounter->IncrementCounter();

		glfwLockMutex(pScheduler->m_oSchedulerMutex);

		while (!pScheduler->m_oEvents.empty()
			&& pScheduler->m_oTimer.GetRealTime() >= (oEvent = *pScheduler->m_oEvents.begin()).GetTime())
		{
			pScheduler->m_oEvents.erase(pScheduler->m_oEvents.begin());
			oEvent.Execute();
			if (oEvent.GetInterval() > 0) {
				oEvent.SetTime(oEvent.GetTime() + oEvent.GetInterval());
				pScheduler->m_oEvents.insert(oEvent);
			}
		}

		glfwUnlockMutex(pScheduler->m_oSchedulerMutex);

		// Sleep
		/*if (!pScheduler->m_oEvents.empty())
			glfwSleep(pScheduler->m_oEvents.top().GetTime() - pScheduler->m_oTimer.GetRealTime());
		else
			glfwSleep(0.0);*/
		glfwSleep(0.0001);
	}

	//printf("Scheduler thread has ended.\n");
	pThread->ThreadEnded();
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

	for (std::multiset<CTimedEvent>::iterator it1 = m_oEvents.begin(); it1 != m_oEvents.end(); ++it1)
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

	for (std::multiset<CTimedEvent>::iterator it1 = m_oEvents.begin(); it1 != m_oEvents.end(); ++it1)
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
