// initialization
void Init();

// resize the window callback function
void GLFWCALL ResizeWindow(int iWidth, int iHeight);

// set glfw callback functions
void SetGlfwCallbacks();

// quits
void Terminate(int iExitCode);

// syncronizes random seed with all clients
void SyncRandSeed();

// Restarts the game
void RestartGame();