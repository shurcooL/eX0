#define DEFAULT_PORT		9034

// Packets
#define MAX_TCP_PACKET_SIZE	1448

struct TcpPacket_t
{
	short	snPacketSize;
	short	snPacketType;
	char	chData[MAX_TCP_PACKET_SIZE - 2 * sizeof(short)];
};

struct TcpPacketJoinGameRequest_t
{
	short	snPacketSize;
	short	snPacketType;	// 1
	short	snVersion;
	char	chPassphrase[16];
};
struct TcpPacketJoinGameAccept_t
{
	short	snPacketSize;
	short	snPacketType;	// 2
	char	chPlayerID;
	char	chMaxPlayerCount;
};
struct TcpPacketSendTextMessage_t
{
	short	snPacketSize;
	short	snPacketType;	// 10
	char	chTextMessage[MAX_TCP_PACKET_SIZE - 4];
};
struct TcpPacketBroadcastTextMessage_t
{
	short	snPacketSize;
	short	snPacketType;	// 11
	char	chPlayerID;
	char	chTextMessage[MAX_TCP_PACKET_SIZE - 5];
};
struct TcpPacketUpdateOwnPosition_t
{
	short	snPacketSize;
	short	snPacketType;	// 20
	float	fX;
	float	fY;
	float	fZ;
	char	chFire;
};
struct TcpPacketUpdateOthersPosition_t
{
	short	snPacketSize;
	short	snPacketType;	// 21
	char	chPlayerID;
	float	fX;
	float	fY;
	float	fZ;
	char	chFire;
};

struct WeaponInfo_t
{
	int		iClips;
	int		iClipAmmo;
	bool	bReloading;
	int		iTimer;
};

// Initialize the networking component
bool NetworkInit();

// Prints the last error code
void NetworkPrintError(const char *szMessage);

int sendall(SOCKET s, char *buf, int len, int flags);

// Connect to a server
bool NetworkConnect(char *szHost, int nPort);

bool NetworkCreateThread();

void GLFWCALL NetworkThread(void *pArg);

void NetworkDestroyThread();

// Process a received packet
bool NetworkProcessPacket(struct TcpPacket_t * oPacket, SOCKET nSocket);

// Closes a socket
void NetworkCloseSocket(SOCKET nSocket);

// Shutdown the networking component
void NetworkDeinit();
