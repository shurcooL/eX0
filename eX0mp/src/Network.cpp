#include "globals.h"

SOCKET			nServerTcpSocket = INVALID_SOCKET;
SOCKET			nServerUdpSocket = INVALID_SOCKET;
struct sockaddr_in	oLocalUdpAddress;		// Get the local UDP port
socklen_t			nLocalUdpAddressLength = sizeof(oLocalUdpAddress);
struct sockaddr_in	oServerAddress;
volatile int	nJoinStatus = DISCONNECTED;
u_char			cSignature[SIGNATURE_SIZE];
u_int			nSendUdpHandshakePacketEventId = 0;

GLFWthread		oNetworkThread = -1;
volatile bool	bNetworkThreadRun;
GLFWmutex		oTcpSendMutex;
GLFWmutex		oUdpSendMutex;

u_char			cCurrentCommandSequenceNumber = 0;
//u_char			cLastAckedCommandSequenceNumber = 0;
u_char			cLastUpdateSequenceNumber = 0;
GLFWmutex		oPlayerTick;

IndexedCircularBuffer<Move_t, u_char>	oUnconfirmedMoves;

MovingAverage	oRecentLatency(60.0, 10);
MovingAverage	oRecentTimeDifference(60.0, 10);
HashMatcher<PingData_t, double>	oPongSentTimes(PING_SENT_TIMES_HISTORY);
vector<double>	oSentTimeRequestPacketTimes(256);
u_char			cNextTimeRequestSequenceNumber = 0;
double			dShortestLatency = 1000;
double			dShortestLatencyLocalTime;
double			dShortestLatencyRemoteTime;
u_int			nTrpReceived = 0;
u_int			nSendTimeRequestPacketEventId = 0;

u_char			cCommandRate = 20;
u_char			cUpdateRate = 20;

const float		kfInterpolate = 0.1f;
const float		kfMaxExtrapolate = 0.5f;

bool			bGotPermissionToEnter = false;
bool			bFinishedSyncingClock = false;
GLFWmutex		oJoinGameMutex;

// Initialize the networking component
bool NetworkInit()
{
	oTcpSendMutex = glfwCreateMutex();
	oUdpSendMutex = glfwCreateMutex();
	oPlayerTick = glfwCreateMutex();
	oJoinGameMutex = glfwCreateMutex();

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
	return true;
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

int sendall(SOCKET s, const char *buf, int len, int flags)
{
	glfwLockMutex(oTcpSendMutex);

	int total = 0;        // how many bytes we've sent
	int bytesleft = len; // how many we have left to send
	int n = 0;

	while (total < len) {
		n = send(s, buf+total, bytesleft, flags);
		if (n == SOCKET_ERROR) { break; }
		total += n;
		bytesleft -= n;
	}

	//*len = total; // return number actually sent here

	glfwUnlockMutex(oTcpSendMutex);

	return (n == SOCKET_ERROR || total != len) ? SOCKET_ERROR : total; // return SOCKET_ERROR on failure, bytes sent on success
}

int sendudp(SOCKET s, const char *buf, int len, int flags, const sockaddr *to, int tolen)
{
	glfwLockMutex(oUdpSendMutex);

	int nResult;
	if (to != NULL) nResult = sendto(s, buf, len, flags, to, tolen);
	else nResult = send(s, buf, len, flags);

	glfwUnlockMutex(oUdpSendMutex);

	return nResult;
}

void SendTimeRequestPacket(void *p)
{
	static u_int nTrpSent = 0;

	// Send a time request packet
	CPacket oTimeRequestPacket;
	oTimeRequestPacket.pack("cc", (u_char)105, cNextTimeRequestSequenceNumber);
	oSentTimeRequestPacketTimes.at(cNextTimeRequestSequenceNumber) = glfwGetTime();
	++cNextTimeRequestSequenceNumber;
	oTimeRequestPacket.SendUdp(UDP_CONNECTED);

	//printf("Sent TRqP #%d at %.5lf ms\n", ++nTrpSent, glfwGetTime() * 1000);
}

// Connect to a server
bool NetworkConnect(char *szHost, int nPort)
{
	struct hostent *he;

	if ((he = gethostbyname(szHost)) == NULL) {		// get the host info
		NetworkPrintError("gethostbyname");		// herror
		Terminate(1);
	}

	if ((nServerTcpSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
	    NetworkPrintError("socket");
	    Terminate(1);
	}
	if ((nServerUdpSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
	    NetworkPrintError("socket");
	    Terminate(1);
	}

	/* This is the default behaviour already (graceful shutdown, don't linger)
	struct linger oLinger;
	oLinger.l_onoff = 0;
	oLinger.l_linger = 0;
	if (setsockopt(nServerTcpSocket, SOL_SOCKET, SO_LINGER, (char *)&oLinger, sizeof(oLinger)) == SOCKET_ERROR) {
		NetworkPrintError("setsockopt");
		Terminate(1);
	}*/
	// Disable the Nagle algorithm for send coalescing
	int nNoDelay = 1;
	if (setsockopt(nServerTcpSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&nNoDelay, sizeof(nNoDelay)) == SOCKET_ERROR) {
		NetworkPrintError("setsockopt");
		Terminate(1);
	}

	oServerAddress.sin_family = AF_INET;
	oServerAddress.sin_addr = *((struct in_addr *)he->h_addr);
	oServerAddress.sin_port = htons(nPort);
	memset(oServerAddress.sin_zero, 0, sizeof(oServerAddress.sin_zero));

	printf("Connecting to %s.\n", szHost);
	if (connect(nServerTcpSocket, (struct sockaddr *)&oServerAddress, sizeof(oServerAddress)) == SOCKET_ERROR) {
	    NetworkPrintError("connect");
	    Terminate(1);
	}
	struct sockaddr_in	oLocalTcpAddress;
	socklen_t			nLocalTcpAddressLength = sizeof(oLocalTcpAddress);
	if (getsockname(nServerTcpSocket, (struct sockaddr *)&oLocalTcpAddress, &nLocalTcpAddressLength) == SOCKET_ERROR) {
		NetworkPrintError("getsockname");
		Terminate(1);
	}
	// Successfully connected (TCP) to the server
	nJoinStatus = TCP_CONNECTED;
	printf("Established a TCP connection (local port %d), attempting to join the game.\n", ntohs(oLocalTcpAddress.sin_port));

	// Create the networking thread
	NetworkCreateThread();

	// Create and send a Join Server Request packet
	CPacket oJoinServerRequestPacket;
	oJoinServerRequestPacket.pack("hchs", 0, (u_char)1, 1, "somerandompass01");
	double dSignature = glfwGetTime();
	memcpy(cSignature, (void *)&dSignature, SIGNATURE_SIZE);
	for (int nSignatureByte = 0; nSignatureByte < SIGNATURE_SIZE; ++nSignatureByte)
		oJoinServerRequestPacket.pack("c", cSignature[nSignatureByte]);
	oJoinServerRequestPacket.CompleteTpcPacketSize();
	oJoinServerRequestPacket.SendTcp(TCP_CONNECTED);

	return true;
}

bool NetworkCreateThread()
{
	bNetworkThreadRun = true;
	oNetworkThread = glfwCreateThread(&NetworkThread, NULL);

	printf("Network thread created.\n");

	return (oNetworkThread >= 0);
}

void GLFWCALL NetworkThread(void *pArgument)
{
	int fdmax;
	fd_set master;   // master file descriptor list
	fd_set read_fds;

	int			nbytes;
	u_char		buf[2 * MAX_TCP_PACKET_SIZE - 1];
	u_char		cUdpBuffer[MAX_UDP_PACKET_SIZE];
	u_short		snCurrentPacketSize = 0;

	// clear the master and temp sets
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	// Add the sockets to select on
	FD_SET(nServerTcpSocket, &master);
	FD_SET(nServerUdpSocket, &master);

	fdmax = std::max<int>((int)nServerTcpSocket, (int)nServerUdpSocket);

	while (bNetworkThreadRun)
	{
		read_fds = master; // copy it
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == SOCKET_ERROR) {
			NetworkPrintError("select");
			Terminate(1);
		}

		if (!bNetworkThreadRun) {
			break;
		}
		// have TCP data
		else if (FD_ISSET(nServerTcpSocket, &read_fds))
		{
			// got error or connection closed by server
			if ((nbytes = recv(nServerTcpSocket, reinterpret_cast<char *>(buf) + snCurrentPacketSize, sizeof(buf) - snCurrentPacketSize, 0)) <= 0) {
				if (nbytes == 0) {
					// Connection closed gracefully by the server
					printf("Connection closed gracefully by the server.\n");
				} else {
					NetworkPrintError("recv");
					printf("Lost connection to the server.\n");
				}
				bNetworkThreadRun = false;
				nJoinStatus = DISCONNECTED;
				iGameState = 1;
			// we got some data from a client, process it
			} else {
				//printf("Got %d bytes from server\n", nbytes);

				snCurrentPacketSize += nbytes;
				eX0_assert(snCurrentPacketSize <= sizeof(buf), "snCurrentPacketSize <= sizeof(buf)");
				// Check if received enough to check the packet size
				u_short snRealPacketSize = MAX_TCP_PACKET_SIZE;
				if (snCurrentPacketSize >= 2)
					snRealPacketSize = 3 + ntohs(*reinterpret_cast<u_short *>(buf));
				if (snRealPacketSize > MAX_TCP_PACKET_SIZE) {		// Make sure the packet is not larger than allowed
					printf("Got a TCP packet that's larger than allowed.\n");
					snRealPacketSize = MAX_TCP_PACKET_SIZE;
				}
				// Received an entire packet
				while (snCurrentPacketSize >= snRealPacketSize)
				{
					// Process it
					CPacket oPacket(buf, snRealPacketSize);
					if (!NetworkProcessTcpPacket(oPacket)) {
						printf("Couldn't process a TCP packet (type %d):\n  ", *reinterpret_cast<u_char *>(buf + 2));
						oPacket.Print();
					}

					//memmove(buf, buf + snRealPacketSize, sizeof(buf) - snRealPacketSize);
					//snCurrentPacketSize -= snRealPacketSize;
					snCurrentPacketSize -= snRealPacketSize;
					eX0_assert(snCurrentPacketSize <= sizeof(buf) - snRealPacketSize, "snCurrentPacketSize <= sizeof(buf) - snRealPacketSize");
					memmove(buf, buf + snRealPacketSize, snCurrentPacketSize);

					if (snCurrentPacketSize >= 2)
						snRealPacketSize = 3 + ntohs(*reinterpret_cast<u_short *>(buf));
					else snRealPacketSize = MAX_TCP_PACKET_SIZE;
					if (snRealPacketSize > MAX_TCP_PACKET_SIZE) {		// Make sure the packet is not larger than allowed
						printf("Got a TCP packet that's larger than allowed.\n");
						snRealPacketSize = MAX_TCP_PACKET_SIZE;
					}
				}
			}
		}

		if (!bNetworkThreadRun) {
			break;
		}
		// have UDP data
		else if (FD_ISSET(nServerUdpSocket, &read_fds))
		{
			// handle UDP data from the server
			if ((nbytes = recv(nServerUdpSocket, reinterpret_cast<char *>(cUdpBuffer), sizeof(cUdpBuffer), 0)) == SOCKET_ERROR)
			{
				// Error
				NetworkPrintError("recv");
			} else {
				// Got a UDP packet
				//printf("Got a UDP %d byte packet from server!\n", nbytes);

				// Process the received UDP packet
				CPacket oPacket(cUdpBuffer, nbytes);
				if (!NetworkProcessUdpPacket(oPacket)) {
					printf("Couldn't process a UDP packet (type %d):\n  ", cUdpBuffer[0]);
					oPacket.Print();
				}
			}
		}

		// There's no need to Sleep here, since select will essentially do that automatically whenever there's no data
	}

	// Clean up, if Deinit() hasn't done so already
	if (pTimedEventScheduler != NULL)
		pTimedEventScheduler->RemoveAllEvents();

	printf("Network thread has ended.\n");
	oNetworkThread = -1;
}

// Process a received TCP packet
bool NetworkProcessTcpPacket(CPacket & oPacket)
{
	u_short nDataSize; oPacket.unpack("h", &nDataSize);
	u_char cPacketType; oPacket.unpack("c", &cPacketType);

if (glfwGetKey(GLFW_KEY_TAB) == GLFW_RELEASE)
fTempFloat = static_cast<float>(glfwGetTime());

	switch (cPacketType) {
	// Join Server Accept
	case 2:
		// Check if we have already been accepted by the server
		if (nJoinStatus >= ACCEPTED) {
			printf("Error: Already accepted, but received another Join Server Accept packet.\n");
			return false;
		}

		if (nDataSize != 2) return false;		// Check packet size
		else {
			char cLocalPlayerID;
			char cPlayerCount;
			oPacket.unpack("cc", &cLocalPlayerID, (char *)&cPlayerCount);

			iLocalPlayerID = (int)cLocalPlayerID;
			nPlayerCount = (int)cPlayerCount;
			PlayerInit();
			// TODO: Player name (and other local settings?) needs to be assigned, validated (and corrected if needed) in a better way
			PlayerGet(iLocalPlayerID)->bConnected = true;
			PlayerGet(iLocalPlayerID)->SetName(sLocalPlayerName);
			// Got successfully accepted in game by the server
			nJoinStatus = ACCEPTED;
			printf("Got accepted in game: local player id = %d, player count = %d\n", iLocalPlayerID, nPlayerCount);

			// Create the UDP connection
			if (connect(nServerUdpSocket, (struct sockaddr *)&oServerAddress, sizeof(oServerAddress)) == SOCKET_ERROR) {
				NetworkPrintError("connect");
				Terminate(1);
			}
			if (getsockname(nServerUdpSocket, (struct sockaddr *)&oLocalUdpAddress, &nLocalUdpAddressLength) == SOCKET_ERROR) {
				NetworkPrintError("getsockname");
				Terminate(1);
			}
			printf("Created a pending UDP connection to server on port %d.\n", ntohs(oLocalUdpAddress.sin_port));

			// Send UDP packet with the same signature to initiate the UDP handshake
			// Add timed event (Retransmit UdpHandshake packet every 100 milliseconds)
			CTimedEvent oEvent(glfwGetTime(), 0.1, &NetworkSendUdpHandshakePacket, NULL);
			nSendUdpHandshakePacketEventId = oEvent.GetId();
			pTimedEventScheduler->ScheduleEvent(oEvent);
		}
		break;
	// Join Game Refuse
	case 3:
		// Check if we have already joined the game
		if (nJoinStatus >= IN_GAME) {
			printf("Error: Already in game, but received a Join Game Refuse packet.\n");
			return false;
		}

		if (nDataSize != 1) return false;		// Check packet size
		else {
			char cRefuseReason;
			oPacket.unpack("c", &cRefuseReason);

			printf("Got refused with reason %d.\n", (int)cRefuseReason);

			bNetworkThreadRun = false;
			nJoinStatus = DISCONNECTED;		// Server connection status is fully unconnected
			iGameState = 1;
		}
		break;
	// UDP Connection Established
	case 5:
		// Check if we have been accepted by the server
		if (nJoinStatus < ACCEPTED) {
			printf("Error: Not yet accepted by the server.\n");
			return false;
		}

		if (nDataSize != 0) return false;		// Check packet size
		else {
			// The UDP connection with the server is fully established
			nJoinStatus = UDP_CONNECTED;
			printf("Established a UDP connection with the server.\n");

			// Stop sending UDP handshake packets
			pTimedEventScheduler->RemoveEventById(nSendUdpHandshakePacketEventId);

			// Start syncing the clock, send a Time Request packet every 50 ms
			CTimedEvent oEvent(glfwGetTime(), 0.05, SendTimeRequestPacket, NULL);
			nSendTimeRequestPacketEventId = oEvent.GetId();
			pTimedEventScheduler->ScheduleEvent(oEvent);

			// Send a Local Player Info packet
			CPacket oLocalPlayerInfoPacket;
			oLocalPlayerInfoPacket.pack("hc", 0, (u_char)30);
			oLocalPlayerInfoPacket.pack("c", (u_char)PlayerGet(iLocalPlayerID)->GetName().length());
			oLocalPlayerInfoPacket.pack("t", &PlayerGet(iLocalPlayerID)->GetName());
			oLocalPlayerInfoPacket.pack("cc", cCommandRate, cUpdateRate);
			oLocalPlayerInfoPacket.CompleteTpcPacketSize();
			oLocalPlayerInfoPacket.SendTcp(UDP_CONNECTED);

			// We should be a Public client by now
			nJoinStatus = PUBLIC_CLIENT;
		}
		break;
	// Enter Game Permission
	case 6:
		// Check if we have fully joined the server
		if (nJoinStatus < PUBLIC_CLIENT) {
			printf("Error: Not yet fully joined the server.\n");
			return false;
		}

		if (nDataSize != 0) return false;		// Check packet size
		else {
			glfwLockMutex(oJoinGameMutex);
			if (bFinishedSyncingClock)
			{
				// Join Game
				NetworkJoinGame();
			}
			else
				bGotPermissionToEnter = true;
			glfwUnlockMutex(oJoinGameMutex);
		}
		break;
	// Broadcast Text Message
	case 11:
		// Check if we have entered the game
		if (nJoinStatus < IN_GAME) {
			printf("Error: Not yet entered the game.\n");
			return false;
		}

		if (nDataSize < 2) return false;		// Check packet size
		else {
			char cPlayerID;
			string sTextMessage;
			oPacket.unpack("cet", (char *)&cPlayerID, nDataSize - 1, &sTextMessage);

			// Print out the text message
			pChatMessages->AddMessage(PlayerGet(cPlayerID)->GetName() + ": " + sTextMessage);
			printf("%s\n", (PlayerGet(cPlayerID)->GetName() + ": " + sTextMessage).c_str());
		}

		break;
	// Load Level
	case 20:
		// Check if we have fully joined the server
		if (nJoinStatus < PUBLIC_CLIENT) {
			printf("Error: Not yet fully joined the server.\n");
			return false;
		}

		if (nDataSize < 1) return false;		// Check packet size
		else {
			string sLevelName;
			oPacket.unpack("et", nDataSize, &sLevelName);

			printf("Loading level '%s'.\n", sLevelName.c_str());
			string sLevelPath = "levels/" + sLevelName + ".wwl";
			GameDataOpenLevel(sLevelPath.c_str());
		}
		break;
	// Current Players Info
	case 21:
		// Check if we have fully joined the server
		if (nJoinStatus < PUBLIC_CLIENT) {
			printf("Error: Not yet fully joined the server.\n");
			return false;
		} else if (nJoinStatus >= IN_GAME) {
			printf("Error: Already entered the game, but received a Current Players Info packet.\n");
			return false;
		}

		if (nDataSize < nPlayerCount) return false;		// Check packet size
		else {
			u_char cNameLength;
			string sName;
			u_char cTeam;
			u_char cLastCommandSequenceNumber;
			float fX, fY, fZ;

			int nActivePlayers = 0;
			for (int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
			{
				oPacket.unpack("c", &cNameLength);
				if (cNameLength > 0) {
					// Active in-game player
					oPacket.unpack("etc", (int)cNameLength, &sName, &cTeam);
					PlayerGet(nPlayer)->bConnected = true;
					PlayerGet(nPlayer)->SetName(sName);
					PlayerGet(nPlayer)->SetTeam((int)cTeam);
					if (cTeam != 2)
					{
						oPacket.unpack("cfff", &cLastCommandSequenceNumber, &fX, &fY, &fZ);
						//cCurrentCommandSequenceNumber = cLastCommandSequenceNumber;
						PlayerGet(nPlayer)->cLastAckedCommandSequenceNumber = cLastCommandSequenceNumber;
						if (nPlayer == iLocalPlayerID) {
							eX0_assert(false, "local player can't be on a non-spectator team already!");
						} else {
							PlayerGet(nPlayer)->Position(fX, fY, fZ, cLastCommandSequenceNumber);
						}
					}
					// Set the player tick time
					PlayerGet(nPlayer)->fTickTime = 1.0f / cCommandRate;
					if (nPlayer == iLocalPlayerID)
					{
						// Reset the sequence numbers for the local player
						// TODO: cCurrentCommandSequenceNumber will no longer start off at 0, so find a way to set it to the correct value
						cCurrentCommandSequenceNumber = 0;
						//cLastAckedCommandSequenceNumber = 0;
						cLastUpdateSequenceNumber = 0;
						oUnconfirmedMoves.clear();
					} else {
						PlayerGet(nPlayer)->fTicks = 0.0f;
					}

					++nActivePlayers;
				} else {
					// Inactive player slot
					PlayerGet(nPlayer)->bConnected = false;
				}
			}

			printf("%d player%s already in game.\n", nActivePlayers - 1, (nActivePlayers - 1 == 1 ? "" : "s"));
		}
		break;
	// Player Joined Server
	case 25:
		// Check if we have fully joined the server
		if (nJoinStatus < PUBLIC_CLIENT) {
			printf("Error: Not yet fully joined the server.\n");
			return false;
		}

		if (nDataSize < 3) return false;		// Check packet size
		else {
			u_char cPlayerID;
			u_char cNameLength;
			string sName;
			oPacket.unpack("cc", &cPlayerID, &cNameLength);
			oPacket.unpack("et", (int)cNameLength, &sName);
			if (cPlayerID == iLocalPlayerID)
				printf("Got a Player Joined Server packet, with the local player ID %d.", iLocalPlayerID);
			if (PlayerGet(cPlayerID)->bConnected == true)
				printf("Got a Player Joined Server packet, but player %d was already in game.\n", cPlayerID);
			PlayerGet(cPlayerID)->bConnected = true;
			PlayerGet(cPlayerID)->SetName(sName);
			PlayerGet(cPlayerID)->SetTeam(2);

			// Set the other player tick time
			PlayerGet(cPlayerID)->fTickTime = 1.0f / cCommandRate;

			printf("Player #%d (name '%s') is connecting (in Info Exchange)...\n", cPlayerID, sName.c_str());
			// This is a kinda a lie, he's still connecting, in Info Exchange part; should display this when server gets Entered Game Notification (7) packet
			pChatMessages->AddMessage(PlayerGet(cPlayerID)->GetName() + " is entering game.");

			// Send an ACK packet
			// TODO
		}
		break;
	// Player Left Server
	case 26:
		// Check if we have fully joined the server
		if (nJoinStatus < PUBLIC_CLIENT) {
			printf("Error: Not yet fully joined the server.\n");
			return false;
		}

		if (nDataSize != 1) return false;		// Check packet size
		else {
			u_char cPlayerID;
			oPacket.unpack("c", &cPlayerID);

			if (cPlayerID == iLocalPlayerID)
				printf("Got a Player Left Server packet, with the local player ID %d.", iLocalPlayerID);
			if (PlayerGet(cPlayerID)->bConnected == false)
				printf("Got a Player Left Server packet, but player %d was not in game.\n", cPlayerID);
			PlayerGet((int)cPlayerID)->bConnected = false;

			printf("Player #%d (name '%s') has left the game.\n", cPlayerID, PlayerGet(cPlayerID)->GetName().c_str());
			pChatMessages->AddMessage(PlayerGet((int)cPlayerID)->GetName() + " left the game.");

			// Send an ACK packet
			// TODO
		}
		break;
	// Player Joined Team
	case 28:
		// Check if we have fully joined the server
		if (nJoinStatus < PUBLIC_CLIENT) {
			printf("Error: Not yet fully joined the server.\n");
			return false;
		}

		if (nDataSize < 2) return false;		// Check packet size
		else {
glfwLockMutex(oPlayerTick);

			u_char cPlayerID;
			u_char cTeam;
			u_char cLastCommandSequenceNumber;
			float fX, fY, fZ;
			oPacket.unpack("cc", &cPlayerID, &cTeam);

			eX0_assert(nJoinStatus >= IN_GAME || cPlayerID != iLocalPlayerID, "We should be IN_GAME if we receive a Player Joined Team packet about us.");

			if (PlayerGet(cPlayerID)->bConnected == false) printf("Got a Player Joined Team packet, but player %d was not connected.\n", cPlayerID);
			PlayerGet(cPlayerID)->SetTeam(cTeam);
			if (PlayerGet(cPlayerID)->GetTeam() != 2)
			{
				// This resets the variables and increments the Command Series Number
				PlayerGet(cPlayerID)->RespawnReset();

				oPacket.unpack("cfff", &cLastCommandSequenceNumber, &fX, &fY, &fZ);
				PlayerGet(cPlayerID)->cLastAckedCommandSequenceNumber = cLastCommandSequenceNumber;
//printf("cLastAckedCommandSequenceNumber (in packet 28) = %d, while cCurrentCommandSequenceNumber = %d\n", cLastCommandSequenceNumber, cCurrentCommandSequenceNumber);
				PlayerGet(cPlayerID)->Position(fX, fY, fZ, PlayerGet(cPlayerID)->cLastAckedCommandSequenceNumber);
			}

			printf("Player #%d (name '%s') joined team %d.\n", cPlayerID, PlayerGet(cPlayerID)->GetName().c_str(), cTeam);
			pChatMessages->AddMessage(((int)cPlayerID == iLocalPlayerID ? "Joined " : PlayerGet(cPlayerID)->GetName() + " joined ")
				+ (cTeam == 0 ? "team Red" : (cTeam == 1 ? "team Blue" : "Spectators")) + ".");

			if (cPlayerID == iLocalPlayerID)
			{
				oUnconfirmedMoves.clear();
				bSelectTeamReady = true;
			}

glfwUnlockMutex(oPlayerTick);
		}
		break;
	default:
		printf("Error: Got unknown TCP packet of type %d and data size %d.\n", cPacketType, nDataSize);
		return false;
		break;
	}

if (glfwGetKey(GLFW_KEY_TAB) == GLFW_RELEASE)
fTempFloat = static_cast<float>(glfwGetTime()) - fTempFloat;

	return true;
}

void NetworkJoinGame()
{
	// DEBUG: Perform the cCurrentCommandSequenceNumber synchronization
	double d = glfwGetTime() / (256.0 / cCommandRate);
	d -= floor(d);
	d *= 256.0;
	cCurrentCommandSequenceNumber = (u_char)d;
	PlayerGet(iLocalPlayerID)->dNextTickTime = ceil(glfwGetTime() / (1.0 / cCommandRate)) * (1.0 / cCommandRate);
	printf("abc: %f, %d\n", d, cCurrentCommandSequenceNumber);
	d -= floor(d);
	printf("tick %% = %f, nextTickAt = %.10lf\n", d*100, PlayerGet(iLocalPlayerID)->dNextTickTime);
	printf("%.8lf sec: NxtTk=%.15lf, NxtTk/12.8=%.15lf\n", glfwGetTime(), PlayerGet(iLocalPlayerID)->dNextTickTime, PlayerGet(iLocalPlayerID)->dNextTickTime / (256.0 / cCommandRate));
	PlayerGet(iLocalPlayerID)->fTicks = (float)(d * PlayerGet(iLocalPlayerID)->fTickTime);

	nJoinStatus = IN_GAME;

	// Send an Entered Game Notification packet
	CPacket oEnteredGameNotificationPacket;
	oEnteredGameNotificationPacket.pack("hc", 0, (u_char)7);
	oEnteredGameNotificationPacket.CompleteTpcPacketSize();
	oEnteredGameNotificationPacket.SendTcp();

	// Start the game
	printf("Entered the game.\n");
	iGameState = 0;

	bSelectTeamReady = true;
}

void PrintHi(void *p)
{
	//printf("%30.20f\n", glfwGetTime());
	printf("===================== %f\n", glfwGetTime());
}

// Process a received UDP packet
bool NetworkProcessUdpPacket(CPacket & oPacket)
{
	u_int nDataSize = oPacket.size() - 1;
	u_char cPacketType; oPacket.unpack("c", &cPacketType);

	switch (cPacketType) {
	// Time Response Packet
	case 106:
		// Check if we have established a UDP connection to the server
		if (nJoinStatus < UDP_CONNECTED) {
			printf("Error: Not yet UDP connected to the server.\n");
			return false;
		}

		if (nDataSize != 9) return false;		// Check packet size
		else {
			u_char cSequenceNumber;
			double dTime;
			oPacket.unpack("cd", &cSequenceNumber, &dTime);

			++nTrpReceived;

			if (nTrpReceived <= 30) {
				double dLatency = glfwGetTime() - oSentTimeRequestPacketTimes.at(cSequenceNumber);
				if (dLatency <= dShortestLatency) {
					dShortestLatency = dLatency;
					dShortestLatencyLocalTime = glfwGetTime();
					dShortestLatencyRemoteTime = dTime;
				}
				//printf("Got a TRP #%d at %.5lf ms latency: %.5lf ms (shortest = %.5lf ms)\n", nTrpReceived, glfwGetTime() * 1000, dLatency * 1000, dShortestLatency * 1000);
			} else printf("Got an unnecessary TRP #%d packet, ignoring.\n", nTrpReceived);

			if (nTrpReceived == 30) {
				pTimedEventScheduler->RemoveEventById(nSendTimeRequestPacketEventId);

				// Adjust local clock
				glfwSetTime((glfwGetTime() - dShortestLatencyLocalTime) + dShortestLatencyRemoteTime
					+ 0.5 * dShortestLatency + 0.0000135);
				dTimePassed = 0;
				dCurTime = glfwGetTime();
				dBaseTime = dCurTime;

				CTimedEvent oEvent(ceil(glfwGetTime()), 1.0, PrintHi, NULL);
				//pTimedEventScheduler->ScheduleEvent(oEvent);

				glfwLockMutex(oJoinGameMutex);
				if (bGotPermissionToEnter)
				{
					// Join Game
					NetworkJoinGame();
				}
				else
					bFinishedSyncingClock = true;
				glfwUnlockMutex(oJoinGameMutex);
			}
		}
		break;
	// Server Update Packet
	case 2:
		// Check if we have entered the game
		if (nJoinStatus < IN_GAME) {
			printf("Error: Not yet entered the game.\n");
			return false;
		}
		/*// Check if we have established a UDP connection to the server
		if (nJoinStatus < UDP_CONNECTED) {
			printf("Error: Not yet UDP connected to the server.\n");
			return false;
		}*/

		if (nDataSize < 1 + (u_int)nPlayerCount) return false;		// Check packet size
		else {
glfwLockMutex(oPlayerTick);

			u_char cUpdateSequenceNumber;
			u_char cLastCommandSequenceNumber;
			u_char cPlayerInfo;
			float fX, fY, fZ;
			oPacket.unpack("c", &cUpdateSequenceNumber);

			if (cUpdateSequenceNumber == cLastUpdateSequenceNumber) {
				printf("Got a duplicate UDP update packet from the server, discarding.\n");
			} else if ((char)(cUpdateSequenceNumber - cLastUpdateSequenceNumber) < 0) {
				printf("Got an out of order UDP update packet from the server, discarding.\n");
			} else
			{
				++cLastUpdateSequenceNumber;
				if (cUpdateSequenceNumber != cLastUpdateSequenceNumber) {
					printf("Lost %d UDP update packet(s) from the server!\n", (char)(cUpdateSequenceNumber - cLastUpdateSequenceNumber));
				}
				cLastUpdateSequenceNumber = cUpdateSequenceNumber;

				for (int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
				{
					oPacket.unpack("c", &cPlayerInfo);
					if (cPlayerInfo == 1 && PlayerGet(nPlayer)->GetTeam() != 2) {
						// Active player
						oPacket.unpack("c", &cLastCommandSequenceNumber);
						oPacket.unpack("fff", &fX, &fY, &fZ);

						bool bNewerCommand = (cLastCommandSequenceNumber != PlayerGet(nPlayer)->cLastAckedCommandSequenceNumber);
						if (bNewerCommand) {
							PlayerGet(nPlayer)->cLastAckedCommandSequenceNumber = cLastCommandSequenceNumber;

							// Add the new state to player's state history
							SequencedState_t oSequencedState;
							oSequencedState.cSequenceNumber = PlayerGet(nPlayer)->cLastAckedCommandSequenceNumber;
							oSequencedState.oState.fX = fX;
							oSequencedState.oState.fY = fY;
							oSequencedState.oState.fZ = fZ;
							PlayerGet(nPlayer)->PushStateHistory(oSequencedState);
						}

						if (nPlayer == iLocalPlayerID && bNewerCommand)
						{
							if (cCurrentCommandSequenceNumber == PlayerGet(iLocalPlayerID)->cLastAckedCommandSequenceNumber)
							{
								Vector2 oServerPosition(fX, fY);
								//Vector2 oClientPrediction(oUnconfirmedMoves.front().oState.fX, oUnconfirmedMoves.front().oState.fY);
								Vector2 oClientPrediction(PlayerGet(iLocalPlayerID)->GetX(), PlayerGet(iLocalPlayerID)->GetY());
								// If the client prediction differs from the server's value by more than a treshold amount, snap to server's value
								if ((oServerPosition - oClientPrediction).SquaredLength() > 0.001f)
									printf("Snapping-A to server's position (%f difference):\n  server = (%.4f, %.4f), client = (%.4f, %.4f)\n", (oServerPosition - oClientPrediction).Length(),
										oServerPosition.x, oServerPosition.y, oClientPrediction.x, oClientPrediction.y);

								// DEBUG: Should make sure the player can't see other players
								// through walls, if he accidentally gets warped through a wall
								PlayerGet(iLocalPlayerID)->SetX(fX);
								PlayerGet(iLocalPlayerID)->SetY(fY);

								// All moves have been confirmed now
								oUnconfirmedMoves.clear();
							}
							else if ((char)(cCurrentCommandSequenceNumber - PlayerGet(iLocalPlayerID)->cLastAckedCommandSequenceNumber) > 0)
							{
								string str = (string)"inputs empty; " + itos(cCurrentCommandSequenceNumber) + ", " + itos(PlayerGet(iLocalPlayerID)->cLastAckedCommandSequenceNumber);
								//eX0_assert(!oLocallyPredictedInputs.empty(), str);
								eX0_assert(!oUnconfirmedMoves.empty(), str);

								// Discard all the locally predicted inputs that got deprecated by this server update
								// TODO: There's a faster way to get rid of all old useless packets at once
								while (!oUnconfirmedMoves.empty()) {
									if ((char)(PlayerGet(iLocalPlayerID)->cLastAckedCommandSequenceNumber - oUnconfirmedMoves.begin()) > 0)
									{
										// This is an outdated predicted input, the server's update supercedes it, thus it's dropped
										oUnconfirmedMoves.pop();
									} else
										break;
								}

								Vector2 oServerPosition(fX, fY);
								Vector2 oClientPrediction(oUnconfirmedMoves.front().oState.fX, oUnconfirmedMoves.front().oState.fY);
								// If the client prediction differs from the server's value by more than a treshold amount, snap to server's value
								if ((oServerPosition - oClientPrediction).SquaredLength() > 0.001f)
								{
									printf("Snapping-B to server's position (%f difference).\n", (oServerPosition - oClientPrediction).Length());

									// DEBUG: Figure out why I have this here, I'm not sure if it's correct or its purpose
									oUnconfirmedMoves.pop();

									// DEBUG: Should make sure the player can't see other players
									// through walls, if he accidents gets warped through a wall
									PlayerGet(iLocalPlayerID)->SetX(fX);
									PlayerGet(iLocalPlayerID)->SetY(fY);

									eX0_assert((char)(oUnconfirmedMoves.begin() - PlayerGet(iLocalPlayerID)->cLastAckedCommandSequenceNumber) > 0, "outdated input being used");

									// Run the simulation for all locally predicted inputs after this server update
									float fOriginalOldX = PlayerGet(iLocalPlayerID)->GetOldX();
									float fOriginalOldY = PlayerGet(iLocalPlayerID)->GetOldY();
									float fOriginalZ = PlayerGet(iLocalPlayerID)->GetZ();
									Input_t oInput;
									for (u_char it1 = oUnconfirmedMoves.begin(); it1 != oUnconfirmedMoves.end(); ++it1)
									{
										oInput = oUnconfirmedMoves[it1].oInput;

										// Set inputs
										PlayerGet(iLocalPlayerID)->MoveDirection(oInput.cMoveDirection);
										PlayerGet(iLocalPlayerID)->SetStealth(oInput.cStealth != 0);
										PlayerGet(iLocalPlayerID)->SetZ(oInput.fZ);

										// Run a tick
										PlayerGet(iLocalPlayerID)->CalcTrajs();
										PlayerGet(iLocalPlayerID)->CalcColResp();
									}
									PlayerGet(iLocalPlayerID)->SetOldX(fOriginalOldX);
									PlayerGet(iLocalPlayerID)->SetOldY(fOriginalOldY);
									PlayerGet(iLocalPlayerID)->SetZ(fOriginalZ);
								}
							} else {
								eX0_assert(false, "WTF - server is ahead of client?? confirmed command from the future lol\n");
							}

							// Drop the moves that have been confirmed now
							// TODO: There's a faster way to get rid of all old useless packets at once
							while (!oUnconfirmedMoves.empty())
							{
								if ((char)(PlayerGet(iLocalPlayerID)->cLastAckedCommandSequenceNumber - oUnconfirmedMoves.begin()) >= 0)
								{
									oUnconfirmedMoves.pop();
								} else
									break;
							}
						}
						else if (nPlayer != iLocalPlayerID && bNewerCommand)
						{
							/*PlayerGet(nPlayer)->Position(fX, fY);
							PlayerGet(nPlayer)->SetZ(fZ);*/

							/*PlayerGet(nPlayer)->SetOldX(PlayerGet(nPlayer)->GetIntX());
							PlayerGet(nPlayer)->SetOldY(PlayerGet(nPlayer)->GetIntY());
							PlayerGet(nPlayer)->SetX(fX);
							PlayerGet(nPlayer)->SetY(fY);
							PlayerGet(nPlayer)->SetZ(fZ);
							PlayerGet(nPlayer)->SetVelX(fVelX);
							PlayerGet(nPlayer)->SetVelY(fVelY);
							PlayerGet(nPlayer)->fTicks = 0.0f;*/

							/*PlayerGet(nPlayer)->SetOldX(PlayerGet(nPlayer)->GetIntX());
							PlayerGet(nPlayer)->SetOldY(PlayerGet(nPlayer)->GetIntY());
							PlayerGet(nPlayer)->SetX(fX + fVelX);
							PlayerGet(nPlayer)->SetY(fY + fVelY);
							PlayerGet(nPlayer)->fOldZ = PlayerGet(nPlayer)->GetZ();
							PlayerGet(nPlayer)->SetZ(fZ);
							PlayerGet(nPlayer)->SetVelX(fVelX);
							PlayerGet(nPlayer)->SetVelY(fVelY);
							PlayerGet(nPlayer)->fTicks = 0.0f;
							PlayerGet(nPlayer)->fUpdateTicks = 0.0f;

							PlayerGet(nPlayer)->CalcColResp();*/
						}
					}
				}
			}
		}

glfwUnlockMutex(oPlayerTick);
		break;
	// Ping Packet
	case 10:
		if (nJoinStatus < IN_GAME) {
			printf("Error: Not yet entered the game.\n");
			return false;
		}

		if (nDataSize != 4 + 2 * nPlayerCount) return false;		// Check packet size
		else {
			PingData_t oPingData;
			oPacket.unpack("cccc", &oPingData.cPingData[0], &oPingData.cPingData[1], &oPingData.cPingData[2], &oPingData.cPingData[3]);

			// Make note of the current time (t1), for own latency calculation (RTT = t2 - t1)
			oPongSentTimes.push(oPingData, glfwGetTime());

			// Respond immediately with a Pong packet
			CPacket oPongPacket;
			oPongPacket.pack("ccccc", (u_char)11, oPingData.cPingData[0], oPingData.cPingData[1], oPingData.cPingData[2], oPingData.cPingData[3]);
			oPongPacket.SendUdp();

			// Update the last latency for all players
			for (int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
			{
				u_short nLastLatency;

				oPacket.unpack("h", &nLastLatency);

				if (PlayerGet(nPlayer)->bConnected && nPlayer != iLocalPlayerID) {
					PlayerGet(nPlayer)->SetLastLatency(nLastLatency);
				}
			}
		}
		break;
	// Pung Packet
	case 12:
		if (nJoinStatus < IN_GAME) {
			printf("Error: Not yet entered the game.\n");
			return false;
		}

		if (nDataSize != 12) return false;		// Check packet size
		else {
			double dLocalTimeAtPungReceive = glfwGetTime();		// Make note of current time (t2)

			PingData_t oPingData;
			double	dServerTime;
			oPacket.unpack("ccccd", &oPingData.cPingData[0], &oPingData.cPingData[1], &oPingData.cPingData[2], &oPingData.cPingData[3], &dServerTime);

			// Get the time sent of the matching Pong packet
			if (!oPongSentTimes.MatchAndRemoveAfter(oPingData)) {
				printf("Error: We never got a ping packet with this oPingData, but got a pung packet! Doesn't make sense.");
				return false;
			} else
			{
				double dLocalTimeAtPongSend = oPongSentTimes.GetLastMatchedValue();

				// Calculate own latency and update it on the scoreboard
				double dLatency = dLocalTimeAtPungReceive - dLocalTimeAtPongSend;
				PlayerGet(iLocalPlayerID)->SetLastLatency(static_cast<u_short>(ceil(dLatency * 10000)));
				oRecentLatency.push(dLatency, dLocalTimeAtPongSend);
				//printf("\nOwn latency is %.5lf ms. LwrQrtl = %.5lf ms\n", dLatency * 1000, oRecentLatency.LowerQuartile() * 1000);

				if (dLatency <= oRecentLatency.LowerQuartile() && oRecentLatency.well_populated())
					// || (dTimeDifference > dLatency && dLatency <= oRecentLatency.Mean() && oRecentLatency.well_populated())
				{
					// Calculate the (local minus remote) time difference: diff = (t1 + t2) / 2 - server_time
					double dTimeDifference = (dLocalTimeAtPongSend + dLocalTimeAtPungReceive) * 0.5 - dServerTime;
					oRecentTimeDifference.push(dTimeDifference, dLocalTimeAtPongSend);
					//printf("Time diff %.5lf ms (l-r) Cnted! Avg=%.5lf ms\n", dTimeDifference * 1000, oRecentTimeDifference.Mean() * 1000);

					// Perform the local time adjustment
					// TODO: Make this more robust, rethink how often to really perform this adjustment, and maybe make it smooth(er)...
					if (oRecentTimeDifference.well_populated() && oRecentTimeDifference.Signum() != 0) {
						double dAverageTimeDifference = -oRecentTimeDifference.WeightedMovingAverage();
						glfwSetTime(dAverageTimeDifference + glfwGetTime());
						oRecentTimeDifference.clear();
						printf("Performed time adjustment by %.5lf ms.\n", dAverageTimeDifference * 1000);
					}
				} //else printf("Time diff %.5lf ms (l-r)\n", ((dLocalTimeAtPongSend + dLocalTimeAtPungReceive) * 0.5 - dServerTime) * 1000);
			}
		}
		break;
	default:
		printf("Error: Got unknown UDP packet of type %d and data size %d.\n", cPacketType, nDataSize);
		return false;
		break;
	}

	return true;
}

void NetworkSendUdpHandshakePacket(void *pArgument)
{
	printf("Sending a UDP Handshake packet...\n");

	// Send UDP packet with the same signature to initiate the UDP handshake
	CPacket oUdpHandshakePacket;
	oUdpHandshakePacket.pack("c", (u_char)100);
	for (int nSignatureByte = 0; nSignatureByte < SIGNATURE_SIZE; ++nSignatureByte)
		oUdpHandshakePacket.pack("c", cSignature[nSignatureByte]);
	oUdpHandshakePacket.SendUdp(ACCEPTED);
}

void NetworkShutdownThread()
{
	if (oNetworkThread >= 0)
	{
		bNetworkThreadRun = false;

		// DEBUG: A hack to send ourselves an empty UDP packet in order to get out of select()
		sendto(nServerUdpSocket, NULL, 0, 0, (struct sockaddr *)&oLocalUdpAddress, nLocalUdpAddressLength);

		shutdown(nServerTcpSocket, SD_BOTH);
		shutdown(nServerUdpSocket, SD_BOTH);
	}
}

void NetworkDestroyThread()
{
	if (oNetworkThread >= 0)
	{
		glfwWaitThread(oNetworkThread, GLFW_WAIT);
		//glfwDestroyThread(oNetworkThread);
		oNetworkThread = -1;

		printf("Network thread has been destroyed.\n");
	}
}

// Shutdown the networking component
void NetworkDeinit()
{
	NetworkShutdownThread();
	NetworkDestroyThread();

	NetworkCloseSocket(nServerTcpSocket);
	NetworkCloseSocket(nServerUdpSocket);

#ifdef WIN32
	WSACleanup();
#else // Linux
	// Nothing to be done on Linux
#endif

	glfwDestroyMutex(oTcpSendMutex);
	glfwDestroyMutex(oUdpSendMutex);
	glfwDestroyMutex(oPlayerTick);
	glfwDestroyMutex(oJoinGameMutex);
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
