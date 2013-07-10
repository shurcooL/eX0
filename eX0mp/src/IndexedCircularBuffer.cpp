template <typename T> IndexedCircularBuffer<T>::IndexedCircularBuffer(int nBufferSize)
{
	eX0_assert(nBufferSize > 1, "nBufferSize > 1");

	m_pBuffer = new T[nBufferSize];
	m_nBufferSize = nBufferSize;
	m_nBufferHead = 0;
	m_nBufferTail = 0;
}

template <typename T> IndexedCircularBuffer<T>::~IndexedCircularBuffer()
{
	delete[] m_pBuffer;
}

template <typename T> bool IndexedCircularBuffer<T>::push(T & oItem, int nPosition)
{
	eX0_assert(nPosition >= 0 && nPosition < m_nBufferSize,
		"nPosition >= 0 && nPosition < m_nBufferSize");

	if (size() < capacity()) {
		if (empty()) {
			m_pBuffer[nPosition] = oItem;
			m_nBufferHead = nPosition;
			m_nBufferTail = (m_nBufferHead + 1) % m_nBufferSize;
		} else {
			eX0_assert(nPosition == m_nBufferTail, "nPosition == m_nBufferTail");

			m_pBuffer[m_nBufferTail] = oItem;
			m_nBufferTail = (m_nBufferTail + 1) % m_nBufferSize;
		}

		return true;
	} else
		return false;
}

template <typename T> void IndexedCircularBuffer<T>::pop()
{
	eX0_assert(!empty(), "!empty()");

	m_nBufferHead = (m_nBufferHead + 1) % m_nBufferSize;
}

template <typename T> T & IndexedCircularBuffer<T>::operator [](int nPosition)
{
	return m_pBuffer[nPosition];
}

template <typename T> int IndexedCircularBuffer<T>::begin() { return m_nBufferHead; }
template <typename T> int IndexedCircularBuffer<T>::end() { return m_nBufferTail; }

template <typename T> T & IndexedCircularBuffer<T>::front()
{
	eX0_assert(!empty(), "!empty()");

	return m_pBuffer[m_nBufferHead];
}

template <typename T> T & IndexedCircularBuffer<T>::back()
{
	eX0_assert(!empty(), "!empty()");

	return m_pBuffer[(m_nBufferTail + m_nBufferSize - 1) % m_nBufferSize];
}

template <typename T> bool IndexedCircularBuffer<T>::empty()
{
	return m_nBufferHead == m_nBufferTail;
}

template <typename T> int IndexedCircularBuffer<T>::size()
{
	if (!empty()) return (m_nBufferTail + m_nBufferSize - m_nBufferHead) % m_nBufferSize;
	else return 0;
}

template <typename T> void IndexedCircularBuffer<T>::clear()
{
	m_nBufferHead = 0;
	m_nBufferTail = 0;
}

template <typename T> int IndexedCircularBuffer<T>::capacity()
{
	return m_nBufferSize - 1;
}
