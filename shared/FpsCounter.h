#pragma once
#ifndef __FpsCounter_H__
#define __FpsCounter_H__

class FpsCounter
{
public:
	inline void IncrementCounter(void) { ++m_nCounter; }
	std::string GetFpsString(void);

	static FpsCounter * CreateCounter(std::string sName);
	static void RemoveCounter(FpsCounter * pCounter);

	static void UpdateCounters(double dTimePassed);
	static std::list<FpsCounter *> GetCounters(void);
	static void PrintCounters(void);

	static void DeleteAll();

private:
	FpsCounter(std::string sName);
	FpsCounter(const FpsCounter &);
	FpsCounter & operator =(const FpsCounter &);
	~FpsCounter();

	std::string		m_sName;
	u_int			m_nCounter;
	double			m_dFpsRate;
	volatile bool	m_bRemove;

	void CalculateFps(double dTimePassed);
	inline u_int GetCounter(void) { return m_nCounter; }

	static std::list<FpsCounter *>		m_oCounters;
	static double		m_dLastTime;
	static double		m_dTimePassed;
	static double		m_dPrintTimePassed;
	static const double m_kdUpdateRate;
	static const double m_kdPrintUpdateRate;
};

#endif // __FpsCounter_H__
