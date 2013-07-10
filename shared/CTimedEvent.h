#pragma once
#ifndef __CTimedEvent_H__
#define __CTimedEvent_H__

typedef void (*EventFunction_f)(void *);

class CTimedEvent
{
public:
	CTimedEvent();
	CTimedEvent(double dDelayTime, double dInterval, EventFunction_f pEventFunction, void * pArgument);
	~CTimedEvent();

	double GetTime() const;
	void SetTime(const double dTime);
	double GetInterval() const;
	u_int GetId() const;
	void SetId(const u_int nId);

	void Execute();

	bool operator <(const CTimedEvent &oOtherEvent) const;

private:
	u_int			m_nId;

	double			m_dTime;
	double			m_dInterval;
	EventFunction_f	m_pEventFunction;
	void			*m_pArgument;

	static u_int	m_nNextFreeId;
};

#endif // __CTimedEvent_H__
