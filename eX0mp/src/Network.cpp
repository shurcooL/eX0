#include "globals.h"

SOCKET			nServerSocket = INVALID_SOCKET;

GLFWthread		oNetworkThread = -1;
volatile bool	bNetworkThreadRun;
GLFWmutex		oTcpSendMutex;

// Initialize the networking component
bool NetworkInit()
{
	oTcpSendMutex = glfwCreateMutex();

#ifdef WIN32
	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nResult != 0)
		printf("Error at WSAStartup(): %d.\n", nResult);
	else if (!(LOBYTE(wsaData.wVersion) == 2 && HIBYTE(wsaData.wVersion) == 2)) {
		printf("Error: Winsock version 2.2 is not avaliable.\n");
		WSACleanup();
		return false;
	}

	return nResult == 0;
#else // Linux
	// Nothing to be done on Linux
#endif
}

void NetworkPrintError(const char *szMessage)
{
#ifdef WIN32
	//printf("%s: %d\n", szMessage, WSAGetLastError());
	printf("%s\n", WSAGetLastErrorMessage(szMessage, WSAGetLastError()));
#else // Linux
	perror(szMessage);
#endif
}

int sendall(SOCKET s, char *buf, int len, int flags)
{
	glfwLockMutex(oTcpSendMutex);

	int total = 0;        // how many bytes we've sent
	int bytesleft = len; // how many we have left to send
	int n;

	while(total < len) {
		n = send(s, buf+total, bytesleft, flags);
		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}

	//*len = total; // return number actually sent here

	glfwUnlockMutex(oTcpSendMutex);

	return (n == -1 || total != len) ? -1 : total; // return -1 on failure, bytes sent on success
}

// Connect to a server
bool NetworkConnect(char *szHost, int nPort)
{
	struct hostent *he;
	struct sockaddr_in oServerAddress;

	if ((he = gethostbyname(szHost)) == NULL) {		// get the host info
		NetworkPrintError("gethostbyname");		// herror
		Terminate(1);
	}

	if ((nServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
	    NetworkPrintError("socket");
	    Terminate(1);
	}

	/* This is the default behaviour already (graceful shutdown, don't linger)
	struct linger oLinger;
	oLinger.l_onoff = 0;
	oLinger.l_linger = 0;
	if (setsockopt(nServerSocket, SOL_SOCKET, SO_LINGER, (char *)&oLinger, sizeof(oLinger)) == SOCKET_ERROR) {
		NetworkPrintError("setsockopt");
		Terminate(1);
	}*/
	// Disable the Nagle algorithm for send coalescing
	int nNoDelay = 1;
	if (setsockopt(nServerSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&nNoDelay, sizeof(nNoDelay)) == SOCKET_ERROR) {
		NetworkPrintError("setsockopt");
		Terminate(1);
	}

	oServerAddress.sin_family = AF_INET;
	oServerAddress.sin_addr = *((struct in_addr *)he->h_addr);
	oServerAddress.sin_port = htons(nPort);
	memset(oServerAddress.sin_zero, 0, sizeof(oServerAddress.sin_zero));

	printf("Connecting to %s.\n", szHost);

	if (connect(nServerSocket, (struct sockaddr *)&oServerAddress, sizeof(oServerAddress)) == SOCKET_ERROR) {
	    NetworkPrintError("connect");
	    Terminate(1);
	}

	printf("Established a connection, attempting to join the game.\n");

	// Create the networking thread
	NetworkCreateThread();

	// Create and send a Join Game Request packet
	char buf[22 + 1];
	*((short*)buf) = htons(22);
	*((short*)(buf+2)) = htons(1);
	*((short*)(buf+4)) = htons(1);
	char *szPassphrase = "somerandompass01";
	memcpy(buf+6, szPassphrase, 16);
	if (sendall(nServerSocket, buf, 22, 0) != 22) {
		NetworkPrintError("sendall");
		Terminate(1);
	}

	return true;
}

bool NetworkCreateThread()
{
	bNetworkThreadRun = true;
	oNetworkThread = glfwCreateThread(NetworkThread, NULL);

	printf("Network thread created.\n");

	return oNetworkThread >= 0;
}

void GLFWCALL NetworkThread(void *pArg)
{
	int			nbytes;
	char		buf[2 * MAX_TCP_PACKET_SIZE];
	short		snCurrentPacketSize = 0;

	while (bNetworkThreadRun)
	{
		// handle data from the server
		if ((nbytes = recv(nServerSocket, buf + snCurrentPacketSize, sizeof(buf) - snCurrentPacketSize, 0)) <= 0) {
			// got error or connection closed by client
			if (nbytes == 0) {
				// Nothing to do
			} else {
				NetworkPrintError("recv");
			}
			if (!bNetworkThreadRun) {
				// connection closed
				printf("Connection to the server closed gracefully.\n");
			} else {
				printf("Lost connection to the server.\n");
				bNetworkThreadRun = false;
			}
		} else {
			// we got some data from a client, process it
			//printf("Got %d bytes from server\n", nbytes);

			snCurrentPacketSize += nbytes;
			short snRealPacketSize = MAX_TCP_PACKET_SIZE;
			if (snCurrentPacketSize >= 2)
				snRealPacketSize = ntohs(((struct TcpPacket_t *)buf)->snPacketSize);
			// Check if received a full maximum-length packet
			// or enough to check the packet size
			while (snCurrentPacketSize >= snRealPacketSize
				&& snRealPacketSize <= MAX_TCP_PACKET_SIZE)		// Make sure the packet is not larger than allowed
			{
				// Received the entire packet, process it
				if (!NetworkProcessPacket((struct TcpPacket_t *)buf, nServerSocket))
					printf("Couldn't process a packet.\n");
				memmove(buf, buf + snRealPacketSize, sizeof(buf) - snRealPacketSize);
				snCurrentPacketSize -= snRealPacketSize;

				if (snCurrentPacketSize >= 2)
					snRealPacketSize = ntohs(((struct TcpPacket_t *)buf)->snPacketSize);
				else snRealPacketSize = MAX_TCP_PACKET_SIZE;
			}
		}

		// Sleep
		glfwSleep(0.0);
	}
}

void NetworkDestroyThread()
{
	if (oNetworkThread < 0) return;

	bNetworkThreadRun = false;

	shutdown(nServerSocket, SD_BOTH);

	glfwWaitThread(oNetworkThread, GLFW_WAIT);
	//glfwDestroyThread(oNetworkThread);

	printf("Network thread destroyed.\n");
}

// Process a received packet
bool NetworkProcessPacket(struct TcpPacket_t * oPacket, SOCKET nSocket)
{
	short snPacketSize = ntohs(oPacket->snPacketSize);
	short snPacketType = ntohs(oPacket->snPacketType);

if (glfwGetKey(GLFW_KEY_TAB) == GLFW_RELEASE)
fTempFloat = static_cast<float>(glfwGetTime());

	switch (snPacketType) {
	// Join Game Accept
	case 2:
		if (snPacketSize != sizeof(struct TcpPacketJoinGameAccept_t)) return false;		// Check packet size

		// Valid Join Game Accept packet
		printf("Got accepted TcpPacketJoinGameAccept_t\n");

		iLocalPlayerID = (int)((struct TcpPacketJoinGameAccept_t *)oPacket)->chPlayerID;
		nPlayerCount = (int)((struct TcpPacketJoinGameAccept_t *)oPacket)->chMaxPlayerCount;

		printf("Got accepted in game: local player id = %d, player count = %d\n", iLocalPlayerID, nPlayerCount);

		RestartGame();
		iGameState = 0;

		break;
	// Broadcast Text Message
	case 11:
		if (snPacketSize <= 5) return false;		// Check packet size

		if (true)
		{
			// Valid Broadcast Text Message packet
			printf("Got valid TcpPacketBroadcastTextMessage_t, message follows.\n");

			string sTextMessage(((struct TcpPacketBroadcastTextMessage_t *)oPacket)->chTextMessage,
				snPacketSize - 5);

			// Print out the text message
			printf("Player %d sends: '%s'\n", (int)((struct TcpPacketBroadcastTextMessage_t *)oPacket)->chPlayerID,
				sTextMessage.c_str());
		}

		break;
	// Update Others Position (temporary debug packet)
	case 21:
		if (snPacketSize != sizeof(struct TcpPacketUpdateOthersPosition_t)) return false;		// Check packet size

		if (true)
		{
			int nPlayerID = (int)((struct TcpPacketUpdateOthersPosition_t *)oPacket)->chPlayerID;
			float fX = ((struct TcpPacketUpdateOthersPosition_t *)oPacket)->fX;
			float fY = ((struct TcpPacketUpdateOthersPosition_t *)oPacket)->fY;
			float fZ = ((struct TcpPacketUpdateOthersPosition_t *)oPacket)->fZ;
			bool bFire = (bool)((struct TcpPacketUpdateOthersPosition_t *)oPacket)->chFire;

			//printf("Got update others position for player %d.\n", nPlayerID);

			PlayerGet(nPlayerID)->Position(fX, fY);
			PlayerGet(nPlayerID)->SetZ(fZ);
			if (bFire) PlayerGet(nPlayerID)->Fire();
		}

		break;

	default:
		return false;
		break;
	}

if (glfwGetKey(GLFW_KEY_TAB) == GLFW_RELEASE)
fTempFloat = static_cast<float>(glfwGetTime()) - fTempFloat;

	return true;
}

// Closes a socket
void NetworkCloseSocket(SOCKET nSocket)
{
#ifdef WIN32
	closesocket(nSocket);
#else // Linux
	close(nSocket);
#endif

	nSocket = INVALID_SOCKET;
}

// Shutdown the networking component
void NetworkDeinit()
{
	NetworkDestroyThread();

	NetworkCloseSocket(nServerSocket);

#ifdef WIN32
	WSACleanup();
#else // Linux
	// Nothing to be done on Linux
#endif

	glfwDestroyMutex(oTcpSendMutex);
}
