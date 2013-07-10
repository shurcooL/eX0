template <typename T, typename Tp> IndexedCircularBuffer<T, Tp>::IndexedCircularBuffer() : m_knBufferSize(1 << (8 * sizeof(Tp)))
{
	m_pBuffer = new T[m_knBufferSize];
	m_nBufferHead = 0;
	m_nBufferTail = 0;
}

template <typename T, typename Tp> IndexedCircularBuffer<T, Tp>::~IndexedCircularBuffer()
{
	delete[] m_pBuffer;
}

template <typename T, typename Tp> bool IndexedCircularBuffer<T, Tp>::push(T & oItem, Tp nPosition)
{
	//eX0_assert(nPosition >= 0 && nPosition < m_knBufferSize, "nPosition >= 0 && nPosition < m_knBufferSize");

	if (size() < capacity()) {
		if (empty()) {
			m_pBuffer[nPosition] = oItem;
			m_nBufferHead = nPosition;
			m_nBufferTail = (nPosition + 1) % m_knBufferSize;
		} else {
			eX0_assert(nPosition == m_nBufferTail, "nPosition == m_nBufferTail");

			m_pBuffer[m_nBufferTail] = oItem;
			m_nBufferTail = (m_nBufferTail + 1) % m_knBufferSize;
		}

		return true;
	} else
		return false;
}

template <typename T, typename Tp> void IndexedCircularBuffer<T, Tp>::pop()
{
	eX0_assert(!empty(), "tried to pop() an empty buffer");

	m_nBufferHead = (m_nBufferHead + 1) % m_knBufferSize;
}

template <typename T, typename Tp> T & IndexedCircularBuffer<T, Tp>::operator [](Tp nPosition)
{
	eX0_assert(!empty(), "tried to access operator[] on an empty buffer");

	return m_pBuffer[nPosition];
}

template <typename T, typename Tp> Tp IndexedCircularBuffer<T, Tp>::begin() { return m_nBufferHead; }
template <typename T, typename Tp> Tp IndexedCircularBuffer<T, Tp>::end() { return m_nBufferTail; }

template <typename T, typename Tp> T & IndexedCircularBuffer<T, Tp>::front()
{
	eX0_assert(!empty(), "tried to get front() of an empty buffer");

	return m_pBuffer[m_nBufferHead];
}

template <typename T, typename Tp> T & IndexedCircularBuffer<T, Tp>::back()
{
	eX0_assert(!empty(), "tried to get back() of an empty buffer");

	return m_pBuffer[(m_nBufferTail - 1 + m_knBufferSize) % m_knBufferSize];
}

template <typename T, typename Tp> bool IndexedCircularBuffer<T, Tp>::empty()
{
	return m_nBufferHead == m_nBufferTail;
}

template <typename T, typename Tp> u_int IndexedCircularBuffer<T, Tp>::size()
{
	return (m_nBufferTail - m_nBufferHead + m_knBufferSize) % m_knBufferSize;
}

template <typename T, typename Tp> void IndexedCircularBuffer<T, Tp>::clear()
{
	m_nBufferHead = 0;
	m_nBufferTail = 0;
}

template <typename T, typename Tp> u_int IndexedCircularBuffer<T, Tp>::capacity()
{
	return m_knBufferSize - 1;
}
