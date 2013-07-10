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

template <typename T, u_int Tsize> void ThreadSafeQueue<T, Tsize>::pop(T & oElement)
{
	if (!empty()) {
		oElement = *const_cast<T *>(m_pBuffer[m_nReadHead]);
		delete m_pBuffer[m_nReadHead];
		m_nReadHead = (m_nReadHead + 1) % m_knBufferSize;
	}
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
