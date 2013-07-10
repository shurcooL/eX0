#include "globals.h"

// no console window
//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#pragma comment(linker, "/NODEFAULTLIB:\"LIBCMT\"")
//#pragma comment(linker, "/NODEFAULTLIB:\"LIBC\"")

volatile int	iGameState = 1;
bool			bPaused = false;

bool			bWireframe = false;
bool			bUseDefaultTriangulation = false;
bool			bStencilOperations = false;

GLFWvidmode		oDesktopMode;
bool			bFullscreen = false;

float			fTimePassed = 0;
float			fCurTime = 0;
float			fBaseTime = 0;
int				iFpsFrames = 0;
float			fFpsBaseTime = 0;
float			fFpsTimePassed = 0;
string			sFpsString = (string)"eX0";

string			sTempString = (string)"";
float			fTempFloat = 0;
int				iTempInt = 0;

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

	// let the use choose whether to run in fullscreen mode
	bFullscreen = false;
	if (argc >= 2 && strcmp(argv[1], "--fullscreen") == 0) bFullscreen = true;
#ifdef WIN32
	//else bFullscreen = MessageBox(NULL, "would you like to run in fullscreen mode?", "eX0", MB_YESNO | MB_ICONQUESTION) == IDYES;
#endif

	// create the window
	glfwGetDesktopMode(&oDesktopMode);
	//if (!glfwOpenWindow(640, 480, 5, 6, 5, 0, 24, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW))
	if (!glfwOpenWindow(640, 480, 8, 8, 8, 0, 24, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW))
		return false;
	//glfwSetWindowPos(oDesktopMode.Width / 2 - 320, oDesktopMode.Height / 2 - 240);
	glfwSetWindowPos(oDesktopMode.Width - 640, oDesktopMode.Height / 2 - 240);
	glfwSetWindowTitle(((string)"eX0 v0.0 (Built on " + __DATE__ + " at " + __TIME__ + ")").c_str());	// set the window title

	glfwSwapInterval(0);		// Turn V-Sync off

	stringstream x;
	x << "GLFW_ACCELERATED: " << glfwGetWindowParam(GLFW_ACCELERATED)
	  << "\nGLFW_RED_BITS: " << glfwGetWindowParam(GLFW_RED_BITS)
	  << "\nGLFW_GREEN_BITS: " << glfwGetWindowParam(GLFW_GREEN_BITS)
	  << "\nGLFW_BLUE_BITS: " << glfwGetWindowParam(GLFW_BLUE_BITS)
	  << "\nGLFW_ALPHA_BITS: " << glfwGetWindowParam(GLFW_ALPHA_BITS)
	  << "\nGLFW_DEPTH_BITS: " << glfwGetWindowParam(GLFW_DEPTH_BITS)
	  << "\nGLFW_STENCIL_BITS: " << glfwGetWindowParam(GLFW_STENCIL_BITS)
	  << "\nGLFW_REFRESH_RATE: " << glfwGetWindowParam(GLFW_REFRESH_RATE);
	//MessageBox(NULL, x.str().c_str(), "MessageBox1", NULL);
	printf("%s\n", x.str().c_str());

	// hide the mouse cursor and put in in center
	glfwDisable(GLFW_MOUSE_CURSOR);
	glfwSetMousePos(320, 240);

	// sub-init
	SetGlfwCallbacks();
	GameDataLoad();						// load game data
	WeaponInitSpecs();
	//nPlayerCount = 0; PlayerInit();		// Initialize the players
	if (!NetworkInit())					// Initialize the networking
		return false;
	SyncRandSeed();

	// init OpenGL
	if (!OglUtilsInitGL()) {
		//ERROR_HANDLER.SetLastError("OpenGL initilization failed.");
		printf("OpenGL initilization failed.\n");
		return false;
	}

	// set global variables
	// ...

	// make sure that the physics won't count all the time passed during init
	fBaseTime = static_cast<float>(glfwGetTime());

	return true;
}

// deinitialization
void Deinit()
{
	printf("Deinit\n");

	// show the mouse cursor
	glfwEnable(GLFW_MOUSE_CURSOR);

	// sub-deinit
	NetworkDeinit();					// Shutdown the networking component
	GameDataUnload();					// unload game data
	OglUtilsDeinitGL();					// Deinit OpenGL stuff
	nPlayerCount = 0; PlayerInit();		// delete all players
	// ...

	// close the window
	if (glfwGetWindowParam(GLFW_OPENED))
		glfwCloseWindow();

	// terminate glfw
	glfwTerminate();
}

// quits
void Terminate(int nExitCode)
{
	// deinit
	Deinit();

	// DEBUG: Print out the memory usage stats
	m_dumpMemoryReport();

	exit(nExitCode);
}

// resize the window callback function
void GLFWCALL ResizeWindow(int iWidth, int iHeight)
{
	if (iWidth != 640 || iHeight != 480)
		Terminate(1);		// Refuse to run in non-native resolution, for now
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
		oPlayers[nLoop1]->Position(x, y);
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

// main function
int main(int argc, char *argv[]) 
{
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

	// start the main loop
	while (glfwGetWindowParam(GLFW_OPENED) /*&& glfwGetWindowParam(GLFW_ACTIVE)*/)
	{
		// Pause the execution if the window isn't active, and resume when it's activated
		if (!glfwGetWindowParam(GLFW_ACTIVE)) {
			glfwEnable(GLFW_MOUSE_CURSOR);
			glfwSleep(0.1);
			//glfwPollEvents();
			glfwSwapBuffers();
			if (glfwGetWindowParam(GLFW_ACTIVE)) {
				glfwDisable(GLFW_MOUSE_CURSOR);
				glfwSetMousePos(320, 240);
				fBaseTime = (float)glfwGetTime();
			}
			continue;
		}

		// time passed calcs
		fCurTime = static_cast<float>(glfwGetTime());
		//if (!bPaused) fTimePassed = fCurTime - fBaseTime; else fTimePassed = 0;
		fTimePassed = fCurTime - fBaseTime;
		fBaseTime = fCurTime;

		// fps calcs
		iFpsFrames++;
		fFpsTimePassed = fCurTime - fFpsBaseTime;
		if (fFpsTimePassed >= 0.75)
		{
			sFpsString = (string)"eX0 - " + ftos(iFpsFrames / fFpsTimePassed) + " fps";
			//glfwSetWindowTitle(sTempString.c_str());
			fFpsBaseTime = fCurTime;
			iFpsFrames = 0;
		}

		// clear the buffer
		OglUtilsSwitchMatrix(WORLD_SPACE_MATRIX);
		glLoadIdentity();
		glClear(GL_COLOR_BUFFER_BIT);

		if (!iGameState)
		// in game
		{
			// mouse moved?
			InputMouseMovCalcs();

			// key or mouse button held down?
			InputKeyHold();
			InputMouseHold();

			// player tick
			//PlayerTick();
			PlayerGet(iLocalPlayerID)->Tick();

			// particle engine tick
			if (bPaused) { fTempFloat = fTimePassed; fTimePassed = 0; }
			oParticleEngine.Tick();
			if (bPaused) fTimePassed = fTempFloat;

			// render the static scene
			RenderStaticScene();

			// render the interactive scene
			//RenderInteractiveScene();

			// render the fov zone
			if (bStencilOperations) RenderFOV();

			// Enable the FOV masking
			OglUtilsSetMaskingMode(WITH_MASKING_MODE);

			// render all players
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

		glfwSleep(0.0);
	}

	Terminate(0);

	return 0;
}
