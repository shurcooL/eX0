template <typename Th, typename T> HashMatcher<Th, T>::HashMatcher(u_int nMaxSize)
	: m_knMaxSize(nMaxSize)
{
}

template <typename Th, typename T> HashMatcher<Th, T>::~HashMatcher()
{
}

template <typename Th, typename T> void HashMatcher<Th, T>::push(Th oHash, T oValue)
{
	if (m_oData.size() >= m_knMaxSize) m_oData.pop_front();
	m_oData.push_back(pair<Th, T>(oHash, glfwGetTime()));		// The back of the list has the latest entries
}

template <typename Th, typename T> bool HashMatcher<Th, T>::MatchAndRemoveAfter(Th oHash)
{
	// Start from the oldest (beginning of the list), and go towards the most recently added items (end of the list)
	for (typename list<pair<Th, T> >::iterator it1 = m_oData.begin(); it1 != m_oData.end(); ++it1) {
		if (it1->first == oHash) {
			m_oLastMatchedValue = it1->second;
			// Remove all the items older and the one found
			m_oData.erase(m_oData.begin(), ++it1);

			return true;
		}
	}

	return false;
}

template <typename Th, typename T> T HashMatcher<Th, T>::GetLastMatchedValue() const
{
	return m_oLastMatchedValue;
}
