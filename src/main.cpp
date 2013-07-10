#include "globals.h"

// no console window
//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#pragma comment(linker, "/NODEFAULTLIB:\"LIBCMT\"")
//#pragma comment(linker, "/NODEFAULTLIB:\"LIBC\"")

int				iGameState = 0;
bool			bPaused = false;

bool			bWireframe = false;
bool			bUseDefaultTriangulation = true;

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

// initialization
void Init(int argc, char *argv[])
{
	// init glfw
	glfwInit();

	// let the use choose whether to run in fullscreen mode
	bFullscreen = false;
	if (argc >= 2 && strcmp(argv[1], "--fullscreen") == 0) bFullscreen = true;
#ifdef WIN32
	bFullscreen = MessageBox(NULL, "would you like to run in fullscreen mode?", "eX0", MB_YESNO | MB_ICONQUESTION) == IDYES;
#endif

	// create the window
	glfwGetDesktopMode(&oDesktopMode);
	//if (!glfwOpenWindow(640, 480, 5, 6, 5, 0, 24, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW))
	if (!glfwOpenWindow(640, 480, 8, 8, 8, 0, 24, 8, bFullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW))
		Terminate(1);
	glfwSetWindowPos(oDesktopMode.Width / 2 - 320, oDesktopMode.Height / 2 - 240);
	glfwSetWindowTitle(((string)"eX0 (Built on " + __DATE__" at " + __TIME__ + ")").c_str());	// set the window title

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
	SyncRandSeed();
	SetGlfwCallbacks();
	GameDataLoad();			// load game data
	WeaponInitSpecs();
	PlayerInit();
	// ...

	// init OpenGL
	if (!OglUtilsInitGL())
	{
		//ERROR_HANDLER.SetLastError("OpenGL initilization failed.");
		printf("OpenGL initilization failed.\n");
		Terminate(1);
	}

	// set global variables
	// ...

	// make sure that the physics won't count all the time passed during init
	fBaseTime = static_cast<float>(glfwGetTime());
}

// deinitialization
void Deinit()
{
	// show the mouse cursor
	glfwEnable(GLFW_MOUSE_CURSOR);

	// sub-deinit
	GameDataUnload();		// unload game data
	OglUtilsDeinitGL();		// Deinit OpenGL stuff
	iNumPlayers = 0; PlayerInit();	// delete all players
	// ...

	// close the window
	if (glfwGetWindowParam(GLFW_OPENED))
		glfwCloseWindow();

	// terminate glfw
	glfwTerminate();
}

// resize the window callback function
void ResizeWindow(int iWidth, int iHeight)
{
	if (iWidth != 640 || iHeight != 480)
		Terminate(1);		// Refuse to run in non-native resolution, for now
}

// set glfw callback functions
void SetGlfwCallbacks()
{
	glfwSetWindowSizeCallback(ResizeWindow);
	glfwSetKeyCallback(InputProcessKey);
	glfwSetMouseButtonCallback(InputProcessMouse);
}

// quits
void Terminate(int iExitCode)
{
	// deinit
	Deinit();

	// DEBUG: Print out the memory usage stats
	m_dumpMemoryReport();

	exit(iExitCode);
}

// syncronizes random seed with all clients
void SyncRandSeed()
{
	//srand(2);
	srand(static_cast<unsigned int>(time(NULL)));
}

// Restarts the game
void RestartGame()
{
	// Reset the players
	PlayerInit();

	// Reset the particle engine
	oParticleEngine.Reset();

	// DEBUG - set temp ai and position players
	//oPlayers[0]->Position(125, 50);
	//oPlayers[0]->Position(20, 262);
	//oPlayers[0]->SetZ(0.8f);
	for (int iLoop1 = 0; iLoop1 < iNumPlayers; ++iLoop1) {
		float x, y;
		do {
			x = static_cast<float>(rand() % 2000 - 1000);
			y = static_cast<float>(rand() % 2000 - 1000);
		} while (ColHandIsPointInside((int)x, (int)y) || !ColHandCheckPlayerPos(&x, &y));
		oPlayers[iLoop1]->Position(x, y);
		oPlayers[iLoop1]->SetZ(0.001f * (rand() % 1000) * Math::TWO_PI);
	}
	if (iNumPlayers >= 2) {
		//oPlayers[1]->Position(0, 20);
		//oPlayers[1]->Position(145, -240);
		//oPlayers[1]->SetZ(3.0f);
		oPlayers[1]->SetTeam(1);
		//oPlayers[1]->SetStealth(true);
		//oPlayers[1]->GiveHealth(-100);		// damn ppl can't even kill the stupid bot
		//oPlayers[0]->GiveHealth(+5000);		// semi-god mode?
	}

	printf("Game restarted. ============================\n");
}

// main function
int main(int argc, char *argv[]) 
{
	// DEBUG vars
	static float fTempTimer = 0;
	static int iWhere;

	// initialize
	Init(argc, argv);

	// DEBUG - This is a hack - we only need to position the players here... But I called RestartGame() to do that
	RestartGame();

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
				fBaseTime = glfwGetTime();
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

			// DEBUG - test bot AI
			if (iNumPlayers >= 2) {
				#define BOT_AI_SENSOR_LENGTH 13.0f
				// update bot's rotated vels
				static float fTempTimer2 = -1.0f;
				static float fTempTimer3 = -1050.0f;
				static float fTempTimer4 = 0.0f;
				if (fTempTimer2 < 0.0f) fTempTimer2 += fTimePassed;
				if (fTempTimer3 > 0.0f) fTempTimer3 -= fTimePassed;
				if (fTempTimer4 > -1.0f) fTempTimer4 -= fTimePassed;
				if (fTempTimer2 >= 0.0f && !oPlayers[0]->IsDead())
				{
					oPlayers[1]->SetZ(MathCoordToRad(oPlayers[0]->GetIntX() + oPlayers[0]->GetVelX() * 1.5, oPlayers[0]->GetIntY() + oPlayers[0]->GetVelY() * 1.5, oPlayers[1]->GetIntX(), oPlayers[1]->GetIntY()) - Math::HALF_PI + ((rand() % 1000)/1000.0f * 0.25f - 0.125));
					if (!ColHandSegmentIntersects(oPlayers[1]->GetIntX(), oPlayers[1]->GetIntY(), oPlayers[0]->GetIntX(), oPlayers[0]->GetIntY())) { if (fTempTimer3 < -1000.0f) fTempTimer3 = 0.5f; } else fTempTimer3 = -1050.0f;
					if (fTempTimer3 > -1000.0f && fTempTimer3 <= 0.0f) oPlayers[1]->Fire();
					if (oPlayers[1]->GetSelClips() <= 1) oPlayers[1]->BuyClip();
					if (oPlayers[1]->GetSelClipAmmo() <= 0) oPlayers[1]->Reload();
					fTempTimer += fTimePassed;
					if (fTempTimer >= 0.0)
					{
						fTempTimer -= 1.25;
						if (!ColHandIsPointInside(oPlayers[1]->GetIntX() + Math::Sin(oPlayers[1]->GetZ()) * BOT_AI_SENSOR_LENGTH, oPlayers[1]->GetIntY() + Math::Cos(oPlayers[1]->GetZ()) * BOT_AI_SENSOR_LENGTH) && !oPlayers[1]->IsReloading())
						{
							iWhere = 0;
						}
						else if (!ColHandIsPointInside(oPlayers[1]->GetIntX() + Math::Sin(oPlayers[1]->GetZ() + Math::HALF_PI * 0.5) * BOT_AI_SENSOR_LENGTH, oPlayers[1]->GetIntY() + Math::Cos(oPlayers[1]->GetZ() + Math::HALF_PI * 0.5) * BOT_AI_SENSOR_LENGTH) && !oPlayers[1]->IsReloading())
						{
							iWhere = 1;
						}
						else if (!ColHandIsPointInside(oPlayers[1]->GetIntX() + Math::Sin(oPlayers[1]->GetZ() + Math::HALF_PI * -0.5) * BOT_AI_SENSOR_LENGTH, oPlayers[1]->GetIntY() + Math::Cos(oPlayers[1]->GetZ() + Math::HALF_PI * -0.5) * BOT_AI_SENSOR_LENGTH) && !oPlayers[1]->IsReloading())
						{
							iWhere = 7;
						}
						else if (!ColHandIsPointInside(oPlayers[1]->GetIntX() + Math::Sin(oPlayers[1]->GetZ() + Math::HALF_PI * 1.0) * BOT_AI_SENSOR_LENGTH, oPlayers[1]->GetIntY() + Math::Cos(oPlayers[1]->GetZ() + Math::HALF_PI * 1.0) * BOT_AI_SENSOR_LENGTH) && !oPlayers[1]->IsReloading())
						{
							iWhere = 2;
							fTempTimer -= 3.0f;
						}
						else if (!ColHandIsPointInside(oPlayers[1]->GetIntX() + Math::Sin(oPlayers[1]->GetZ() + Math::HALF_PI * -1.0) * BOT_AI_SENSOR_LENGTH, oPlayers[1]->GetIntY() + Math::Cos(oPlayers[1]->GetZ() + Math::HALF_PI * -1.0) * BOT_AI_SENSOR_LENGTH) && !oPlayers[1]->IsReloading())
						{
							iWhere = 6;
							fTempTimer -= 3.0f;
						}
						else if (!ColHandIsPointInside(oPlayers[1]->GetIntX() + Math::Sin(oPlayers[1]->GetZ() + Math::HALF_PI * 1.5) * BOT_AI_SENSOR_LENGTH, oPlayers[1]->GetIntY() + Math::Cos(oPlayers[1]->GetZ() + Math::HALF_PI * 1.5) * BOT_AI_SENSOR_LENGTH))
						{
							iWhere = 3;
							fTempTimer -= 10.0f;
						}
						else if (!ColHandIsPointInside(oPlayers[1]->GetIntX() + Math::Sin(oPlayers[1]->GetZ() + Math::HALF_PI * -1.5) * BOT_AI_SENSOR_LENGTH, oPlayers[1]->GetIntY() + Math::Cos(oPlayers[1]->GetZ() + Math::HALF_PI * -1.5) * BOT_AI_SENSOR_LENGTH))
						{
							iWhere = 5;
							fTempTimer -= 10.0f;
						}
						else
						{
							iWhere = 4;
							fTempTimer -= 5.0f;
						}
					}
					if (Math::Sqrt(Math::Pow(oPlayers[1]->GetIntX() + oPlayers[0]->GetIntX(), 2) + Math::Pow(oPlayers[1]->GetIntY() + oPlayers[0]->GetIntY(), 2)) < PLAYER_WIDTH * 2.0 && !oPlayers[1]->IsReloading())
						//oPlayers[1]->Move(-1);
						fTempTimer4 = 1.0f;
					static int i2 = 1;
					if (fTempTimer4 > 0.0f) {
						int iWhere2 = (iWhere + 4) % 8;
						int i = ((int)Math::Ceil(fTempTimer4 * 4) % 2) * 2 - 1;
						iWhere2 = (iWhere + 4 + i + i2) % 8;
						oPlayers[1]->Move(iWhere2);
					} else {
						oPlayers[1]->Move(iWhere);
						if (++i2 > 1) i2 = -1;
					}
					if (fTempTimer3 > -1000.0f && fTempTimer3 <= 0.0f) oPlayers[1]->Move(-1);
				}
				else if (fTempTimer2 >= 0)
				{
					oPlayers[1]->Move(0);
					if (ColHandIsPointInside(oPlayers[1]->GetIntX() + Math::Sin(oPlayers[1]->GetZ()) * BOT_AI_SENSOR_LENGTH, oPlayers[1]->GetIntY() + Math::Cos(oPlayers[1]->GetZ()) * BOT_AI_SENSOR_LENGTH * 2))
						oPlayers[1]->Move(-1);
				} else { if (glfwGetKey('Q')) oPlayers[1]->Move(0); else oPlayers[1]->Move(-1); }
			}

			// DEBUG: Make other bots shoot
				for (int iLoop1 = 2; iLoop1 < iNumPlayers; iLoop1++) {
					oPlayers[iLoop1]->SetTeam(1);
					//oPlayers[iLoop1]->iSelWeapon = 3;
					oPlayers[iLoop1]->BuyClip();
					if (oPlayers[iLoop1]->GetSelClipAmmo() <= 0) oPlayers[iLoop1]->Reload();
					oPlayers[iLoop1]->Fire();
					oPlayers[iLoop1]->Rotate(0.5 * fTimePassed);
				}

			// player tick
			PlayerTick();

			// particle engine tick
			if (bPaused) { fTempFloat = fTimePassed; fTimePassed = 0; }
			oParticleEngine.Tick();
			if (bPaused) fTimePassed = fTempFloat;

			// render the static scene
			RenderStaticScene();

			// render the interactive scene
			//RenderInteractiveScene();

			// render the fov zone
			RenderFOV();

			// Enable the FOV masking
			OglUtilsSetMaskingMode(WITH_MASKING_MODE);

			// render all players
			RenderPlayers();

			// render all particles
			RenderParticles();

			// Disable the masking
			OglUtilsSetMaskingMode(NO_MASKING_MODE);

// DEBUG - testing temporary bot AI check points
if (iNumPlayers >= 2 && glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
{
oPlayers[1]->Render();
int x, y;
for (int i = 0; i < 1; ++i)
{
	//int i = 0;
	x = (int)(oPlayers[1]->GetIntX() + Math::Sin(oPlayers[1]->GetZ() + i * Math::PI * 0.25) * 13.0f);
	y = (int)(oPlayers[1]->GetIntY() + Math::Cos(oPlayers[1]->GetZ() + i * Math::PI * 0.25) * 13.0f);
	//x = (int)(oPlayers[1]->GetIntX() + Math::Sin(oPlayers[1]->GetZ() + Math::HALF_PI * 0.5) * 13.0f);
	//y = (int)(oPlayers[1]->GetIntY() + Math::Cos(oPlayers[1]->GetZ() + Math::HALF_PI * 0.5) * 13.0f);
	glBegin(GL_POINTS);
		ColHandIsPointInside(x, y) ? glColor3f(1.0, 0.0, 0.0) : glColor3f(0.0, 0.0, 1.0);
		glVertex2f(x, y);
	glEnd();
	glBegin(GL_LINES);
		glVertex2f(oPlayers[0]->GetIntX(), oPlayers[0]->GetIntY());
		glVertex2f(oPlayers[1]->GetIntX(), oPlayers[1]->GetIntY());
	glEnd();
}
}

			// render HUD
			RenderHUD();

			//if (glfwGetKey('P')) m_dumpMemoryReport();
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
	}

	Terminate(0);

	return 0;
}
