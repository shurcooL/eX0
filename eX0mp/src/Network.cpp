#include "globals.h"

ServerConnection *	pServer = NULL;
//SOCKET			nServerTcpSocket = INVALID_SOCKET;
//SOCKET			nServerUdpSocket = INVALID_SOCKET;
sockaddr_in	oLocalUdpAddress;		// Get the local UDP port
socklen_t			nLocalUdpAddressLength = sizeof(oLocalUdpAddress);
//sockaddr_in	oServerAddress;
//volatile int	nJoinStatus = DISCONNECTED;
//u_char			cSignature[SIGNATURE_SIZE];
u_int			nSendUdpHandshakePacketEventId = 0;

Thread *		pNetworkThread;
//GLFWthread		oNetworkThread = -1;
//volatile bool	bNetworkThreadRun;
GLFWmutex		oTcpSendMutex;
GLFWmutex		oUdpSendMutex;

//u_char			g_cCurrentCommandSequenceNumber = 0;
double			g_dNextTickTime = GLFW_INFINITY;
GLFWmutex		oPlayerTick;

//IndexedCircularBuffer<Move_t, u_char>	oUnconfirmedMoves;

MovingAverage	oRecentLatency(60.0, 10);
MovingAverage	oRecentTimeDifference(60.0, 10);
HashMatcher<PingData_t, double>	oPongSentTimes(PING_SENT_TIMES_HISTORY);
std::vector<double>		oSentTimeRequestPacketTimes(256);
u_char			cNextTimeRequestSequenceNumber = 0;
double			dShortestLatency = 1000;
double			dShortestLatencyLocalTime;
double			dShortestLatencyRemoteTime;
u_int			nTrpReceived = 0;
u_int			nSendTimeRequestPacketEventId = 0;

u_char			g_cCommandRate = 20;
u_char			g_cUpdateRate = 19;

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

void SendTimeRequestPacket(void *)
{
	static u_int nTrpSent = 0;

	// Send a Time Request Packet
	CPacket oTimeRequestPacket;
	oTimeRequestPacket.pack("cc", (u_char)105, cNextTimeRequestSequenceNumber);
	oSentTimeRequestPacketTimes.at(cNextTimeRequestSequenceNumber) = g_pGameSession->LogicTimer().GetRealTime();
	++cNextTimeRequestSequenceNumber;
	pServer->SendUdp(oTimeRequestPacket, UDP_CONNECTED);

	//printf("Sent TRqP #%d at %.5lf ms\n", ++nTrpSent, g_pGameSession->LogicTimer().GetRealTime() * 1000);
}

// Connect to a server
bool NetworkConnect(const char * szHostname, u_short nPort)
{
	if (szHostname == NULL || *szHostname == '\0')
		pServer = new LocalServerConnection();
	else
		pServer = new ServerConnection();

	if (!pServer->Connect(szHostname, nPort)) {
		return false;
	}

	return true;
}

bool NetworkCreateThread()
{
	pNetworkThread = new Thread(&NetworkThread, NULL, "Network");

	return true;
}

void GLFWCALL NetworkThread(void * pArgument)
{
	Thread * pThread = Thread::GetThisThreadAndRevertArgument(pArgument);
	FpsCounter * pFpsCounter = pThread->GetFpsCounter();

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
	FD_SET(pServer->GetTcpSocket(), &master);
	FD_SET(pServer->GetUdpSocket(), &master);

	fdmax = std::max<int>((int)pServer->GetTcpSocket(), (int)pServer->GetUdpSocket());

	while (pThread->ShouldBeRunning())
	{
		pFpsCounter->IncrementCounter();

		read_fds = master; // copy it
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == SOCKET_ERROR) {
			NetworkPrintError("select");
			Terminate(1);
		}

		if (!pThread->ShouldBeRunning()) {
			break;
		}
		// have TCP data
		else if (FD_ISSET(pServer->GetTcpSocket(), &read_fds))
		{
			// got error or connection closed by server
			if ((nbytes = recv(pServer->GetTcpSocket(), reinterpret_cast<char *>(buf) + snCurrentPacketSize, sizeof(buf) - snCurrentPacketSize, 0)) <= 0) {
				if (nbytes == 0) {
					// Connection closed gracefully by the server
					printf("Connection closed gracefully by the server.\n");
				} else {
					NetworkPrintError("recv");
					printf("Lost connection to the server.\n");
				}
				pThread->RequestStop();
				pServer->SetJoinStatus(DISCONNECTED);
				iGameState = 1;
			// we got some data from a client, process it
			} else {
				//printf("Got %d bytes from server\n", nbytes);

				snCurrentPacketSize += static_cast<u_short>(nbytes);
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

		if (!pThread->ShouldBeRunning()) {
			break;
		}
		// have UDP data
		else if (FD_ISSET(pServer->GetUdpSocket(), &read_fds))
		{
			// handle UDP data from the server
			if ((nbytes = recv(pServer->GetUdpSocket(), reinterpret_cast<char *>(cUdpBuffer), sizeof(cUdpBuffer), 0)) == SOCKET_ERROR)
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

	pThread->ThreadEnded();
}

// Process a received TCP packet
bool NetworkProcessTcpPacket(CPacket & oPacket)
{
	u_short nDataSize; oPacket.unpack("h", &nDataSize);
	u_char cPacketType; oPacket.unpack("c", &cPacketType);

	switch (cPacketType) {
	// Join Server Accept
	case 2:
		// Check if we have already been accepted by the server
		if (pServer->GetJoinStatus() >= ACCEPTED) {
			printf("Error: Already accepted, but received another Join Server Accept packet.\n");
			return false;
		}

		if (nDataSize != 2) return false;		// Check packet size
		else {
			u_char cLocalPlayerId;
			u_char cPlayerCount;
			oPacket.unpack("cc", &cLocalPlayerId, (char *)&cPlayerCount);

			nPlayerCount = (int)cPlayerCount + 1;

			//PlayerInit();
			pLocalPlayer = new CPlayer(static_cast<u_int>(cLocalPlayerId));
			pLocalPlayer->m_pController = new HidController(*pLocalPlayer);
			pLocalPlayer->m_pStateAuther = new NetworkStateAuther(*pLocalPlayer);

			// DEBUG: Change the fake 'new RemoteCC' to the real ServerConnection (pServer) perhaps?
			(new RemoteClientConnection())->SetPlayer(pLocalPlayer);

			// TODO: Player name (and other local settings?) needs to be assigned, validated (and corrected if needed) in a better way
			pLocalPlayer->SetName(sLocalPlayerName);

			// Got successfully accepted in game by the server
			pServer->SetJoinStatus(ACCEPTED);
			printf("Got accepted in game: local player id = %d, player count = %d\n", pLocalPlayer->iID, nPlayerCount);

			// Bind the UDP socket to the server
			// This will have to be done in ServerConnection, if at all... Gotta think if it's worth having two different SendUdp()s
			/*if (connect(pServer->GetUdpSocket(), (const sockaddr *)&pServer->GetUdpAddress(), sizeof(pServer->GetUdpAddress())) == SOCKET_ERROR) {
				NetworkPrintError("connect");
				Terminate(1);
			}
			if (getsockname(pServer->GetUdpSocket(), (sockaddr *)&oLocalUdpAddress, &nLocalUdpAddressLength) == SOCKET_ERROR) {
				NetworkPrintError("getsockname");
				Terminate(1);
			}
			printf("Created a pending UDP connection to server on port %d.\n", ntohs(oLocalUdpAddress.sin_port));*/

			// Send UDP packet with the same signature to initiate the UDP handshake
			// Add timed event (Retransmit UdpHandshake packet every 100 milliseconds)
			CTimedEvent oEvent(0, UDP_HANDSHAKE_RETRY_TIME, &NetworkSendUdpHandshakePacket, NULL);
			nSendUdpHandshakePacketEventId = oEvent.GetId();
			pTimedEventScheduler->ScheduleEvent(oEvent);
		}
		break;
	// Join Game Refuse
	case 3:
		// Check if we have already joined the game
		if (pServer->GetJoinStatus() >= IN_GAME) {
			printf("Error: Already in game, but received a Join Game Refuse packet.\n");
			return false;
		}

		if (nDataSize != 1) return false;		// Check packet size
		else {
			char cRefuseReason;
			oPacket.unpack("c", &cRefuseReason);

			printf("Got refused with reason %d.\n", (int)cRefuseReason);

			pNetworkThread->RequestStop();
			pServer->SetJoinStatus(DISCONNECTED);		// Server connection status is fully unconnected
			iGameState = 1;
		}
		break;
	// UDP Connection Established
	case 5:
		// Check if we have been accepted by the server
		if (pServer->GetJoinStatus() < ACCEPTED) {
			printf("Error: Not yet accepted by the server.\n");
			return false;
		}

		if (nDataSize != 0) return false;		// Check packet size
		else {
			// The UDP connection with the server is fully established
			pServer->SetJoinStatus(UDP_CONNECTED);
			printf("Established a UDP connection with the server.\n");

			// Stop sending UDP handshake packets
			pTimedEventScheduler->RemoveEventById(nSendUdpHandshakePacketEventId);

			// Start syncing the clock, send a Time Request packet every 50 ms
			CTimedEvent oEvent(0, TIME_REQUEST_SEND_RATE, &SendTimeRequestPacket, NULL);
			nSendTimeRequestPacketEventId = oEvent.GetId();
			pTimedEventScheduler->ScheduleEvent(oEvent);

			// Send a Local Player Info packet
			CPacket oLocalPlayerInfoPacket;
			oLocalPlayerInfoPacket.pack("hc", 0, (u_char)30);
			oLocalPlayerInfoPacket.pack("c", (u_char)pLocalPlayer->GetName().length());
			oLocalPlayerInfoPacket.pack("t", &pLocalPlayer->GetName());
			oLocalPlayerInfoPacket.pack("cc", g_cCommandRate, g_cUpdateRate);
			oLocalPlayerInfoPacket.CompleteTpcPacketSize();
			pServer->SendTcp(oLocalPlayerInfoPacket, UDP_CONNECTED);

			// We should be a Public client by now
			pServer->SetJoinStatus(PUBLIC_CLIENT);
		}
		break;
	// Enter Game Permission
	case 6:
		// Check if we have fully joined the server
		if (pServer->GetJoinStatus() < PUBLIC_CLIENT) {
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
		if (pServer->GetJoinStatus() < IN_GAME) {
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
		if (pServer->GetJoinStatus() < PUBLIC_CLIENT) {
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
		if (pServer->GetJoinStatus() < PUBLIC_CLIENT) {
			printf("Error: Not yet fully joined the server.\n");
			return false;
		} else if (pServer->GetJoinStatus() >= IN_GAME) {
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
			for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
			{
				oPacket.unpack("c", &cNameLength);
				if (cNameLength > 0) {
					// Active in-game player
					oPacket.unpack("etc", (int)cNameLength, &sName, &cTeam);

					if (nPlayer != pLocalPlayer->iID) {
						new CPlayer(nPlayer);
						PlayerGet(nPlayer)->m_pController = NULL;		// No player controller
						PlayerGet(nPlayer)->m_pStateAuther = new NetworkStateAuther(*PlayerGet(nPlayer));

						(new RemoteClientConnection())->SetPlayer(PlayerGet(nPlayer));
					}

					PlayerGet(nPlayer)->SetName(sName);
					PlayerGet(nPlayer)->SetTeam((int)cTeam);
					if (cTeam != 2)
					{
						eX0_assert(nPlayer != pLocalPlayer->iID, "local player can't be on a non-spectator team already!");

						oPacket.unpack("cfff", &cLastCommandSequenceNumber, &fX, &fY, &fZ);
						//cCurrentCommandSequenceNumber = cLastCommandSequenceNumber;
						//PlayerGet(nPlayer)->oLatestAuthStateTEST.cSequenceNumber = cLastCommandSequenceNumber;
						//static_cast<NetworkStateAuther *>(PlayerGet(nPlayer)->m_pStateAuther)->cLastAckedCommandSequenceNumber = cLastCommandSequenceNumber;

						PlayerGet(nPlayer)->Position(fX, fY, fZ, cLastCommandSequenceNumber);
					}
					// Set the player tick time
					//PlayerGet(nPlayer)->fTickTime = 1.0f / g_cCommandRate;
					if (nPlayer == pLocalPlayer->iID)
					{
						// Reset the sequence numbers for the local player
					} else {
						//PlayerGet(nPlayer)->fTicks = 0.0f;
					}

					++nActivePlayers;
				}
			}

			printf("%d player%s already in game.\n", nActivePlayers - 1, (nActivePlayers - 1 == 1 ? "" : "s"));
		}
		break;
	// Player Joined Server
	case 25:
		// Check if we have fully joined the server
		if (pServer->GetJoinStatus() < PUBLIC_CLIENT) {
			printf("Error: Not yet fully joined the server.\n");
			return false;
		}

		if (nDataSize < 3) return false;		// Check packet size
		else {
glfwLockMutex(oPlayerTick);

			u_char cPlayerId;
			u_char cNameLength;
			string sName;
			oPacket.unpack("cc", &cPlayerId, &cNameLength);
			oPacket.unpack("et", (int)cNameLength, &sName);
			if (cPlayerId == pLocalPlayer->iID)
				printf("Got a Player Joined Server packet, with the local player ID %d.", pLocalPlayer->iID);

			if (false == pServer->IsLocal())
			{
				if (PlayerGet(cPlayerId) != NULL) {
					printf("Got a Player Joined Server packet, but player %d was already in game.\n", cPlayerId);
					return false;
				}

				new CPlayer(static_cast<u_int>(cPlayerId));
				PlayerGet(cPlayerId)->m_pController = NULL;		// No player controller
				PlayerGet(cPlayerId)->m_pStateAuther = new NetworkStateAuther(*PlayerGet(cPlayerId));

				(new RemoteClientConnection())->SetPlayer(PlayerGet(cPlayerId));

				PlayerGet(cPlayerId)->SetName(sName);
				PlayerGet(cPlayerId)->SetTeam(2);
			}

			printf("Player #%d (name '%s') is connecting (in Info Exchange)...\n", cPlayerId, sName.c_str());
			// This is a kinda a lie, he's still connecting, in Info Exchange part; should display this when server gets Entered Game Notification (7) packet
			pChatMessages->AddMessage(PlayerGet(cPlayerId)->GetName() + " is entering game.");

			// TODO: Send an ACK packet

glfwUnlockMutex(oPlayerTick);
		}
		break;
	// Player Left Server
	case 26:
		// Check if we have fully joined the server
		if (pServer->GetJoinStatus() < PUBLIC_CLIENT) {
			printf("Error: Not yet fully joined the server.\n");
			return false;
		}

		if (nDataSize != 1) return false;		// Check packet size
		else {
glfwLockMutex(oPlayerTick);

			u_char cPlayerID;
			oPacket.unpack("c", &cPlayerID);

			if (cPlayerID == pLocalPlayer->iID)
				printf("WARNING: Got a Player Left Server packet, with the local player ID %d.", pLocalPlayer->iID);
			if (PlayerGet(cPlayerID) == NULL) {
				printf("WARNING: Got a Player Left Server packet, but player %d was not in game.\n", cPlayerID);
				return false;
			}

			printf("Player #%d (name '%s') has left the game.\n", cPlayerID, PlayerGet(cPlayerID)->GetName().c_str());
			pChatMessages->AddMessage(PlayerGet((int)cPlayerID)->GetName() + " left the game.");

			delete PlayerGet((int)cPlayerID);

			// TODO: Send an ACK packet

glfwUnlockMutex(oPlayerTick);
		}
		break;
	// Player Joined Team
	case 28:
		// Check if we have fully joined the server
		if (pServer->GetJoinStatus() < PUBLIC_CLIENT) {
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

			eX0_assert(pServer->GetJoinStatus() >= IN_GAME || cPlayerID != pLocalPlayer->iID, "We should be IN_GAME if we receive a Player Joined Team packet about us.");
			if (PlayerGet(cPlayerID) == NULL) { printf("ERROR: Got a Player Joined Team packet, but player %d was not connected.\n", cPlayerID); return false; }

			printf("Pl#%d ('%s') joined team %d at logic time %f/%d [client].\n", cPlayerID, PlayerGet(cPlayerID)->GetName().c_str(), cTeam, g_pGameSession->LogicTimer().GetGameTime(), g_pGameSession->GlobalStateSequenceNumberTEST);

			if (false == pServer->IsLocal()) {
				PlayerGet(cPlayerID)->SetTeam(cTeam);
				if (PlayerGet(cPlayerID)->GetTeam() != 2)
				{
					// This resets the variables
					PlayerGet(cPlayerID)->RespawnReset();

					// Increment the Command packet series
					static_cast<NetworkStateAuther *>(PlayerGet(cPlayerID)->m_pStateAuther)->cCurrentCommandSeriesNumber += 1;

					oPacket.unpack("cfff", &cLastCommandSequenceNumber, &fX, &fY, &fZ);
					//static_cast<NetworkStateAuther *>(PlayerGet(cPlayerID)->m_pStateAuther)->cLastAckedCommandSequenceNumber = cLastCommandSequenceNumber;
					PlayerGet(cPlayerID)->GlobalStateSequenceNumberTEST = cLastCommandSequenceNumber;

					// Safe to do this here, because we're in Network thread (so can't be pushing into Queue),
					// and using PlayerTickMutex, so Logic thread can't be popping from Queue
					PlayerGet(cPlayerID)->m_oCommandsQueue.clear();
					PlayerGet(cPlayerID)->m_oUpdatesQueue.clear();

					PlayerGet(cPlayerID)->Position(fX, fY, fZ, cLastCommandSequenceNumber);
				}
			}

			pChatMessages->AddMessage(((int)cPlayerID == pLocalPlayer->iID ? "Joined " : PlayerGet(cPlayerID)->GetName() + " joined ")
				+ (cTeam == 0 ? "team Red" : (cTeam == 1 ? "team Blue" : "Spectators")) + ".");

			if (cPlayerID == pLocalPlayer->iID)
			{
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

	return true;
}

void NetworkJoinGame()
{
	// DEBUG: Perform the cCurrentCommandSequenceNumber synchronization
	double d = g_pGameSession->LogicTimer().GetRealTime() / (256.0 / g_cCommandRate);
	d -= static_cast<uint32>(d);
	d *= 256;
	g_pGameSession->GlobalStateSequenceNumberTEST = (u_char)d + 1;
	g_dNextTickTime = ceil(g_pGameSession->LogicTimer().GetRealTime() / (1.0 / g_cCommandRate)) * (1.0 / g_cCommandRate);
	printf("abc: %f, %d, %f/%f\n", d, g_pGameSession->GlobalStateSequenceNumberTEST, g_dNextTickTime, g_pGameSession->LogicTimer().GetRealTime());
	d -= floor(d);
	printf("tick %% = %f, nextTickAt = %.10lf\n", d*100, g_dNextTickTime);
	printf("%.8lf sec: NxtTk=%.15lf, NxtTk/12.8=%.15lf\n", g_pGameSession->LogicTimer().GetRealTime(), g_dNextTickTime, g_dNextTickTime / (256.0 / g_cCommandRate));
	pLocalPlayer->GlobalStateSequenceNumberTEST = (u_char)d;

	pServer->SetJoinStatus(IN_GAME);

	// Send an Entered Game Notification packet
	CPacket oEnteredGameNotificationPacket;
	oEnteredGameNotificationPacket.pack("hc", 0, (u_char)7);
	oEnteredGameNotificationPacket.CompleteTpcPacketSize();
	pServer->SendTcp(oEnteredGameNotificationPacket);

	// Start the game
	printf("Entered the game.\n");
	iGameState = 0;

	bSelectTeamDisplay = true;
	bSelectTeamReady = true;
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
		if (pServer->GetJoinStatus() < UDP_CONNECTED) {
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
				double dLatency = g_pGameSession->LogicTimer().GetRealTime() - oSentTimeRequestPacketTimes.at(cSequenceNumber);
				if (dLatency <= dShortestLatency) {
					dShortestLatency = dLatency;
					dShortestLatencyLocalTime = g_pGameSession->LogicTimer().GetRealTime();
					dShortestLatencyRemoteTime = dTime;
				}
				//printf("Got a TRP #%d at %.5lf ms latency: %.5lf ms (shortest = %.5lf ms)\n", nTrpReceived, g_pGameSession->LogicTimer().GetRealTime() * 1000, dLatency * 1000, dShortestLatency * 1000);
			} else printf("Got an unnecessary TRP #%d packet, ignoring.\n", nTrpReceived);

			if (nTrpReceived == 30) {
				pTimedEventScheduler->RemoveEventById(nSendTimeRequestPacketEventId);

				// Adjust local clock
				/*glfwSetTime((glfwGetTime() - dShortestLatencyLocalTime) + dShortestLatencyRemoteTime
					+ 0.5 * dShortestLatency + 0.0000135);
				dTimePassed = 0;
				dCurTime = glfwGetTime();
				dBaseTime = dCurTime;*/
				g_pGameSession->LogicTimer().SetTime((g_pGameSession->LogicTimer().GetRealTime() - dShortestLatencyLocalTime)
					+ dShortestLatencyRemoteTime + 0.5 * dShortestLatency + 0.0000135);

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
		if (pServer->GetJoinStatus() < IN_GAME) {
			printf("Error: Not yet entered the game.\n");
			return false;
		}
		/*// Check if we have established a UDP connection to the server
		if (pServer->GetJoinStatus() < UDP_CONNECTED) {
			printf("Error: Not yet UDP connected to the server.\n");
			return false;
		}*/

		if (nDataSize < 1 + (u_int)nPlayerCount) return false;		// Check packet size
		else {
glfwLockMutex(oPlayerTick);

			NetworkStateAuther::ProcessUpdate(oPacket);

glfwUnlockMutex(oPlayerTick);
		}
		break;
	// Ping Packet
	case 10:
		if (pServer->GetJoinStatus() < IN_GAME) {
			printf("Error: Not yet entered the game.\n");
			return false;
		}

		if (nDataSize != 4 + 2 * nPlayerCount) return false;		// Check packet size
		else {
			PingData_t oPingData;
			oPacket.unpack("cccc", &oPingData.cPingData[0], &oPingData.cPingData[1], &oPingData.cPingData[2], &oPingData.cPingData[3]);

			// Make note of the current time (t1), for own latency calculation (RTT = t2 - t1)
			oPongSentTimes.push(oPingData, g_pGameSession->LogicTimer().GetRealTime());

			// Respond immediately with a Pong packet
			CPacket oPongPacket;
			oPongPacket.pack("ccccc", (u_char)11, oPingData.cPingData[0], oPingData.cPingData[1], oPingData.cPingData[2], oPingData.cPingData[3]);
			pServer->SendUdp(oPongPacket);

/*double t2d=0, t2=glfwGetTime();
printf("test message nothing more\n");
double t3d=0, t3=glfwGetTime();
printf("\nt3-t2 = %f ms\n", (t3-t2)*1000);
printf("t-t = %f ms\n", -(glfwGetTime()-glfwGetTime())*1000);*/
//double t1=glfwGetTime();
			// Update the last latency for all players
			for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
			{
				u_short nLastLatency;

				oPacket.unpack("h", &nLastLatency);

				if (PlayerGet(nPlayer) != NULL && PlayerGet(nPlayer) != pLocalPlayer) {
					PlayerGet(nPlayer)->pConnection->SetLastLatency(nLastLatency);
				}
			}
//double t1d=glfwGetTime()-t1;
//printf("wasted %f ms\n", t1d*1000);
/*printf(" of those %f ms in t2\n", t2d*1000);
printf("      and %f ms in t3\n", t3d*1000);*/
		}
		break;
	// Pung Packet
	case 12:
		if (pServer->GetJoinStatus() < IN_GAME) {
			printf("Error: Not yet entered the game.\n");
			return false;
		}

		if (nDataSize != 12) return false;		// Check packet size
		else {
			double dLocalTimeAtPungReceive = g_pGameSession->LogicTimer().GetRealTime();		// Make note of current time (t2)

			PingData_t oPingData;
			double	dServerTime;
			oPacket.unpack("ccccd", &oPingData.cPingData[0], &oPingData.cPingData[1], &oPingData.cPingData[2], &oPingData.cPingData[3], &dServerTime);

			// Get the time sent of the matching Pong packet
			try {
				double dLocalTimeAtPongSend = oPongSentTimes.MatchAndRemoveAfter(oPingData);

				// Calculate own latency and update it on the scoreboard
				double dLatency = dLocalTimeAtPungReceive - dLocalTimeAtPongSend;
				pLocalPlayer->pConnection->SetLastLatency(static_cast<u_short>(ceil(dLatency * 10000)));
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
						//glfwSetTime(dAverageTimeDifference + glfwGetTime());
						g_pGameSession->LogicTimer().SetTime(g_pGameSession->LogicTimer().GetRealTime() + dAverageTimeDifference);
						oRecentTimeDifference.clear();
						printf("Performed time adjustment by %.5lf ms.\n", dAverageTimeDifference * 1000);
					}
				} //else printf("Time diff %.5lf ms (l-r)\n", ((dLocalTimeAtPongSend + dLocalTimeAtPungReceive) * 0.5 - dServerTime) * 1000);
			} catch (...) {
				printf("Error: We never got a ping packet with this oPingData, but got a pung packet! Doesn't make sense.");
				return false;
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

void NetworkSendUdpHandshakePacket(void *)
{
	printf("Sending a UDP Handshake packet...\n");

	// Send UDP packet with the same signature to initiate the UDP handshake
	CPacket oUdpHandshakePacket;
	oUdpHandshakePacket.pack("c", (u_char)100);
	for (int nSignatureByte = 0; nSignatureByte < NetworkConnection::m_knSignatureSize; ++nSignatureByte)
		oUdpHandshakePacket.pack("c", pServer->GetSignature()[nSignatureByte]);
	pServer->SendUdp(oUdpHandshakePacket, ACCEPTED);
}

void NetworkShutdownThread()
{
	if (pNetworkThread != NULL && pNetworkThread->IsAlive())
	{
		pNetworkThread->RequestStop();

		// DEBUG: A hack to send ourselves an empty UDP packet in order to get out of select()
		sendto(pServer->GetUdpSocket(), NULL, 0, 0, (sockaddr *)&oLocalUdpAddress, nLocalUdpAddressLength);

		shutdown(pServer->GetTcpSocket(), SD_BOTH);
		shutdown(pServer->GetUdpSocket(), SD_BOTH);
	}
}

void NetworkDestroyThread()
{
	delete pNetworkThread; pNetworkThread = NULL;
}

// Shutdown the networking component
void NetworkDeinit()
{
	NetworkShutdownThread();
	NetworkDestroyThread();

	delete pServer; pServer = NULL;
	//NetworkCloseSocket(pServer->GetTcpSocket());
	//NetworkCloseSocket(pServer->GetUdpSocket());

#ifdef WIN32
	WSACleanup();
#else // Linux
	// Nothing to be done on Linux
#endif

	glfwDestroyMutex(oTcpSendMutex); oTcpSendMutex = NULL;
	glfwDestroyMutex(oUdpSendMutex); oUdpSendMutex = NULL;
	glfwDestroyMutex(oPlayerTick); oPlayerTick = NULL;
	glfwDestroyMutex(oJoinGameMutex); oJoinGameMutex = NULL;
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
