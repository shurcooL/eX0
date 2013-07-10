#include "globals.h"

#if !defined(EX0_DEBUG) && defined(WIN32)
	// no console window
//#	pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

volatile bool	bProgramRunning = true;
volatile int	iGameState = 1;
bool			bPaused = false;

bool			bWireframe = false;
bool			bUseDefaultTriangulation = true;
bool			bStencilOperations =
#ifdef EX0_DEBUG
									 false;
#else
									 true;
#endif // EX0_DEBUG

GLFWvidmode		oDesktopMode;
bool			bFullscreen = false;

/*double			dTimePassed = 0;
double			dCurTime = 0;
double			dBaseTime = 0;*/
std::string		sFpsString = std::string();

std::string		sTempString = std::string();
float			fTempFloat = 0;
int				iTempInt = 0;

int				nGlobalExitCode = 0;

// DEBUG: A hack to decide whether or not to run local server
int nRunModeDEBUG;
bool bWindowModeDEBUG = true;

void eX0_assert(bool expression, string message, bool fatal)
{
	if (!expression) {
		printf("\nAssertion FAILED: '%s'\n\n", message.c_str());
		if (fatal) exit(51);
	}
}

bool CheckWindow()
{
	if (glfwGetWindowParam(GLFW_ACCELERATED) && glfwGetWindowParam(GLFW_STENCIL_BITS))
		return true;
	else {
		if (glfwGetWindowParam(GLFW_OPENED)) glfwCloseWindow();
		return false;
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

	if (bWindowModeDEBUG)
	{
		// let the use choose whether to run in fullscreen mode
		bFullscreen = false;
		//if (argc >= 2 && strcmp(argv[1], "--fullscreen") == 0) bFullscreen = true;
#ifdef WIN32
		//bFullscreen = MessageBox(NULL, "would you like to run in fullscreen mode?", EX0_BUILD_STRING, MB_YESNO | MB_ICONQUESTION) == IDYES;
#endif

		// create the window
		glfwGetDesktopMode(&oDesktopMode);
		glfwOpenWindowHint(GLFW_FSAA_SAMPLES,
#ifdef EX0_DEBUG
											  8);
#else
											  0);
#endif // EX0_DEBUG
		glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
		//if (glfwOpenWindow(1920, 1200, 8, 8, 8, 0, 24, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW) && CheckWindow()) { printf("Opened a window (try 0)...\n"); } else
		if (glfwOpenWindow(640, 480, 8, 8, 8, 0, 24, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW) && CheckWindow())
		{
			printf("Opened a window (try 1)...\n");
		}
		else if (glfwOpenWindow(640, 480, 8, 8, 8, 8, 24, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW) && CheckWindow())
		{
			printf("Opened a window (try 2)...\n");
		}
		else if (glfwOpenWindow(640, 480, 5, 6, 5, 0, 24, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW) && CheckWindow())
		{
			printf("Opened a window (try 3)...\n");
		}
		else if (glfwOpenWindow(640, 480, 5, 6, 5, 0, 8, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW) && CheckWindow())
		{
			printf("Opened a window (try 4)...\n");
		}
		else if (glfwOpenWindow(640, 480, 5, 6, 5, 0, 16, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW) && CheckWindow())
		{
			printf("Opened a window (try 5)...\n");
		}
		else {
			printf("ERROR: Couldn't open an accelerated window...\n");
			/*if (glfwGetWindowParam(GLFW_OPENED)) glfwCloseWindow();
			return false;*/
			if (!glfwGetWindowParam(GLFW_OPENED)) return false;
		}
		glfwSwapInterval(1);		// Turn V-Sync on
		//glfwSetWindowPos(oDesktopMode.Width / 2 - 320, oDesktopMode.Height / 2 - 240);
		glfwSetWindowPos(oDesktopMode.Width - 650, oDesktopMode.Height / 2 - 240);
		glfwSetWindowTitle(EX0_BUILD_STRING);	// set the window title

		// init OpenGL
		if (!OglUtilsInitGL()) {
			printf("OpenGL initilization failed.\n");
			return false;
		}

		SetGlfwCallbacks();

		std::stringstream x;
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
	}

	// Initialize components
	if (!GameDataLoad()) {				// load game data
		printf("Couldn't load game data.\n");
		return false;
	}
	WeaponInitSpecs();
	//nPlayerCount = 255;					// Set the max player limit for this server
	nPlayerCount = 16;					// Set the max player limit for this server
	if (bWindowModeDEBUG) pChatMessages = new CHudMessageQueue(0, 480 - 150, 5, 7.5f);
	//FpsCounter::Initialize();
	if (bWindowModeDEBUG)
		g_pInputManager = new InputManager();
	g_pGameSession = new GameSession();
	if (!NetworkInit()) {				// Initialize the networking
		printf("Couldn't initialize the networking.\n");
		return false;
	}
	pTimedEventScheduler = new CTimedEventScheduler();
	pGameLogicThread = new GameLogicThread();
	if (nRunModeDEBUG != 0)
		pGameServer = new GameServer(nRunModeDEBUG != 3);

	SyncRandSeed();

	return true;
}

// deinitialization
void Deinit()
{
	printf("Deinit\n");

	// Sub-deinit
	delete pGameLogicThread; pGameLogicThread = nullptr;
	delete pTimedEventScheduler; pTimedEventScheduler = nullptr;
	delete pGameServer; pGameServer = nullptr;
	NetworkDeinit();					// Shutdown the networking component
	// DEBUG: Wrong place? This is done twice on Server-instance, once on Client-instance... Refactor this thoroughly
	ClientConnection::CloseAll();
	delete g_pGameSession; g_pGameSession = nullptr;
	delete g_pInputManager; g_pInputManager = nullptr;
	FpsCounter::DeleteAll();//FpsCounter::Deinitialize();
	CPlayer::DeleteAll();				// Delete all players
	delete pChatMessages; pChatMessages = nullptr;
	GameDataUnload();					// unload game data
	if (bWindowModeDEBUG) OglUtilsDeinitGL();					// Deinit OpenGL stuff
	// ...

	if (bWindowModeDEBUG) {
		// close the window
		if (glfwGetWindowParam(GLFW_OPENED))
			glfwCloseWindow();
	}

	// terminate glfw
	glfwTerminate();
}

// resize the window callback function
void GLFWCALL WindowSizeCallback(int iWidth, int iHeight)
{
	printf("ResizeWindow to %dx%d.\n", iWidth, iHeight);
	if (((iWidth | iHeight) != 0) && (iWidth != 640 || iHeight != 480)) {
#ifdef WIN32
		//MessageBox(NULL, "Refusing to run in non-native resolution (for now).", EX0_BUILD_STRING, MB_OK);
#else
		printf("Refusing to run in non-native resolution (for now).\n");
#endif
		//Terminate(1);		// Refuse to run in non-native resolution, for now
	}
}

int GLFWCALL WindowCloseCallback()
{
	bProgramRunning = false;

	return GL_FALSE;
}

// set glfw callback functions
void SetGlfwCallbacks()
{
	glfwSetWindowSizeCallback(&WindowSizeCallback);
	glfwSetWindowCloseCallback(&WindowCloseCallback);
}

// Restarts the game
/*void RestartGame()
{
	// Reset the players
	//PlayerInit();

	// Reset the particle engine
	oParticleEngine.Reset();

	// DEBUG - set temp ai and position players
	//oPlayers[0]->Position(125, 50);
	//oPlayers[0]->Position(20, 262);
	//oPlayers[0]->SetZ(0.8f);
	for (u_int nLoop1 = 0; nLoop1 < nPlayerCount; ++nLoop1) {
		float x, y;
		do {
			x = static_cast<float>(rand() % 2000 - 1000);
			y = static_cast<float>(rand() % 2000 - 1000);
		} while (ColHandIsPointInside((int)x, (int)y) || !ColHandCheckPlayerPos(&x, &y));
		PlayerGet(nLoop1)->Position(x, y, 0);
		PlayerGet(nLoop1)->SetZ(0.001f * (rand() % 1000) * Math::TWO_PI);
		PlayerGet(nLoop1)->SetTeam(iLocalPlayerID == nLoop1 ? 0 : 1);
	}

	printf("Game restarted. ============================\n");
}*/

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

#ifdef WIN32
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
	if (CTRL_C_EVENT == dwCtrlType || CTRL_BREAK_EVENT == dwCtrlType || CTRL_CLOSE_EVENT == dwCtrlType
		|| CTRL_LOGOFF_EVENT == dwCtrlType || CTRL_SHUTDOWN_EVENT == dwCtrlType)
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

// DEBUG
void DumpStateHistory(std::list<AuthState_st> & oStateHistory)
{
	printf("=== oStateHistory DUMP ====================\n");

	double dCurrentTime = g_pGameSession->MainTimer().GetTime();
	double dCurrentTimepoint = dCurrentTime / (256.0 / g_cCommandRate);
	dCurrentTimepoint -= static_cast<uint32>(dCurrentTimepoint);
	dCurrentTimepoint *= 256;

	printf("dCurrentTimepoint = %f\n", dCurrentTimepoint);
	printf("size %d\n", oStateHistory.size());
	printf("front 2nd %d: (%f, %f, %f)\n", oStateHistory.begin().operator ++().operator *().oState.cSequenceNumber, oStateHistory.begin().operator ++().operator *().oState.oState.fX, oStateHistory.begin().operator ++().operator *().oState.oState.fY, oStateHistory.begin().operator ++().operator *().oState.oState.fZ);
	printf("front 1st %d: (%f, %f, %f)\n", oStateHistory.front().oState.cSequenceNumber, oStateHistory.front().oState.oState.fX, oStateHistory.front().oState.oState.fY, oStateHistory.front().oState.oState.fZ);
	printf("GameSession->GlobalStateSequenceNumberTEST %d\n", g_pGameSession->GlobalStateSequenceNumberTEST);

	printf("\n");
}

std::string unescape(char * cstr)
{
	std::string str;
	for (uint32 ch = 0; ch < strlen(cstr); ++ch)
	{
		if (cstr[ch] == '"')
			str = str + "\\\"";
		else
			str = str + cstr[ch];
	}
	return str;
}

// main function
int main(int argc, char * argv[])
{
	eX0_assert(sizeof(int8)*8 == 8 && sizeof(uint8)*8 == 8, "sizeof(u/int8) incorrect", true);
	eX0_assert(sizeof(int16)*8 == 16 && sizeof(uint16)*8 == 16, "sizeof(u/int16) incorrect", true);
	eX0_assert(sizeof(int32)*8 == 32 && sizeof(uint32)*8 == 32, "sizeof(u/int32) incorrect", true);
	eX0_assert(sizeof(int64)*8 == 64 && sizeof(uint64)*8 == 64, "sizeof(u/int64) incorrect", true);

	/*printf("^%s^\n", GetCommandLine());
	for (int i = 0; i < argc; ++i)
		printf("param %d: ^%s^\n", i, argv[i]);
	printf("done. press enter."); getchar();
	printf("exiting.\n");
	return 0;*/

	/*for (int i = 0; i < argc; ++i)
		printf("param %d: ^%s^\n", i, argv[i]);
	printf("\n");

	// Re-run the exe in a cmd environment, so that the console window won't get closed right away
	// NOTE: All original command line parameters get preserved
	char * runcode = "-superdupercodethatallowseX0torunforreal";
	if (argc >= 2 && 0 == strcmp(argv[argc - 1], runcode))
		argc -= 1;		// Ignore the runcode parameter
	else {
		std::string cmd = "(";
		for (int i = 0; i < argc; ++i) {
			/*if (nullptr == strchr(argv[i], ' '))	// No space in parameter
				cmd = cmd + argv[i] + " ";
			else									// Space in parameter* /
				cmd = cmd + "\"" + unescape(argv[i]) + "\" ";
		}
		cmd = cmd + runcode + ") & echo. & echo If anything went wrong, please copy the log above and send it to shurcooL. & pause";
		printf("Executing ^%s^\n", cmd.c_str());
		system(cmd.c_str());
		return 0;
	}

	for (int i = 0; i < argc; ++i)
		printf("param %d: ^%s^\n", i, argv[i]);
	return 0;*/

	// Re-run the exe in a cmd environment, so that the console window won't get closed right away
	// NOTE: At this time, all command lines parameters get stripped
	/*char * runcode = "-superdupercodethatallowseX0torunforreal";
	if (2 == argc && 0 == strcmp(argv[1], runcode))
		argc = 1;		// Ignore the runcode parameter
	else {
		std::string cmd; cmd = cmd + "(start /b eX0 " + runcode + ") & echo. & echo If anything went wrong, please copy the log above and send it to shurcooL.";
		system(cmd.c_str());
		return 0;
	}*/
	/*STARTUPINFO siStartupInfo; memset(&siStartupInfo, 0, sizeof(siStartupInfo)); siStartupInfo.cb = sizeof(siStartupInfo);
	PROCESS_INFORMATION piProcessInfo; memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	//std::string cmd; cmd = cmd + "cmd /C \"\"" + argv[0] + "\" " + "-param1" + " & echo. & echo If anything went wrong, please copy the log above and send it to shurcooL. & pause";
	std::string cmd; cmd = cmd + "cmd /C \"\"" + "C:\\Work\\Generation 10\\eX0\\eX0 simple.exe" + "\" -param1 \"-param two\" & pause\"";
	return CreateProcess(NULL, const_cast<char *>(cmd.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &siStartupInfo, &piProcessInfo);
	//return CreateProcess("C:\\Windows\\system32\\cmd.exe", "/C calc.exe", NULL, NULL, FALSE, NULL, NULL, NULL, &siStartupInfo, &piProcessInfo);
	//printf("error: %d\n", GetLastError());
	//return 0;*/

#if !defined(EX0_DEBUG) && defined(WIN32)
	// Re-run the exe in a cmd environment, so that the console window won't get closed right away
	// NOTE: At this time, all command lines parameters get stripped
	char * runcode = "-superdupercodethatallowseX0torunforreal";
	if (2 == argc && 0 == strcmp(argv[1], runcode))
		argc = 1;		// Ignore the runcode parameter
	else {
		std::string cmd; cmd = cmd + "\"" + argv[0] + "\" " + runcode + " & echo. & echo If anything went wrong, please copy the log above and send it to shurcooL. & pause";
		system(cmd.c_str());
		/*STARTUPINFO siStartupInfo; memset(&siStartupInfo, 0, sizeof(siStartupInfo)); siStartupInfo.cb = sizeof(siStartupInfo);
		PROCESS_INFORMATION piProcessInfo; memset(&piProcessInfo, 0, sizeof(piProcessInfo));
		std::string cmd; cmd = cmd + "cmd /C \"\"" + argv[0] + "\" " + runcode + " & echo. & echo If anything went wrong, please copy the log above and send it to shurcooL. & pause\"";
		CreateProcess(NULL, const_cast<char *>(cmd.c_str()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &siStartupInfo, &piProcessInfo);*/
		return 0;
	}
#endif

	// Add a Ctrl+C signal handler, for abrupt termination
#ifdef WIN32
	SetConsoleCtrlHandler(&ConsoleCtrlHandler, TRUE);
#else
	signal(SIGINT, &signal_handler);
	signal(SIGHUP, &signal_handler);
#endif



	// ...
//return 0;



	// Print the version and date/time built
	printf("%s\n\n", EX0_BUILD_STRING);

#ifdef EX0_DEBUG
	char ch;
	ch = static_cast<char>(getchar());
	//ch = 'n';
	if (ch == 'c') nRunModeDEBUG = 0;			// Client only
	else if (ch == 's') nRunModeDEBUG = 1;		// Server only
	else if (ch == 'b') nRunModeDEBUG = 2;		// Both server and client
	else if (ch == 'n') nRunModeDEBUG = 3;		// Off-line client/server (no network)
	else if (ch == 'd') { nRunModeDEBUG = 1; bWindowModeDEBUG = false; }	// Dedicated server (no window)
	else { printf("Invalid mode, exiting.\n"); return 0; }
#else
	nRunModeDEBUG = 0;		// Force client-only functionality
	argc = 1;				// Don't allow any command line arguments
#endif // EX0_DEBUG

	// DEBUG: Set the level name through the 3rd command line param
	if (argc >= 4)
		sLevelName = argv[3];

	// Initialize
	if (!Init(argc, argv))
		Terminate(1);

	// DEBUG: Set the local player name through the 2nd command line param
	if (nRunModeDEBUG != 1)
	{
		if (argc >= 3)
			sLocalPlayerName = argv[2];
#ifndef EX0_DEBUG
		else {
			CHAR lpBuffer[256+1];
			DWORD lpnSize = 256+1;
			if (GetUserName(lpBuffer, &lpnSize))
				sLocalPlayerName = lpBuffer;
		}
#endif // EX0_DEBUG
	}

	// Connect to the server
	if (nRunModeDEBUG == 0)	// Client only
	{
		glfwSetWindowPos(oDesktopMode.Width - 650 * 2, oDesktopMode.Height / 2 - 240);
		if (!(argc >= 2 && NetworkConnect(argv[1], DEFAULT_PORT)) &&
			!NetworkConnect("shurcool.no-ip.org"/*"cse.yorku.ca"*/, DEFAULT_PORT))
		{
			Terminate(1);
		}
		//NetworkConnect("shurvaio", DEFAULT_PORT);
	} else if (nRunModeDEBUG == 2)			// Both server and client
	{
		// Connect to local server
		NetworkConnect(NULL, 0);
		//pServer = new LocalServerConnection();
		pServer->SetJoinStatus(IN_GAME);

glfwLockMutex(oPlayerTick);
		pLocalPlayer = new CPlayer(0);
		pLocalPlayer->SetName(sLocalPlayerName);
		pLocalPlayer->m_pController = new HidController(*pLocalPlayer);
		/*{
			PlayerInputListener * pPlayerInputListener = new PlayerInputListener();
			g_pInputManager->RegisterListener(pPlayerInputListener);

			HidController * pHidController = new HidController(*pLocalPlayer);
			pHidController->m_pPlayerInputListener = pPlayerInputListener;

			pLocalPlayer->m_pController = pHidController;
		}*/
		pLocalPlayer->m_pStateAuther = new LocalStateAuther(*pLocalPlayer);
		(new LocalClientConnection())->SetPlayer(pLocalPlayer);
		pLocalPlayer->pConnection->SetJoinStatus(IN_GAME);

		for (int nBotNumber = 1; nBotNumber <= 0; ++nBotNumber)
		{//Bot test
			CPlayer * pTestPlayer = new CPlayer();
			std::string sName = "Test Mimic " + itos(nBotNumber);
			pTestPlayer->SetName(sName);
			pTestPlayer->m_pController = new AiController(*pTestPlayer);
			pTestPlayer->m_pStateAuther = new LocalStateAuther(*pTestPlayer);
			//(new LocalClientConnection())->SetPlayer(pTestPlayer);
			//pTestPlayer->pConnection->SetJoinStatus(IN_GAME);
			pLocalPlayer->pConnection->AddPlayer(pTestPlayer);

glfwUnlockMutex(oPlayerTick);
			// Send a Join Team Request packet
			CPacket oJoinTeamRequest(CPacket::BOTH);
			oJoinTeamRequest.pack("hccc", 0, (u_char)27, (u_char)nBotNumber /*nth player on this connection*/, (u_char)1);
			oJoinTeamRequest.CompleteTpcPacketSize();
			pServer->SendTcp(oJoinTeamRequest);
glfwLockMutex(oPlayerTick);
		}

		// Start the game
		printf("Entered the game.\n");
		//iGameState = 0;

		bSelectTeamDisplay = true;
		bSelectTeamReady = true;
glfwUnlockMutex(oPlayerTick);
	} else if (nRunModeDEBUG == 3)		// Off-line client/server (no network)
	{
		// Connect to local server
		NetworkConnect(NULL, 0);
		pServer->SetJoinStatus(IN_GAME);

glfwLockMutex(oPlayerTick);
		pLocalPlayer = new CPlayer(0);
		pLocalPlayer->SetName(sLocalPlayerName);
		pLocalPlayer->m_pController = new HidController(*pLocalPlayer);
		pLocalPlayer->m_pStateAuther = new LocalStateAuther(*pLocalPlayer);
		(new LocalClientConnection())->SetPlayer(pLocalPlayer);
		pLocalPlayer->pConnection->SetJoinStatus(IN_GAME);

		// Start the game
		printf("Entered the game.\n");
		//iGameState = 0;

		bSelectTeamDisplay = true;
		bSelectTeamReady = true;
glfwUnlockMutex(oPlayerTick);
	}

	FpsCounter * pFpsCounter = FpsCounter::CreateCounter("Main");

	//GetReady();
	{
		// hide the mouse cursor and put in in center
		/*glfwGetMousePos(&nDesktopCursorX, &nDesktopCursorY);
		glfwDisable(GLFW_MOUSE_CURSOR);
		glfwSetMousePos(320, 240);*/

		// make sure that the physics won't count all the time passed during init
		//dBaseTime = glfwGetTime();
		if (nullptr == pGameServer)
			g_pGameSession->LogicTimer().Start();
		g_pGameSession->MainTimer().Start();
	}

	// Main loop
	while (bProgramRunning && (!bWindowModeDEBUG || glfwGetWindowParam(GLFW_OPENED)))
	{
		g_pGameSession->MainTimer().UpdateTime();

		pFpsCounter->IncrementCounter();
		FpsCounter::UpdateCounters(g_pGameSession->MainTimer().GetTime());
		if (!bWindowModeDEBUG) FpsCounter::PrintCounters();
//if (glfwGetKey('O')) {CTimedEventScheduler * p = pTimedEventScheduler; pTimedEventScheduler = NULL; delete p;}

// DEBUG
//if (nRunModeDEBUG == 0 && g_pGameSession->LogicTimer().GetGameTime() >= 14.990) InputProcessKey('1', GLFW_PRESS);

		if (bWindowModeDEBUG)
		{
			// clear the buffer
			OglUtilsSwitchMatrix(WORLD_SPACE_MATRIX);
			glLoadIdentity();
			glClear(GL_COLOR_BUFFER_BIT);

			if (iGameState == 0)
			// in game
			{
#ifdef EX0_CLIENT
				if (pLocalPlayer != NULL) {
					// mouse moved?
					//InputMouseMovCalcs();

					// key or mouse button held down?
					//InputKeyHold();
					//InputMouseHold();
				}
#endif // EX0_CLIENT

glfwLockMutex(oPlayerTick);
				for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
					if (NULL != PlayerGet(nPlayer)) {
						PlayerGet(nPlayer)->SeekRealtimeInput(g_pGameSession->MainTimer().GetTimePassed());
					}
				}
glfwUnlockMutex(oPlayerTick);

glfwLockMutex(oPlayerTick);
				// DEBUG: But this should be done by a separate camera type of thing
				for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer) {
					if (PlayerGet(nPlayer) != NULL)
						PlayerGet(nPlayer)->UpdateRenderState();
				}
glfwUnlockMutex(oPlayerTick);

				// render the static scene
				RenderStaticScene();

				// render the interactive scene
				//RenderInteractiveScene();

				// render the fov zone
				if (bStencilOperations) RenderFOV();

				// Enable the FOV masking
				if (pLocalPlayer != NULL && pLocalPlayer->GetTeam() != 2) OglUtilsSetMaskingMode(WITH_MASKING_MODE);

				// render all players
glfwLockMutex(oPlayerTick);
				//pLocalPlayer->RenderInPast(2 * pLocalPlayer->GetZ());
				//pLocalPlayer->RenderInPast(Math::TWO_PI - 2 * pLocalPlayer->GetZ());
				//for (int i = 1; i <= 100; i += 1) PlayerGet(0/*iLocalPlayerID*/)->RenderInPast(i * 0.1f);
				//pLocalPlayer->RenderInPast(kfInterpolate);
				RenderPlayers();
glfwUnlockMutex(oPlayerTick);

				// render all particles
				// TODO: Need to use a different mutex here
glfwLockMutex(oPlayerTick);
				RenderParticles();
glfwUnlockMutex(oPlayerTick);

				// Disable the masking
				if (pLocalPlayer != NULL && pLocalPlayer->GetTeam() != 2) OglUtilsSetMaskingMode(NO_MASKING_MODE);

				// render HUD
glfwLockMutex(oPlayerTick);
				RenderHUD();
glfwUnlockMutex(oPlayerTick);
			}
			else
			// in menus
			{
				//InputMouseMovCalcs();

				//MainMenuRender();
			}

			// Swap Buffers (and Poll Events)
			if (!bPaused) glfwSwapBuffers();
			else { glfwSleep(0.010); glfwPollEvents(); }

			if (glfwGetWindowParam(GLFW_ACTIVE))
			{
				g_pInputManager->ProcessJoysticks();

				g_pInputManager->TimePassed(g_pGameSession->MainTimer().GetTimePassed());		// DEBUG: Not the right place

				glfwSleep(0.0);
			}
			else
			{
				bSelectTeamDisplay = true;
				g_pInputManager->ShowMouseCursor();		// DEBUG: Not the right place

				glfwSleep(0.015);
			}
		}
		else
		{
			glfwSleep(0.0001);
		}
	}

	FpsCounter::RemoveCounter(pFpsCounter);

	// Clean up and exit nicely
	Deinit();

	printf("Returning %d from main().                       %s\n", nGlobalExitCode, nGlobalExitCode == 0 ? ":) :) :) :) :) :)))" : ">___________________<");
	return 0;
}
