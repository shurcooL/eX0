// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

Thread::Thread(GLFWthreadfun oFunction, void * pArgument, string sName)
	: m_sName(sName)
{
	printf("%s thread Constructor() started.\n", m_sName.c_str());

	m_bShouldBeRunning = true;
	m_oThread = glfwCreateThread(oFunction, pArgument);
	if (m_oThread >= 0)
		printf("%s thread (tid = %d) created.\n", m_sName.c_str(), m_oThread);
	else {
		printf("Couldn't create %s thread.\n", m_sName.c_str());
		throw 1;
	}
}

Thread::~Thread()
{
	if (m_oThread >= 0)
	{
		RequestStop();

		glfwWaitThread(m_oThread, GLFW_WAIT);
		m_oThread = -1;
		printf("%s thread has been shut down.\n", m_sName.c_str());
	}

	printf("%s thread ~Destructor() done.\n", m_sName.c_str());
}

void Thread::ThreadEnded()
{
	printf("%s thread has ended.\n", m_sName.c_str());
	m_oThread = -1;
}
