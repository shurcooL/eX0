// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

std::list<FpsCounter *> FpsCounter::m_oCounters;
double FpsCounter::m_dLastTime = 0;
double FpsCounter::m_dTimePassed = 0;
double FpsCounter::m_dPrintTimePassed = m_kdPrintUpdateRate - 1;
const double FpsCounter::m_kdUpdateRate = 0.75;
const double FpsCounter::m_kdPrintUpdateRate = 100;

FpsCounter::FpsCounter(std::string sName)
	: m_sName(sName),
	  m_nCounter(0),
	  m_dFpsRate(0),
	  m_bRemove(false)
{
	m_oCounters.push_back(this);
}

FpsCounter::~FpsCounter()
{
}

std::string FpsCounter::GetFpsString()
{
	return m_sName + ": " + to_string<double>(m_dFpsRate, 4) + " fps";
}

void FpsCounter::CalculateFps(double dTimePassed)
{
	m_dFpsRate = static_cast<double>(GetCounter()) / dTimePassed;
	m_nCounter = 0;
}

FpsCounter * FpsCounter::CreateCounter(std::string sName)
{
	///eX0_assert(this is done in main thread);

	return new FpsCounter(sName);
}

void FpsCounter::RemoveCounter(FpsCounter * pCounter)
{
	pCounter->m_bRemove = true;
}

void FpsCounter::UpdateCounters(double dCurrentTime)
{
	///eX0_assert(this is done in main thread);

	double dTimePassed = dCurrentTime - m_dLastTime;
	if (dTimePassed < 0) dTimePassed = 1;

	m_dLastTime = dCurrentTime;

	m_dTimePassed += dTimePassed;
	m_dPrintTimePassed += dTimePassed;

	if (m_dTimePassed >= m_kdUpdateRate)
	{
		for (std::list<FpsCounter *>::iterator it1 = m_oCounters.begin(); it1 != m_oCounters.end(); )
		{
			if ((*it1)->m_bRemove == false)
			{
				(*it1)->CalculateFps(m_dTimePassed);
				++it1;
			} else {
				printf("Counter '%s' was removed nicely!\n", (*it1)->m_sName.c_str());
				delete (*it1);
				it1 = m_oCounters.erase(it1);
			}
		}

		m_dTimePassed = 0;
	}
}

std::list<FpsCounter *> FpsCounter::GetCounters()
{
	///eX0_assert(this is done in main thread);

	return m_oCounters;
}

void FpsCounter::PrintCounters()
{
	///eX0_assert(this is done in main thread);

	while (m_dPrintTimePassed >= m_kdPrintUpdateRate)
	{
		printf("---\n");
		for (std::list<FpsCounter *>::iterator it1 = m_oCounters.begin(); it1 != m_oCounters.end(); ++it1)
		{
			printf("%s\n", (*it1)->GetFpsString().c_str());
		}

		m_dPrintTimePassed -= m_kdPrintUpdateRate;
	}
}

void FpsCounter::DeleteAll()
{
	///eX0_assert(this is done in main thread);

	while (!m_oCounters.empty()) {
		printf("Removing a counter '%s'.\n", (*m_oCounters.begin())->m_sName.c_str());
		delete *(m_oCounters.begin());
		m_oCounters.erase(m_oCounters.begin());
	}
}
