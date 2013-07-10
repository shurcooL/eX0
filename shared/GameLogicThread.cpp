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
// CONTINUE: Ok, problem here is that the very first tick (when g_dNextTickTime == 0) happens before local player even joins the server,
//           So, need to figure something out to fix that, or go back to not performing the 1st tick...
//printf("tick(++%d) - logic_1 %f\n", g_cCurrentCommandSequenceNumber, g_pGameSession->LogicTimer().GetRealTime());

				g_dNextTickTime += 1.0 / g_cCommandRate;
				++g_cCurrentCommandSequenceNumber;

				for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
					if (PlayerGet(nPlayer) != NULL)
						PlayerGet(nPlayer)->Tick();
				}
			}*/
			while (g_pGameSession->LogicTimer().GetTime() >= g_dNextTickTime)
			{
				g_dNextTickTime += 1.0 / g_cCommandRate;
				++g_cCurrentCommandSequenceNumber;
				++g_pGameSession->GlobalStateSequenceNumberTEST;
			}
			for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
				if (PlayerGet(nPlayer) != NULL)
					PlayerGet(nPlayer)->Tick();
			}
			for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
				if (PlayerGet(nPlayer) != NULL)
					PlayerGet(nPlayer)->WeaponTickTEST();
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
			oParticleEngine.Tick();
			//if (bPaused) dTimePassed = fTempFloat;

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
		//glfwSleep(0.0678);
		//glfwSleep(0.234);
	}

	pThread->ThreadEnded();
}
