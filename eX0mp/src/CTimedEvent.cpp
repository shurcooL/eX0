#include "globals.h"

u_int CTimedEvent::m_nNextFreeId = 1;

CTimedEvent::CTimedEvent()
{
	m_nId = 0;

	m_dTime = 0.0;
	m_dInterval = -1.0;
	m_pEventFunction = NULL;
	m_pArgument = NULL;
}

CTimedEvent::CTimedEvent(double dTime, double dInterval, EventFunction_f pEventFunction, void *pArgument)
{
	m_nId = m_nNextFreeId++;

	m_dTime = dTime;
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
	(*m_pEventFunction)(m_pArgument);
}

bool CTimedEvent::operator <(const CTimedEvent &oOtherEvent) const
{
	return GetTime() > oOtherEvent.GetTime();
}
