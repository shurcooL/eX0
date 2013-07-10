// Initialize the server
bool ServerInit(void);

// Start the server
bool ServerStart(void);

bool ServerCreateThread(void);

void GLFWCALL ServerThread(void *pArg);

void ServerDestroyThread(void);

// Shutdown the server
void ServerDeinit(void);
