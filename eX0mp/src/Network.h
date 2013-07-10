class CPacket;

#define DEFAULT_PORT			9034

// Packets
#define MAX_TCP_PACKET_SIZE		1448
#define MAX_UDP_PACKET_SIZE		1448
#define MAX_PACKET_SIZE			__max(MAX_TCP_PACKET_SIZE, MAX_UDP_PACKET_SIZE)

typedef struct {
	u_char	cSequenceNumber;
	char	cMoveDirection;
	float	fZ;
	//char	cStealth;
} Input_t;

// Initialize the networking component
bool NetworkInit();

// Prints the last error code
void NetworkPrintError(const char *szMessage);

int sendall(SOCKET s, char *buf, int len, int flags);

// Connect to a server
bool NetworkConnect(char *szHost, int nPort);

bool NetworkCreateThread();

void GLFWCALL NetworkThread(void *pArg);

// Process a received TCP packet
bool NetworkProcessTcpPacket(CPacket & oPacket/*, CClient * pClient*/);

// Process a received UDP packet
bool NetworkProcessUdpPacket(CPacket & oPacket, int nPacketSize/*, CClient * pClient*/);

void NetworkDestroyThread();

// Shutdown the networking component
void NetworkDeinit();

// Closes a socket
void NetworkCloseSocket(SOCKET nSocket);
