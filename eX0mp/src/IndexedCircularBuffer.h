#pragma once
#ifndef __IndexedCircularBuffer_H__
#define __IndexedCircularBuffer_H__

template <typename T, typename Tp> class IndexedCircularBuffer
{
public:
	IndexedCircularBuffer();
	~IndexedCircularBuffer();

	bool push(T & oItem, Tp nPosition);
	void pop();

	T & operator [](Tp nPosition);

	Tp begin();
	Tp end();

	T & front();
	T & back();

	bool empty();
	//bool full();
	u_int size();

	void clear();

private:
	T		*m_pBuffer;
	const u_int		m_knBufferSize;
	Tp		m_nBufferHead;
	Tp		m_nBufferTail;

	u_int capacity();
};

#include "IndexedCircularBuffer.cpp"

#endif // __IndexedCircularBuffer_H__
