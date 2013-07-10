template <typename T, u_int Tsize> ThreadSafeQueue<T, Tsize>::ThreadSafeQueue()
	: m_pBuffer(),
	  m_nWriteHead(0),
	  m_nReadHead(0)
{
}

template <typename T, u_int Tsize> ThreadSafeQueue<T, Tsize>::~ThreadSafeQueue()
{
}

template <typename T, u_int Tsize> bool ThreadSafeQueue<T, Tsize>::push(T & oElement)
{
	if (!full()) {
		m_pBuffer[m_nWriteHead] = new T(oElement);
		m_nWriteHead = (m_nWriteHead + 1) % m_knBufferSize;

		return true;
	} else
		return false;
}

template <typename T, u_int Tsize> void ThreadSafeQueue<T, Tsize>::pop()
{
	if (!empty()) {
		delete m_pBuffer[m_nReadHead];
		m_nReadHead = (m_nReadHead + 1) % m_knBufferSize;
	}
	else
		throw 1;
}

template <typename T, u_int Tsize> T & ThreadSafeQueue<T, Tsize>::front()
{
	if (!empty()) {
		return *const_cast<T *>(m_pBuffer[m_nReadHead]);
	}
	else
		throw 1;
}

template <typename T, u_int Tsize> const T & ThreadSafeQueue<T, Tsize>::front() const
{
	if (!empty()) {
		return *m_pBuffer[m_nReadHead];
	}
	else
		throw 1;
}

template <typename T, u_int Tsize> inline bool ThreadSafeQueue<T, Tsize>::empty() const
{
	return m_nWriteHead == m_nReadHead;
}

template <typename T, u_int Tsize> inline bool ThreadSafeQueue<T, Tsize>::full() const
{
	return (m_nWriteHead + 1) % m_knBufferSize == m_nReadHead;
}

template <typename T, u_int Tsize> u_int ThreadSafeQueue<T, Tsize>::size() const
{
	return (m_nWriteHead - m_nReadHead + m_knBufferSize) % m_knBufferSize;
}

template <typename T, u_int Tsize> u_int ThreadSafeQueue<T, Tsize>::capacity() const
{
	return Tsize;
}

template <typename T, u_int Tsize> void ThreadSafeQueue<T, Tsize>::clear()
{
	while (!empty()) {
		delete m_pBuffer[m_nReadHead];
		m_nReadHead = (m_nReadHead + 1) % m_knBufferSize;
	}
}
