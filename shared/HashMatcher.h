#pragma once
#ifndef __HashMatcher_H__
#define __HashMatcher_H__

template <typename Th, typename T> class HashMatcher
{
public:
	HashMatcher(u_int nMaxSize);
	~HashMatcher();

	void push(Th oHash, T oValue);
	T MatchAndRemoveAfter(Th oHash);

private:
	HashMatcher(const HashMatcher &);
	HashMatcher & operator =(const HashMatcher &);

	list<pair<Th, T> >	m_oData;
	const u_int			m_knMaxSize;
};

#include "HashMatcher.cpp"

#endif // __HashMatcher_H__
