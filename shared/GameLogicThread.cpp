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

	int			nFpsFrames = 0;
	double		dFpsTimePassed = 0, dFpsBaseTime = glfwGetTime();

	// Main loop
	while (pThread->ShouldBeRunning())
	{
		// time passed calcs
		dCurTime = glfwGetTime();
		dTimePassed = dCurTime - dBaseTime;
		dBaseTime = dCurTime;

		// fps calcs
		pFpsCounter->IncrementCounter();
		nFpsFrames++;
		dFpsTimePassed = dCurTime - dFpsBaseTime;
		if (dFpsTimePassed >= 0.75)
		{
			sFpsString = string("eX0 - ") + ftos(nFpsFrames / static_cast<float>(dFpsTimePassed)) + " fps (game logic thread)";
			//glfwSetWindowTitle(sFpsString.c_str());
			dFpsBaseTime = glfwGetTime();
			nFpsFrames = 0;
		}

		if (!iGameState)
		// in game
		{
#ifdef EX0_CLIENT
			if (pLocalPlayer != NULL) {
				// mouse moved?
				InputMouseMovCalcs();

				// key or mouse button held down?
				InputKeyHold();
				InputMouseHold();
			}
#endif // EX0_CLIENT

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

	//printf("GameLogicThread has ended.\n");
	pThread->ThreadEnded();
}
