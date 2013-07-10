#include <set>
#include <list>
#include <stdio.h>
#include "NetworkIncludes.h"

/*#ifdef EX0_CLIENT
#	include "mmgr/mmgr.h"
#else
#	include "../eX0ds/src/mmgr/mmgr.h"
#endif // EX0_CLIENT*/

#include "MovingAverage.h"

MovingAverage::MovingAverage(double dTimeSpan, u_int nMinSize)
	: m_kdTimeSpan(dTimeSpan),
	  m_knMinSize(nMinSize),
	  m_oValues(),
	  m_oPointersByTime(),
	  m_nLastPositiveValueIndex(0),
	  m_nLastNegativeValueIndex(0)
{
}

MovingAverage::~MovingAverage()
{
}

void MovingAverage::push(double dValue, double dTime)
{
	// Insert the given value
	std::multiset<double>::iterator oPointerToValue;
	oPointerToValue = m_oValues.insert(dValue);

	// And keep track of it in a list sorted by time (end of list contains the latest entries)
	std::list<std::pair<double, std::multiset<double>::iterator> >::reverse_iterator rit1;
	u_int nInsertedIndex = 0;
	for (rit1 = m_oPointersByTime.rbegin(); rit1 != m_oPointersByTime.rend() && dTime < rit1->first; ++rit1) { ++nInsertedIndex; }
	m_oPointersByTime.insert(rit1.base(), std::pair<double, std::multiset<double>::iterator>(dTime, oPointerToValue));

	// This has to occur at the end, in case the newly added item is actually older than all current ones - meaning it should be the one to get removed
	trim(dTime);

	// Keep track of the latest positive and negative values
	if (nInsertedIndex <= m_nLastPositiveValueIndex) m_nLastPositiveValueIndex = std::min<u_int>(m_nLastPositiveValueIndex + 1, size());
	if (nInsertedIndex <= m_nLastNegativeValueIndex) m_nLastNegativeValueIndex = std::min<u_int>(m_nLastNegativeValueIndex + 1, size());
	if (dValue > 0.0) m_nLastPositiveValueIndex = std::min<u_int>(nInsertedIndex, m_nLastPositiveValueIndex);
	if (dValue < 0.0) m_nLastNegativeValueIndex = std::min<u_int>(nInsertedIndex, m_nLastNegativeValueIndex);
}

// Removes oldest entries if they are out of the time period covered, and our size is larger than MinSize
void MovingAverage::trim(double dTime)
{
	while (size() > m_knMinSize && m_oPointersByTime.front().first < (dTime - m_kdTimeSpan)) {
		m_oValues.erase(m_oPointersByTime.front().second);
		m_oPointersByTime.pop_front();
	}
}

void MovingAverage::clear()
{
	m_oValues.clear();
	m_oPointersByTime.clear();
}

inline
u_int MovingAverage::size() const
{
	return m_oValues.size();
}

// Returns true if there are at least MinSize elements added already, and it's not zero
bool MovingAverage::well_populated() const
{
	return size() >= m_knMinSize && size() > 0;
}

double MovingAverage::Sum() const
{
	double dSum = 0;

	for (std::multiset<double>::const_iterator it1 = m_oValues.begin(); it1 != m_oValues.end(); ++it1) {
		dSum += *it1;
	}

	return dSum;
}

double MovingAverage::Mean() const
{
	if (!well_populated()) return 0;

	return Sum() / m_oValues.size();
}

double MovingAverage::LowerQuartile() const
{
	if (!well_populated()) return 0;

	u_int nLowerHalfSize = size() / 2;
	u_int nLowerHalfMedianIndex = nLowerHalfSize / 2;

	if ((nLowerHalfSize & 1) == 1) {
		// Odd number of elements in lower half, easy to find its median
		std::multiset<double>::const_iterator it1 = m_oValues.begin();
		while (nLowerHalfMedianIndex-- > 0) ++it1;
		return *it1;
	} else {
		// Even number of elements in lower half
		std::multiset<double>::const_iterator it1 = m_oValues.begin();
		while (nLowerHalfMedianIndex-- > 1) ++it1;
		return (*it1 + *(++it1)) * 0.5;
	}
}

double MovingAverage::WeightedMovingAverage() const
{
	if (!well_populated()) return 0;

	double dWeightedSum = 0;
	int dWeightFactor = 0;

	// Iterate values from oldest to latest
	std::list<std::pair<double, std::multiset<double>::iterator> >::const_iterator it1;
	for (it1 = m_oPointersByTime.begin(); it1 != m_oPointersByTime.end(); ++it1) {
		dWeightedSum += (++dWeightFactor) * *(it1->second);
	}

	return dWeightedSum / (size() * (size() + 1) * 0.5);
}

// Returns +1 when the there are only positive numbers, -1 when only negative numbers, 0 otherwise
char MovingAverage::Signum() const
{
	char bContainsPositiveValues = (m_nLastPositiveValueIndex < size()) ? 1 : 0;
	char bContainsNegativeValues = (m_nLastNegativeValueIndex < size()) ? 1 : 0;

	return bContainsPositiveValues - bContainsNegativeValues;
}

void MovingAverage::Print()
{
	std::list<std::pair<double, std::multiset<double>::iterator> >::const_iterator it1;
	for (it1 = m_oPointersByTime.begin(); it1 != m_oPointersByTime.end(); ++it1) {
		printf("%.3f@%.3f | ", *(it1->second) * 1000, it1->first);
	}
	printf("\nm_nLastPositiveValueIndex = %d; m_nLastNegativeValueIndex = %d\n\n", m_nLastPositiveValueIndex, m_nLastNegativeValueIndex);
}
