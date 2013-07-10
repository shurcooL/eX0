#pragma once
#ifndef __IndexedCircularBuffer_H__
#define __IndexedCircularBuffer_H__

template <typename T, typename Tp> class IndexedCircularBuffer
{
public:
	IndexedCircularBuffer();
	~IndexedCircularBuffer();

	bool push(T & oElement, Tp nPosition);
	void pop();

	T & operator [](Tp nPosition);

	Tp begin() const;
	Tp end() const;

	T & front();
	T & back();

	bool empty() const;
	bool full() const;
	u_int size() const;

	void clear();

private:
	IndexedCircularBuffer(const IndexedCircularBuffer &);
	IndexedCircularBuffer & operator =(const IndexedCircularBuffer &);

	const u_int		m_knBufferSize;
	T		*m_pBuffer;
	Tp		m_nBufferHead;
	Tp		m_nBufferTail;

	u_int capacity() const;
};

#include "IndexedCircularBuffer.cpp"

#endif // __IndexedCircularBuffer_H__
