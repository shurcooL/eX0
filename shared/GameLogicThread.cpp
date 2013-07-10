// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

GameLogicThread * pGameLogicThread = NULL;

GameLogicThread::GameLogicThread()
{
	printf("GameLogicThread() Constructor started.\n");

	m_bThreadRun = true;
	m_oThread = glfwCreateThread(&GameLogicThread::Thread, this);

	if (m_oThread >= 0)
		printf("GameLogicThread (tid = %d) created.\n", m_oThread);
	else {
		printf("Couldn't create GameLogicThread.\n");
		throw 1;
	}
}

GameLogicThread::~GameLogicThread()
{
	if (m_oThread >= 0)
	{
		m_bThreadRun = false;

		glfwWaitThread(m_oThread, GLFW_WAIT);
		//glfwDestroyThread(m_oThread);
		m_oThread = -1;

		printf("GameLogicThread thread has been destroyed.\n");
	}

	printf("GameLogicThread() ~Destructor done.\n");
}

void GLFWCALL GameLogicThread::Thread(void * pArgument)
{
	GameLogicThread * pThread = static_cast<GameLogicThread *>(pArgument);

	int			nFpsFrames = 0;
	double		dFpsTimePassed = 0, dFpsBaseTime = glfwGetTime();

	// Main loop
	while (pThread->m_bThreadRun)
	{
		// time passed calcs
		dCurTime = glfwGetTime();
		dTimePassed = dCurTime - dBaseTime;
		dBaseTime = dCurTime;

		// fps calcs
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
			// mouse moved?
			InputMouseMovCalcs();

			// key or mouse button held down?
			InputKeyHold();
			InputMouseHold();
#endif

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

#ifndef EX0_CLIENT
glfwLockMutex(oPlayerTick);
			for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
				if (PlayerGet(nPlayer) != NULL)
					PlayerGet(nPlayer)->SendUpdate();
			}
glfwUnlockMutex(oPlayerTick);
#endif
		}

		glfwSleep(0.0001);
	}

	printf("GameLogicThread has ended.\n");
}
