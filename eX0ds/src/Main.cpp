#include "globals.h"
#include <iostream>

#pragma comment(linker, "/NODEFAULTLIB:\"LIBCMT\"")

double		dTimePassed = 0;
double		dCurTime = 0;
double		dBaseTime = 0;
u_int		nFpsFrames = 0;
double		dFpsBaseTime = 0;
double		dFpsTimePassed = 0;
string		sFpsString = (string)"eX0";

void eX0_assert(bool expression, string message)
{
	if (!expression)
		printf("\nAssertion FAILED: '%s'\n\n", message.c_str());
}

// initialization
bool Init(int argc, char *argv[])
{
	// init glfw
	if (!glfwInit())
		return false;

	// Initialize components
	if (!GameDataLoad()) {				// load game data
		printf("Couldn't load game data.\n");
		return false;
	}
	WeaponInitSpecs();
	nPlayerCount = 8; PlayerInit();		// Initialize the players
	if (!NetworkInit()) {				// Initialize the networking
		printf("Couldn't initialize the networking.\n");
		return false;
	}
	pTimedEventScheduler = new CTimedEventScheduler();
	if (!ServerInit()) {				// Initialize the server
		printf("Couldn't initialize the server.\n");
		return false;
	}
	SyncRandSeed();

	return true;
}

// deinitialization
void Deinit()
{
	printf("Deinit\n");

	// Sub-deinit
	delete pTimedEventScheduler; pTimedEventScheduler = NULL;
	ServerDeinit();
	NetworkDeinit();					// Shutdown the networking component
	nPlayerCount = 0; PlayerInit();		// Delete all players
	GameDataUnload();					// unload game data

	// terminate glfw
	glfwTerminate();
}

void Terminate(int nExitCode)
{
	// deinit
	Deinit();

	// DEBUG: Print out the memory usage stats
	//m_dumpMemoryReport();

	exit(nExitCode);
}

// syncronizes random seed with all clients
void SyncRandSeed(void)
{
	// DEBUG: This has no effect, due to PolyBoolean doing srand(clock()) anyway
	srand(426);
	//srand(static_cast<unsigned int>(time(NULL)));
}

#ifdef WIN32
BOOL WINAPI CtrlCHandler(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT)
		Terminate(1);

	return FALSE;
}
#else
void sigint_handler(int sig)
{
	Terminate(1);
}
#endif

void PrintHi(void *p)
{
	//printf("%30.20f\n", glfwGetTime());
	printf("===================== %f\n", glfwGetTime());
}

int main(int argc, char *argv[])
{
	// Add a Ctrl+C signal handler, for abrupt termination
#ifdef WIN32
	SetConsoleCtrlHandler(CtrlCHandler, TRUE);
#else
	signal(SIGINT, sigint_handler);
#endif

	// Print the version and date/time built
	printf("eX0ds v0.0 - Built on %s at %s.\n\n", __DATE__, __TIME__);

	// DEBUG: Set the level name through the 1st command line param
	if (argc >= 2)
		sLevelName = argv[1];

	// Initialize the dedicated server
	if (!Init(argc, argv))
		Terminate(1);

	if (!ServerStart())
	{
		printf("Couldn't start the server, exiting.\n");
		Terminate(1);
	} else printf("Started the server successfully.\n");

	// make sure that the physics won't count all the time passed during init
	dBaseTime = glfwGetTime();
	dFpsBaseTime = dBaseTime;

	CTimedEvent oEvent(ceil(glfwGetTime() * 0.1) * 10, 10.0, PrintHi, NULL);
	pTimedEventScheduler->ScheduleEvent(oEvent);

	// Main loop
	while (true)
	{
		// time passed calcs
		dCurTime = glfwGetTime();
		dTimePassed = dCurTime - dBaseTime;
		dBaseTime = dCurTime;

		// fps calcs
		nFpsFrames++;
		dFpsTimePassed = dCurTime - dFpsBaseTime;
		if (dFpsTimePassed >= 10.0f)
		{
			sFpsString = (string)"eX0ds - " + ftos(nFpsFrames / (float)dFpsTimePassed) + " fps";
			//printf("%s\n", sFpsString.c_str());
			dFpsBaseTime = dCurTime;
			nFpsFrames = 0;
		}

		glfwSleep(0.01);
	}

	// Clean up and exit nicely
	Terminate(0);

	return 0;
}
