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
#include "FpsCounter.h"
#include "Thread.h"
#include "GameTimer.h"
#include "CTimedEventScheduler.h"

#include "CTimedEvent.h"

extern CTimedEventScheduler * pTimedEventScheduler;

void eX0_assert(bool expression, std::string message = "", bool fatal = false); // TODO: Create a centralized 'common stuff' file, and include it there instead

u_int CTimedEvent::m_nNextFreeId = 1;

CTimedEvent::CTimedEvent()
{
	m_nId = 0;

	m_dTime = 0.0;
	m_dInterval = -1.0;
	m_pEventFunction = NULL;
	m_pArgument = NULL;
}

CTimedEvent::CTimedEvent(double dDelayTime, double dInterval, EventFunction_f pEventFunction, void * pArgument)
{
	m_nId = m_nNextFreeId++;

	m_dTime = pTimedEventScheduler->m_oTimer.GetRealTime() + dDelayTime;
	m_dInterval = dInterval;
	m_pEventFunction = pEventFunction;
	m_pArgument = pArgument;
}

CTimedEvent::~CTimedEvent()
{
}

double CTimedEvent::GetTime() const { return m_dTime; }
void CTimedEvent::SetTime(const double dTime) { m_dTime = dTime; }

u_int CTimedEvent::GetId() const { return m_nId; }
void CTimedEvent::SetId(const u_int nId) { m_nId = nId; }

void CTimedEvent::Execute()
{
	eX0_assert(GetId() != 0, "CTimedEvent::Execute: GetId() != 0"); // TODO: Create a global EX0_DEBUG define, and only call assert when it's on, as to increase performance otherwise

	(*m_pEventFunction)(m_pArgument);
}

bool CTimedEvent::operator <(const CTimedEvent &oOtherEvent) const
{
	return GetTime() < oOtherEvent.GetTime();
}
