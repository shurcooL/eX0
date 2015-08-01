// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

GameLogicThread * pGameLogicThread = NULL;

GameLogicThread::GameLogicThread()
	: m_pThread(new Thread(&GameLogicThread::ThreadFunction, NULL, "GameLogic"))
{
}

GameLogicThread::~GameLogicThread()
{
	delete m_pThread;
}

void GLFWCALL GameLogicThread::ThreadFunction(void * pArgument)
{
	Thread * pThread = Thread::GetThisThreadAndRevertArgument(pArgument);
	FpsCounter * pFpsCounter = pThread->GetFpsCounter();

	// Wait until LogicTimer is started
	while (false == g_pGameSession->LogicTimer().IsStarted() && pThread->ShouldBeRunning())
		glfwSleep(0);

	// Main loop
	while (pThread->ShouldBeRunning())
	{
		// time passed calcs
		/*dCurTime = glfwGetTime();
		dTimePassed = dCurTime - dBaseTime;
		dBaseTime = dCurTime;*/
		g_pGameSession->LogicTimer().UpdateTime();

		// fps calcs
		pFpsCounter->IncrementCounter();

		if (!iGameState)
		// in game
		{
glfwLockMutex(oPlayerTick);
			/*while (g_pGameSession->LogicTimer().GetTime() >= g_dNextTickTime)
			{
//printf("tick(++%d) - logic_1 %f\n", g_cCurrentCommandSequenceNumber, g_pGameSession->LogicTimer().GetRealTime());

				g_dNextTickTime += 1.0 / g_cCommandRate;
				++g_cCurrentCommandSequenceNumber;

				for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
					if (PlayerGet(nPlayer) != NULL)
						PlayerGet(nPlayer)->Tick();
				}
			}*/
			while (g_pGameSession->LogicTimer().GetTime() >= g_dNextTickTime)
			{
				g_dNextTickTime += 1.0 / g_cCommandRate;
				//++g_cCurrentCommandSequenceNumber;
				++g_pGameSession->GlobalStateSequenceNumberTEST;
				//printf("GlobalStateSequenceNumberTEST: %d\n", g_pGameSession->GlobalStateSequenceNumberTEST);
			}
			for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
				if (PlayerGet(nPlayer) != NULL)
					PlayerGet(nPlayer)->Tick();
			}
			for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
				if (PlayerGet(nPlayer) != NULL)
					PlayerGet(nPlayer)->WeaponTickTEST();
			}
			for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
				if (PlayerGet(nPlayer) != NULL) {
					PlayerGet(nPlayer)->ProcessCommands();
				}
			}
			for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
				if (PlayerGet(nPlayer) != NULL) {
					PlayerGet(nPlayer)->ProcessWpnCommands();
				}
			}
			for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
				if (PlayerGet(nPlayer) != NULL) {
					PlayerGet(nPlayer)->ProcessUpdates();
				}
			}
glfwUnlockMutex(oPlayerTick);

			// particle engine tick
	double t1 = glfwGetTime();
glfwLockMutex(oPlayerTick);
			oParticleEngine.Tick();
glfwUnlockMutex(oPlayerTick);
	double td = glfwGetTime() - t1;
	//if (td > 0.003) printf("particle tick took %.10f ms\n", td * 1000);
			//if (bPaused) dTimePassed = fTempFloat;

//#ifndef EX0_CLIENT
glfwLockMutex(oPlayerTick);
			for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
				if (PlayerGet(nPlayer) != NULL)
					PlayerGet(nPlayer)->SendUpdate();
			}
glfwUnlockMutex(oPlayerTick);
//#endif
		}

		glfwSleep(0.001);
		//glfwSleep(0.0678);
		//glfwSleep(0.234);
	}

	pThread->ThreadEnded();
}
