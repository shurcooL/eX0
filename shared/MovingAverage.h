#pragma once
#ifndef __MovingAverage_H__
#define __MovingAverage_H__

class MovingAverage
{
public:
	MovingAverage(double dTimeSpan, u_int nMinSize);
	~MovingAverage();

	void push(double dValue, double dTime);		// Time in seconds
	void clear();

	u_int size() const;
	bool well_populated() const;
	double Mean() const;
	double LowerQuartile() const;
	double WeightedMovingAverage() const;
	char Signum() const;

	void Print();

private:
	MovingAverage(const MovingAverage &);
	MovingAverage & operator =(const MovingAverage &);

	const double	m_kdTimeSpan;		// Time in seconds
	const u_int		m_knMinSize;

	std::multiset<double>											m_oValues;				// Sorted by value, from lowest to highest
	std::list<std::pair<double, std::multiset<double>::iterator> >	m_oPointersByTime;		// Sorted (manually) by time, from oldest to newest

	u_int			m_nLastPositiveValueIndex;
	u_int			m_nLastNegativeValueIndex;
};

#endif // __MovingAverage_H__
