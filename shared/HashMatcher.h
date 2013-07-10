#pragma once
#ifndef __HashMatcher_H__
#define __HashMatcher_H__

template <typename Th, typename T> class HashMatcher
{
public:
	HashMatcher(u_int nMaxSize);
	~HashMatcher();

	void push(Th oHash, T oValue);
	bool MatchAndRemoveAfter(Th oHash);
	T GetLastMatchedValue() const;

private:
	list<pair<Th, T> >	m_oData;
	T					m_oLastMatchedValue;
	const u_int			m_knMaxSize;
};

#include "HashMatcher.cpp"

#endif // __HashMatcher_H__
