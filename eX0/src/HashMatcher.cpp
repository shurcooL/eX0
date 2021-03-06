template <typename Th, typename T> HashMatcher<Th, T>::HashMatcher(const u_int nMaxSize)
	: m_knMaxSize(nMaxSize)
{
}

template <typename Th, typename T> HashMatcher<Th, T>::~HashMatcher()
{
}

template <typename Th, typename T> void HashMatcher<Th, T>::push(const Th oHash, const T oValue)
{
	if (m_oData.size() >= m_knMaxSize) m_oData.pop_front();
	m_oData.push_back(std::pair<Th, T>(oHash, oValue));		// The back of the list has the latest entries
}

template <typename Th, typename T> T HashMatcher<Th, T>::MatchAndRemoveAfter(const Th oHash)
{
	// Start from the oldest (beginning of the list), and go towards the most recently added items (end of the list)
	for (typename std::list<std::pair<Th, T> >::iterator it1 = m_oData.begin(); it1 != m_oData.end(); ++it1) {
		if (it1->first == oHash) {
			T oMatchedValue = it1->second;
			// Remove all the items older and the one found
			m_oData.erase(m_oData.begin(), ++it1);

			return oMatchedValue;
		}
	}

	throw 1;
}
