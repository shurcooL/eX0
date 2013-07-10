#pragma once
#ifndef __ThreadSafeQueue_H__
#define __ThreadSafeQueue_H__

template <typename T, u_int Tsize> class ThreadSafeQueue
{
public:
	ThreadSafeQueue();
	~ThreadSafeQueue();

	bool push(T & oElement);
	void pop(T & oElement);

	bool empty() const;
	bool full() const;
	u_int size() const;
	u_int capacity() const;
	void clear();

private:
	ThreadSafeQueue(const ThreadSafeQueue &);
	ThreadSafeQueue & operator =(const ThreadSafeQueue &);

	static const u_int		m_knBufferSize = Tsize + 1;
	volatile T *			m_pBuffer[m_knBufferSize];
	volatile u_int			m_nWriteHead;
	volatile u_int			m_nReadHead;
};

#include "ThreadSafeQueue.cpp"

#endif // __ThreadSafeQueue_H__
