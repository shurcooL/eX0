#include "globals.h"

#pragma comment(linker, "/NODEFAULTLIB:\"LIBCMT\"")

void eX0_assert(bool expression, string message)
{
	if (!expression)
		printf("Assertion '%s' failed.\n", message.c_str());
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
	m_dumpMemoryReport();

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

int main(int argc, char *argv[])
{
	// Add a Ctrl+C signal handler, for server termination
#ifdef WIN32
	SetConsoleCtrlHandler(CtrlCHandler, TRUE);
#else
	signal(SIGINT, sigint_handler);
#endif

	// Initialize the dedicated server
	if (!Init(argc, argv))
		Terminate(1);

	if (!ServerStart())
	{
		printf("Couldn't start the server, exiting.\n");
		Terminate(1);
	} else printf("Started the server successfully.\n");

	// Main loop
	while (true)
	{
		glfwSleep(0.0);
	}
	//glfwSleep(5.0);

	// Clean up and exit nicely
	Terminate(0);

	return 0;
}
