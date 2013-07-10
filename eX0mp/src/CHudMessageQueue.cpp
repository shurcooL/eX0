#include "globals.h"

const double CHudMessageQueue::m_kdMessageTimeoutThreshold = 0.5;
const int CHudMessageQueue::m_knHorizontalDistance = 15;

CHudMessageQueue::CHudMessageQueue(int nTopLeftCornerX, int nTopLeftCornerY, u_int nMessageLimit, double dMessageTimeout)
{
	m_nTopLeftCornerX = nTopLeftCornerX;
	m_nTopLeftCornerY = nTopLeftCornerY;
	m_nMessageLimit = nMessageLimit;
	m_dMessageTimeout = dMessageTimeout;

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
		m_dMessageTimer = g_pGameSession->RenderTimer().GetTime() + m_dMessageTimeout;
	} else if (m_oMessages.empty()) {
		// Empty message queue
		m_dMessageTimer = g_pGameSession->RenderTimer().GetTime() + 1.5 * m_dMessageTimeout;
	} else if (m_dMessageTimer <= g_pGameSession->RenderTimer().GetTime() + m_kdMessageTimeoutThreshold) {
		// Neither full or empty, but the last message is about to disappear
		m_oMessages.pop_front();
		m_dMessageTimer += m_dMessageTimeout;
	}
	m_oMessages.push_back(sMessage);

	glfwUnlockMutex(m_oMessageMutex);
}

void CHudMessageQueue::Render()
{
	glfwLockMutex(m_oMessageMutex);

	if (!m_oMessages.empty())
	{
		if (m_dMessageTimer <= g_pGameSession->RenderTimer().GetTime()) {
			m_oMessages.pop_front();
			m_dMessageTimer += m_dMessageTimeout;
		}

		int nMessageNumber = 0;
		for (std::list<string>::iterator it1 = m_oMessages.begin(); it1 != m_oMessages.end(); ++it1)
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
