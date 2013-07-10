void eX0_assert(bool expression, string message);

// initialization
bool Init(int argc, char *argv[]);

// deinitialization
void Deinit(void);

// resize the window callback function
//void GLFWCALL ResizeWindow(int iWidth, int iHeight);

// set glfw callback functions
//void SetGlfwCallbacks(void);

// quits
void Terminate(int nExitCode);

// syncronizes random seed with all clients
void SyncRandSeed(void);

// Restarts the game
//void RestartGame(void);
