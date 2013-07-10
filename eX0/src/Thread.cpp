// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

Thread::Thread(GLFWthreadfun pFunction, void * pArgument, std::string sName)
	: m_bCanStart(false),
	  m_bShouldBeRunning(true),
	  m_bThreadEnded(false),
	  m_sName(sName),
	  m_pFpsCounter(FpsCounter::CreateCounter(sName))
{
	m_pArguments[0] = this;
	m_pArguments[1] = pArgument;
	m_oThread = glfwCreateThread(pFunction, m_pArguments);
	if (IsAlive())
		printf("%s thread (tid = %d) created.\n", m_sName.c_str(), m_oThread);
	else {
		printf("Couldn't create %s thread.\n", m_sName.c_str());
		throw 1;
	}

	m_bCanStart = true;		// Allow the thread to start
}

Thread::~Thread()
{
	if (IsAlive())
	{
		RequestStop();

		glfwWaitThread(m_oThread, GLFW_WAIT);
		m_oThread = -1;
		//printf("%s thread has been shut down.\n", m_sName.c_str());
	}

	FpsCounter::RemoveCounter(m_pFpsCounter);
}

void Thread::ThreadEnded()
{
	m_bThreadEnded = true;
	printf("%s thread has ended.\n", m_sName.c_str());
}

FpsCounter * Thread::GetFpsCounter()
{
	return m_pFpsCounter;
}

Thread * Thread::GetThisThreadAndRevertArgument(void *& pArgument)
{
	void ** pArguments = static_cast<void **>(pArgument);

	Thread * pThread = static_cast<Thread *>(pArguments[0]);
	pArgument = pArguments[1];

	// Delay the thread from starting until the end of its Thread object constructor
	while (!pThread->m_bCanStart)
		glfwSleep(0);

	return pThread;
}
