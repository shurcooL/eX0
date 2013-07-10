#pragma once
#ifndef __HashMatcher_H__
#define __HashMatcher_H__

template <typename Th, typename T> class HashMatcher
{
public:
	HashMatcher(const u_int nMaxSize);
	~HashMatcher();

	void push(const Th oHash, const T oValue);
	T MatchAndRemoveAfter(const Th oHash);

private:
	HashMatcher(const HashMatcher &);
	HashMatcher & operator =(const HashMatcher &);

	std::list<std::pair<Th, T> >	m_oData;		// The back of the list has the latest entries
	const u_int						m_knMaxSize;
};

#include "HashMatcher.cpp"

#endif // __HashMatcher_H__
