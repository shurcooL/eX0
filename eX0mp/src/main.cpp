#include "globals.h"

// no console window
//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#pragma comment(linker, "/NODEFAULTLIB:\"LIBCMT\"")
//#pragma comment(linker, "/NODEFAULTLIB:\"LIBC\"")

volatile int	iGameState = 1;
bool			bPaused = false;

bool			bWireframe = false;
bool			bUseDefaultTriangulation = true;
bool			bStencilOperations = false;

GLFWvidmode		oDesktopMode;
bool			bFullscreen = false;

double			dTimePassed = 0;
double			dCurTime = 0;
double			dBaseTime = 0;
int				iFpsFrames = 0;
double			dFpsBaseTime = 0;
double			dFpsTimePassed = 0;
string			sFpsString = (string)"eX0";

string			sTempString = (string)"";
float			fTempFloat = 0;
int				iTempInt = 0;

u_long counter1 = 0;
u_long counter2 = 0;

void eX0_assert(bool expression, string message)
{
	if (!expression)
		printf("\nAssertion FAILED: '%s'\n\n", message.c_str());
}

bool CloseBadWindow()
{
	if (glfwGetWindowParam(GLFW_OPENED)) glfwCloseWindow();
	return false;
}

// initialization
bool Init(int argc, char *argv[])
{
	// init glfw
	if (!glfwInit()) {
		printf("Couldn't init GLFW...\n");
		return false;
	}

	// let the use choose whether to run in fullscreen mode
	bFullscreen = false;
	//if (argc >= 2 && strcmp(argv[1], "--fullscreen") == 0) bFullscreen = true;
#ifdef WIN32
	//bFullscreen = MessageBox(NULL, "would you like to run in fullscreen mode?", "eX0", MB_YESNO | MB_ICONQUESTION) == IDYES;
#endif

	// create the window
	glfwGetDesktopMode(&oDesktopMode);
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 32);
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	if (glfwOpenWindow(640, 480, 8, 8, 8, 0, 24, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW)
			&& glfwGetWindowParam(GLFW_ACCELERATED) && glfwGetWindowParam(GLFW_STENCIL_BITS) || CloseBadWindow())
	{
		printf("Opened a window (try 1)...\n");
	}
	else if (glfwOpenWindow(640, 480, 8, 8, 8, 8, 24, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW)
			&& glfwGetWindowParam(GLFW_ACCELERATED) && glfwGetWindowParam(GLFW_STENCIL_BITS) || CloseBadWindow())
	{
		printf("Opened a window (try 2)...\n");
	}
	else if (glfwOpenWindow(640, 480, 5, 6, 5, 0, 24, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW)
			&& glfwGetWindowParam(GLFW_ACCELERATED) && glfwGetWindowParam(GLFW_STENCIL_BITS) || CloseBadWindow())
	{
		printf("Opened a window (try 3)...\n");
	}
	else if (glfwOpenWindow(640, 480, 5, 6, 5, 0, 8, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW)
			&& glfwGetWindowParam(GLFW_ACCELERATED) && glfwGetWindowParam(GLFW_STENCIL_BITS) || CloseBadWindow())
	{
		printf("Opened a window (try 4)...\n");
	}
	else if (glfwOpenWindow(640, 480, 5, 6, 5, 0, 16, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW)
			&& glfwGetWindowParam(GLFW_ACCELERATED) && glfwGetWindowParam(GLFW_STENCIL_BITS) || CloseBadWindow())
	{
		printf("Opened a window (try 5)...\n");
	}
	else {
		printf("Couldn't open an accelerated window...\n");
		/*if (glfwGetWindowParam(GLFW_OPENED)) glfwCloseWindow();
		return false;*/
		if (!glfwGetWindowParam(GLFW_OPENED)) return false;
	}
	//glfwSetWindowPos(oDesktopMode.Width / 2 - 320, oDesktopMode.Height / 2 - 240);
	glfwSetWindowPos(oDesktopMode.Width - 650, oDesktopMode.Height / 2 - 240);
	glfwSetWindowTitle(((string)"eX0 v0.0 (Built on " + __DATE__ + " at " + __TIME__ + ")").c_str());	// set the window title
	glfwSwapInterval(1);		// Turn V-Sync on

	// init OpenGL
	if (!OglUtilsInitGL()) {
		printf("OpenGL initilization failed.\n");
		return false;
	}

	stringstream x;
	x << "CPU Count: " << glfwGetNumberOfProcessors()
	  << "\nGL Renderer: " << glGetString(GL_VENDOR) << " " << glGetString(GL_RENDERER) << " v" << glGetString(GL_VERSION)
	  << "\nGLFW_ACCELERATED: " << glfwGetWindowParam(GLFW_ACCELERATED)
	  << "\nGLFW_RED_BITS: " << glfwGetWindowParam(GLFW_RED_BITS)
	  << "\nGLFW_GREEN_BITS: " << glfwGetWindowParam(GLFW_GREEN_BITS)
	  << "\nGLFW_BLUE_BITS: " << glfwGetWindowParam(GLFW_BLUE_BITS)
	  << "\nGLFW_ALPHA_BITS: " << glfwGetWindowParam(GLFW_ALPHA_BITS)
	  << "\nGLFW_DEPTH_BITS: " << glfwGetWindowParam(GLFW_DEPTH_BITS)
	  << "\nGLFW_STENCIL_BITS: " << glfwGetWindowParam(GLFW_STENCIL_BITS)
	  << "\nGLFW_REFRESH_RATE: " << glfwGetWindowParam(GLFW_REFRESH_RATE)
	  << "\nGLFW_FSAA_SAMPLES: " << glfwGetWindowParam(GLFW_FSAA_SAMPLES);
	//MessageBox(NULL, x.str().c_str(), "MessageBox1", NULL);
	printf("%s\n", x.str().c_str());

	// sub-init
	SetGlfwCallbacks();
	GameDataLoad();						// load game data
	WeaponInitSpecs();
	pChatMessages = new CHudMessageQueue(0, 480 - 150, 5, 7.5f);
	//nPlayerCount = 0; PlayerInit();		// Initialize the players
	pTimedEventScheduler = new CTimedEventScheduler();
	if (!NetworkInit())					// Initialize the networking
		return false;

	SyncRandSeed();

	return true;
}

// deinitialization
void Deinit()
{
	printf("Deinit\n");

	// show the mouse cursor
	glfwEnable(GLFW_MOUSE_CURSOR);

	// sub-deinit
	delete pTimedEventScheduler; pTimedEventScheduler = NULL;
	NetworkDeinit();					// Shutdown the networking component
	nPlayerCount = 0; PlayerInit();		// delete all players
	delete pChatMessages; pChatMessages = NULL;
	GameDataUnload();					// unload game data
	OglUtilsDeinitGL();					// Deinit OpenGL stuff
	// ...

	// close the window
	if (glfwGetWindowParam(GLFW_OPENED))
		glfwCloseWindow();

	// terminate glfw
	glfwTerminate();
}

// resize the window callback function
void GLFWCALL ResizeWindow(int iWidth, int iHeight)
{
	printf("ResizeWindow to %dx%d.\n", iWidth, iHeight);
	if ((iWidth * iHeight != 0) && (iWidth != 640 || iHeight != 480)) {
#ifdef WIN32
		MessageBox(NULL, "Refusing to run in non-native resolution (for now).", "eX0", MB_OK);
#else
		printf("Refusing to run in non-native resolution (for now).\n");
#endif
		//Terminate(1);		// Refuse to run in non-native resolution, for now
	}
}

// set glfw callback functions
void SetGlfwCallbacks()
{
	glfwSetWindowSizeCallback(ResizeWindow);
	glfwSetKeyCallback(InputProcessKey);
	glfwSetCharCallback(InputProcessChar);
	glfwSetMouseButtonCallback(InputProcessMouse);
}

// Restarts the game
void RestartGame()
{
	// Reset the players
	//PlayerInit();

	// Reset the particle engine
	oParticleEngine.Reset();

	// DEBUG - set temp ai and position players
	//oPlayers[0]->Position(125, 50);
	//oPlayers[0]->Position(20, 262);
	//oPlayers[0]->SetZ(0.8f);
	for (int nLoop1 = 0; nLoop1 < nPlayerCount; ++nLoop1) {
		float x, y;
		do {
			x = static_cast<float>(rand() % 2000 - 1000);
			y = static_cast<float>(rand() % 2000 - 1000);
		} while (ColHandIsPointInside((int)x, (int)y) || !ColHandCheckPlayerPos(&x, &y));
		oPlayers[nLoop1]->Position(x, y, 0);
		oPlayers[nLoop1]->SetZ(0.001f * (rand() % 1000) * Math::TWO_PI);
		oPlayers[nLoop1]->SetTeam(iLocalPlayerID == nLoop1 ? 0 : 1);
	}

	printf("Game restarted. ============================\n");
}

// syncronizes random seed with all clients
void SyncRandSeed(void)
{
	// DEBUG: This has no effect, due to PolyBoolean doing srand(clock()) anyway
	//srand(237);
	//srand(static_cast<unsigned int>(time(NULL)));
}

// quits
void Terminate(int nExitCode)
{
	if (counter1 != counter2) printf("WARNING!!!!: counter1 = %d != counter2 = %d\n", counter1, counter2);

	if (nExitCode != 0) {
		// deinit
		Deinit();

		// DEBUG: Print out the memory usage stats
		//m_dumpMemoryReport();

		exit(nExitCode);
	} else {
		if (glfwGetWindowParam(GLFW_OPENED)) glfwCloseWindow();
		else Terminate(10);
	}
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



	/*multimap<int, int> myset;
	multimap<int, int>::iterator it1;
	vector<multimap<int, int>::iterator> its;

	// insert some values:
	for (int i=1; i<10; i++) myset.insert(pair<int, int>(i*10, i));  // 10 20 30 40 50 60 70 80 90

	for (it1 = myset.begin(); it1 != myset.end(); ++it1) {
		its.push_back(it1);
	}

	printf("myset contains:"); for (it1 = myset.begin(); it1 != myset.end(); ++it1) printf(" %d", it1->first); printf("\n");
	printf("  its point to:"); for (vector<multimap<int, int>::iterator>::iterator it2 = its.begin(); it2 != its.end(); ++it2) printf(" %d", (*it2)->first); printf("\n");

	printf("removing 5th elem..\n");
	myset.erase(its.at(4));
	its.erase(its.begin() + 4);

	printf("myset contains:"); for (it1 = myset.begin(); it1 != myset.end(); ++it1) printf(" %d", it1->first); printf("\n");
	printf("  its point to:"); for (vector<multimap<int, int>::iterator>::iterator it2 = its.begin(); it2 != its.end(); ++it2) printf(" %d", (*it2)->first); printf("\n");

	printf("removing 2nd elem..\n");
	myset.erase(its.at(1));
	its.erase(its.begin() + 1);

	printf("removing 6th elem..\n");
	myset.erase(its.at(5));
	its.erase(its.begin() + 5);

	printf("myset contains:"); for (it1 = myset.begin(); it1 != myset.end(); ++it1) printf(" %d", it1->first); printf("\n");
	printf("  its point to:"); for (vector<multimap<int, int>::iterator>::iterator it2 = its.begin(); it2 != its.end(); ++it2) printf(" %d", (*it2)->first); printf("\n");*/



	/*MovingAverage	avg(60.0, 5);
	int myints[] = { 10, 21, 30, 40, 50, 90 };
	list<int> mylist (myints,myints + 6);
	list<int>::iterator it1;
	for (list<int>::iterator it=mylist.begin(); it != mylist.end(); ++it)
		avg.push(*it, *it);
	avg.Print();

	printf("%.5lf\n", avg.WeightedMovingAverage());
	printf("%d\n", avg.Signum());


return 0;*/


	// Print the version and date/time built
	printf("eX0 v0.0 - Built on %s at %s.\n\n", __DATE__, __TIME__);

	// initialize
	if (!Init(argc, argv))
		Terminate(1);

	// DEBUG - This is a hack - we only need to position the players here... But I called RestartGame() to do that
	//RestartGame();

	// DEBUG: Set the local player name through the 2nd command line param
	if (argc >= 3)
		sLocalPlayerName = argv[2];

	// Connect to the server
	if (argc >= 2)
	{
		NetworkConnect(argv[1], DEFAULT_PORT);
	}

	//GetReady();
	{
		// hide the mouse cursor and put in in center
		/*glfwGetMousePos(&nDesktopCursorX, &nDesktopCursorY);
		glfwDisable(GLFW_MOUSE_CURSOR);
		glfwSetMousePos(320, 240);*/

		// make sure that the physics won't count all the time passed during init
		dBaseTime = glfwGetTime();
		dFpsBaseTime = dBaseTime;
	}

	// start the main loop
	while (glfwGetWindowParam(GLFW_OPENED) /*&& glfwGetWindowParam(GLFW_ACTIVE)*/)
	{
		// time passed calcs
		dCurTime = glfwGetTime();
		//if (!bPaused) dTimePassed = dCurTime - dBaseTime; else dTimePassed = 0;
		dTimePassed = dCurTime - dBaseTime;
		dBaseTime = dCurTime;

		// fps calcs
		iFpsFrames++;
		dFpsTimePassed = dCurTime - dFpsBaseTime;
		if (dFpsTimePassed >= 0.75)
		{
			sFpsString = (string)"eX0 - " + ftos(iFpsFrames / (float)dFpsTimePassed) + " fps";
			//glfwSetWindowTitle(sTempString.c_str());
			dFpsBaseTime = dCurTime;
			iFpsFrames = 0;
		}

		// clear the buffer
		OglUtilsSwitchMatrix(WORLD_SPACE_MATRIX);
		glLoadIdentity();
		glClear(GL_COLOR_BUFFER_BIT);

		if (!iGameState)
		// in game
		{
++counter2;
			// mouse moved?
			InputMouseMovCalcs();

			// key or mouse button held down?
			InputKeyHold();
			InputMouseHold();

			// player tick
			if (bPaused) { fTempFloat = (float)dTimePassed; dTimePassed = 0.00001; }
			//PlayerTick();
glfwLockMutex(oPlayerTick);
			if (PlayerGet(iLocalPlayerID)->GetTeam() != 2 && !PlayerGet(iLocalPlayerID)->IsDead()) {
				PlayerGet(iLocalPlayerID)->Tick();
			} else {
				PlayerGet(iLocalPlayerID)->FakeTick();
			}
glfwUnlockMutex(oPlayerTick);

			// particle engine tick
			oParticleEngine.Tick();
			if (bPaused) dTimePassed = fTempFloat;

			// render the static scene
			// TODO: Don't do any rendering when the window is GLFW_ICONIFIED (minimized)
			RenderStaticScene();

			// render the interactive scene
			//RenderInteractiveScene();

			// render the fov zone
			if (bStencilOperations) RenderFOV();

			// Enable the FOV masking
			OglUtilsSetMaskingMode(WITH_MASKING_MODE);

			// render all players
glfwLockMutex(oPlayerTick);
			//PlayerGet(iLocalPlayerID)->RenderInPast(2 * oPlayers[iLocalPlayerID]->GetZ());
			//PlayerGet(iLocalPlayerID)->RenderInPast(Math::TWO_PI - 2 * oPlayers[iLocalPlayerID]->GetZ());
			//for (int i = 1; i <= 100; i += 1) PlayerGet(0/*iLocalPlayerID*/)->RenderInPast(i * 0.1f);
			//PlayerGet(iLocalPlayerID)->RenderInPast(kfInterpolate);
glfwUnlockMutex(oPlayerTick);
			RenderPlayers();

			// render all particles
			RenderParticles();

			// Disable the masking
			OglUtilsSetMaskingMode(NO_MASKING_MODE);

			// render HUD
			RenderHUD();
		}
		else
		// in menus
		{
			//InputMouseMovCalcs();

			//MainMenuRender();
		}

		// finish it up
		glFlush();
		//glfwPollEvents();
		glfwSwapBuffers();

		if (glfwGetWindowParam(GLFW_ACTIVE))
			glfwSleep(0.0);
		else
			glfwSleep(0.015);
	}

	// Clean up and exit nicely
	Deinit();

	printf("Returning 0 from main(). :)\n");
	return 0;
}
