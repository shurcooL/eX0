#include "globals.h"

const float CHudMessageQueue::m_kfMessageTimeoutThreshold = 0.5f;

CHudMessageQueue::CHudMessageQueue(int nTopLeftCornerX, int nTopLeftCornerY, u_int nMessageLimit, float fMessageTimeout)
{
	m_nTopLeftCornerX = nTopLeftCornerX;
	m_nTopLeftCornerY = nTopLeftCornerY;
	m_nMessageLimit = nMessageLimit;
	m_fMessageTimeout = fMessageTimeout;

	m_oMessageMutex = glfwCreateMutex();
}

CHudMessageQueue::~CHudMessageQueue()
{
	glfwDestroyMutex(m_oMessageMutex);
}

void CHudMessageQueue::AddMessage(string sMessage)
{
	glfwLockMutex(m_oMessageMutex);

	if (m_oMessages.size() >= m_nMessageLimit) {
		// Full message queue
		m_oMessages.pop_front();
		m_fMessageTimer = m_fMessageTimeout;
	} else if (m_oMessages.empty()) {
		// Empty message queue
		m_fMessageTimer = 1.5f * m_fMessageTimeout;
	} else if (m_fMessageTimer <= m_kfMessageTimeoutThreshold) {
		// Neither full or empty, but the last message is about to disappear
		m_oMessages.pop_front();
		m_fMessageTimer = m_fMessageTimeout;
	}
	m_oMessages.push_back(sMessage);

	glfwUnlockMutex(m_oMessageMutex);
}

void CHudMessageQueue::Render()
{
	glfwLockMutex(m_oMessageMutex);

	if (!m_oMessages.empty())
	{
		m_fMessageTimer -= (float)dTimePassed;
		if (m_fMessageTimer <= 0.0f) {
			m_oMessages.pop_front();
			m_fMessageTimer = m_fMessageTimeout;
		}

		int nMessageNumber = 0;
		for (list<string>::iterator it1 = m_oMessages.begin(); it1 != m_oMessages.end(); ++it1)
		{
			glColor3f(0, 0, 0);
			OglUtilsPrint(m_nTopLeftCornerX + 1, m_nTopLeftCornerY + 1 + nMessageNumber * m_knHorizontalDistance, 0, false, (*it1).c_str());

			glColor3f(0.89f, 0.51f, 0.06f);
			OglUtilsPrint(m_nTopLeftCornerX, m_nTopLeftCornerY + nMessageNumber * m_knHorizontalDistance, 0, false, (*it1).c_str());

			++nMessageNumber;
		}
	}

	glfwUnlockMutex(m_oMessageMutex);
}
