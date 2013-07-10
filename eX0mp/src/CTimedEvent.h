#ifndef __CTimedEvent_H__
#define __CTimedEvent_H__

typedef void (*EventFunction_f)(void *);

class CTimedEvent
{
public:
	CTimedEvent(void);
	CTimedEvent(double dTime, double dInterval, EventFunction_f pEventFunction, void *pArgument);
	~CTimedEvent(void);

	double GetTime(void) const;
	void SetTime(const double dTime);
	u_int GetId(void) const;
	void SetId(const u_int nId);

	void Execute(void);

	bool operator <(const CTimedEvent &oOtherEvent) const;

	double			m_dInterval;

private:
	u_int			m_nId;

	double			m_dTime;
	EventFunction_f	m_pEventFunction;
	void			*m_pArgument;

	static u_int	m_nNextFreeId;
};

#endif // __CTimedEvent_H__
