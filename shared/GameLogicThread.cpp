// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
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

	// Main loop
	while (pThread->ShouldBeRunning())
	{
		// time passed calcs
		dCurTime = glfwGetTime();
		dTimePassed = dCurTime - dBaseTime;
		dBaseTime = dCurTime;

		// fps calcs
		pFpsCounter->IncrementCounter();

		if (!iGameState)
		// in game
		{
glfwLockMutex(oPlayerTick);
			while (dCurTime >= g_dNextTickTime)
			{
				g_dNextTickTime += 1.0 / g_cCommandRate;
				++g_cCurrentCommandSequenceNumber;

				for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
					if (PlayerGet(nPlayer) != NULL)
						PlayerGet(nPlayer)->Tick();
				}
			}
			for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
				if (PlayerGet(nPlayer) != NULL) {
					PlayerGet(nPlayer)->AfterTick();
				}
			}
			for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
				if (PlayerGet(nPlayer) != NULL) {
					PlayerGet(nPlayer)->ProcessAuthUpdateTEST();
				}
			}
#ifdef EX0_CLIENT
			// player tick
			//if (bPaused) { fTempFloat = (float)dTimePassed; dTimePassed = 0.00001; }
			/*if (pLocalPlayer->GetTeam() != 2 && !pLocalPlayer->IsDead()) {
				pLocalPlayer->Tick();
			} else {
				//pLocalPlayer->FakeTick();
			}*/
			//PlayerTick();
#endif
glfwUnlockMutex(oPlayerTick);

			// particle engine tick
			/*oParticleEngine.Tick();
			if (bPaused) dTimePassed = fTempFloat;*/

//#ifndef EX0_CLIENT
glfwLockMutex(oPlayerTick);
			for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
				if (PlayerGet(nPlayer) != NULL)
					PlayerGet(nPlayer)->SendUpdate();
			}
glfwUnlockMutex(oPlayerTick);
//#endif
		}

		glfwSleep(0.0001);
	}

	pThread->ThreadEnded();
}
