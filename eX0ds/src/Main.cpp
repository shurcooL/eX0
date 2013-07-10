#include "globals.h"
#include <iostream>

#pragma comment(linker, "/NODEFAULTLIB:\"LIBCMT\"")

volatile bool	bProgramRunning = true;
volatile int	iGameState = 1;
double		dTimePassed = 0;
double		dCurTime = 0;
double		dBaseTime = 0;
string		sFpsString = (string)"eX0";

int			nGlobalExitCode = 0;

void eX0_assert(bool expression, string message)
{
	if (!expression) {
		printf("\nAssertion FAILED: '%s'\n\n", message.c_str());
		//abort();
	}
}

// initialization
bool Init(int, char *[])
{
	// init glfw
	if (!glfwInit()) {
		printf("Couldn't init GLFW...\n");
		return false;
	}
	printf("Main thread tid = %d.\n", glfwGetThreadID());

	// Initialize components
	if (!GameDataLoad()) {				// load game data
		printf("Couldn't load game data.\n");
		return false;
	}
	WeaponInitSpecs();
	nPlayerCount = 8;					// Set the max player limit for this server
	if (!NetworkInit()) {				// Initialize the networking
		printf("Couldn't initialize the networking.\n");
		return false;
	}
	pTimedEventScheduler = new CTimedEventScheduler();
	pGameLogicThread = new GameLogicThread();
	pLocalServer = new LocalServer();

	SyncRandSeed();

	return true;
}

// deinitialization
void Deinit()
{
	printf("Deinit\n");

	// Sub-deinit
	delete pGameLogicThread; pGameLogicThread = NULL;
	delete pTimedEventScheduler; pTimedEventScheduler = NULL;
	delete pLocalServer; pLocalServer = NULL;
	NetworkDeinit();					// Shutdown the networking component
	CPlayer::RemoveAll();				// Delete all players
	GameDataUnload();					// unload game data

	// terminate glfw
	glfwTerminate();
}

// syncronizes random seed with all clients
void SyncRandSeed(void)
{
	// DEBUG: This has no effect, due to PolyBoolean doing srand(clock()) anyway
	srand(426);
	//srand(static_cast<unsigned int>(time(NULL)));
}

// quits
void Terminate(int nExitCode)
{
	if (nGlobalExitCode == 0) nGlobalExitCode = nExitCode;

	if (nExitCode != 0 && glfwGetThreadID() == 0) {
		// deinit
		Deinit();

		// DEBUG: Print out the memory usage stats
		//m_dumpMemoryReport();

		exit(nGlobalExitCode);
	} else {
		if (bProgramRunning) bProgramRunning = false;
		else Terminate(10);
	}
}

void PrintHi(void *)
{
	//printf("%30.20f\n", glfwGetTime());
	printf("===================== %f\n", glfwGetTime());
}

#ifdef WIN32
BOOL WINAPI CtrlCHandler(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT || dwCtrlType == CTRL_CLOSE_EVENT
		|| dwCtrlType == CTRL_LOGOFF_EVENT || dwCtrlType == CTRL_SHUTDOWN_EVENT)
	{
		Terminate(0);

		return TRUE;
	}

	return FALSE;
}
#else
void signal_handler(int sig)
{
	Terminate(0);
}
#endif

// main function
int main(int argc, char *argv[])
{
	// Add a Ctrl+C signal handler, for abrupt termination
#ifdef WIN32
	SetConsoleCtrlHandler(&CtrlCHandler, TRUE);
#else
	signal(SIGINT, &signal_handler);
	signal(SIGHUP, &signal_handler);
#endif



	/*new CPlayer();
	new CPlayer();
	CPlayer * p1 = new LocalAuthPlayer();
	new CPlayer();
	delete p1;
	new CPlayer();


	CPlayer::RemoveAll();

return 0;*/





	// Print the version and date/time built
	printf("%s\n\n", EX0_BUILD_STRING);

	FpsCounter * pMainCounter = FpsCounter::CreateCounter("MainThread");

	// DEBUG: Set the level name through the 1st command line param
	if (argc >= 2)
		sLevelName = argv[1];

	// Initialize the dedicated server
	if (!Init(argc, argv))
		Terminate(1);

	CTimedEvent oEvent(ceil(glfwGetTime() * 0.01) * 100, 100.0, &PrintHi, NULL);
	pTimedEventScheduler->ScheduleEvent(oEvent);

	// make sure that the physics won't count all the time passed during init
	dBaseTime = glfwGetTime();

	// Main loop
	while (bProgramRunning)
	{
		pMainCounter->IncrementCounter();
		FpsCounter::UpdateCounters(glfwGetTime());

/*		// time passed calcs
		dCurTime = glfwGetTime();
		dTimePassed = dCurTime - dBaseTime;
		dBaseTime = dCurTime;

		// fps calcs
		nFpsFrames++;
		dFpsTimePassed = dCurTime - dFpsBaseTime;
		if (dFpsTimePassed >= 3.0f)
		{
			sFpsString = (string)"eX0ds - " + ftos(nFpsFrames / (float)dFpsTimePassed) + " fps";
			//printf("%s\n", sFpsString.c_str());
			dFpsBaseTime = dCurTime;
			nFpsFrames = 0;
		}

glfwLockMutex(oPlayerTick);
		for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
			if (PlayerGet(nPlayer) != NULL)
				dynamic_cast<LocalAuthPlayer *>(PlayerGet(nPlayer))->ProcessInputCmdTEST();
		}
glfwUnlockMutex(oPlayerTick);
*/

		glfwSleep(0.0001);
	}

	FpsCounter::DeleteCounter(pMainCounter);

	// Clean up and exit nicely
	Deinit();

	printf("Returning %d from main().                       %s\n", nGlobalExitCode, nGlobalExitCode == 0 ? ":) :) :) :) :) :)))" : ">___________________<");
	return 0;
}
