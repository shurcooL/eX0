// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

#ifndef EX0_CLIENT
GLFWmutex		oTcpSendMutex;
GLFWmutex		oUdpSendMutex;

u_char			g_cCurrentCommandSequenceNumber = 0;
double			g_dNextTickTime = GLFW_INFINITY;
GLFWmutex		oPlayerTick;

u_char			g_cCommandRate = 0;
u_char			g_cUpdateRate = 0;

// Initialize the networking component
bool NetworkInit()
{
	oTcpSendMutex = glfwCreateMutex();
	oUdpSendMutex = glfwCreateMutex();
	oPlayerTick = glfwCreateMutex();

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
#endif // EX0_CLIENT

// Process a received TCP packet
bool NetworkProcessTcpPacket(CPacket & oPacket, ClientConnection *& pConnection)
{
	u_short nDataSize; oPacket.unpack("h", &nDataSize);
	u_char cPacketType; oPacket.unpack("c", &cPacketType);

	switch (cPacketType) {
	// Join Server Request
	case 1:
		// Check if this client has already joined
		if (pConnection->GetJoinStatus() >= ACCEPTED) {
			printf("Error: This client has already joined, but sent another Join Server Request packet.\n");
			return false;
		}

		if (nDataSize != (18 + NetworkConnection::m_knSignatureSize)) return false;		// Check packet size
		else {
			short snVersion;
			char szPassphrase[17];
			u_char cSignature[NetworkConnection::m_knSignatureSize];
			oPacket.unpack("h16s", &snVersion, szPassphrase);
			for (int nSignatureByte = 0; nSignatureByte < NetworkConnection::m_knSignatureSize; ++nSignatureByte)
				oPacket.unpack("c", &cSignature[nSignatureByte]);

			if (memcmp(szPassphrase, NETWORK_PROTOCOL_PASSPHRASE, 16) == 0)
			{
				if (snVersion == NETWORK_PROTOCOL_VERSION)
				{
					// Valid Join Server Request packet
					printf("Got a valid Join Server Request packet! yay.. yay... ;/\n");

					// Cancel the bad client timeout
					pConnection->CancelBadClientTimeout();
				} else {
					// Drop the connection to this unsupported client
					printf("Error: Got an INVALID Join Server Request packet, dropping connection.\n");

					// Refuse join server request with reason 0 (version-mismatch)
					u_char cReason = 0;
					CPacket oJoinServerRefusePacket;
					oJoinServerRefusePacket.pack("hcc", 0, (u_char)3, cReason);
					oJoinServerRefusePacket.CompleteTpcPacketSize();
					pConnection->SendTcp(oJoinServerRefusePacket, TCP_CONNECTED);

					// Disconnect this client
					delete pConnection; pConnection = NULL;
					return true;
				}
			}
			else
			{
				// Drop the connection, since it's not a proper client
				printf("Error: Got an INVALID Join Server Request packet, dropping connection.\n");
				delete pConnection; pConnection = NULL;
				return true;
			}

			// Let the new client join only if there are free slots avaliable
			if (CPlayer::GetPlayerCount() < nPlayerCount)
			{
				printf("Accepting this player (now %d/%d).\n", CPlayer::GetPlayerCount() + 1, nPlayerCount);

				// Assign this client the avaliable player id
				pConnection->SetSignature(cSignature);
glfwLockMutex(oPlayerTick);
				pConnection->SetPlayer(new CPlayer());
				pConnection->GetPlayer()->m_pController = new NetworkController(*pConnection->GetPlayer());
				pConnection->GetPlayer()->m_pStateAuther = new LocalStateAuther(*pConnection->GetPlayer());
glfwUnlockMutex(oPlayerTick);
				pConnection->SetJoinStatus(ACCEPTED);

				// Accept the connection, send a Join Server Accept packet
				CPacket oJoinServerAcceptPacket;
				oJoinServerAcceptPacket.pack("hccc", 0, (u_char)2, (u_char)pConnection->GetPlayerID(), (u_char)(nPlayerCount - 1));
				oJoinServerAcceptPacket.CompleteTpcPacketSize();
				pConnection->SendTcp(oJoinServerAcceptPacket, TCP_CONNECTED);
			} else {
				printf("Refusing to let this player join!\n");

				// Refuse join server request with reason 1
				u_char cReason = 1;
				CPacket oJoinServerRefusePacket;
				oJoinServerRefusePacket.pack("hcc", 0, (u_char)3, cReason);
				oJoinServerRefusePacket.CompleteTpcPacketSize();
				pConnection->SendTcp(oJoinServerRefusePacket, TCP_CONNECTED);

				// Disconnect this client
				delete pConnection; pConnection = NULL;
			}
		}
		break;
	// Entered Game Notification
	case 7:
		// Check if this client has already established a UDP connection
		if (pConnection->GetJoinStatus() < PUBLIC_CLIENT) {
			printf("Error: This client hasn't yet joined, ignoring TCP packet.\n");
			return false;
		} else if (pConnection->GetJoinStatus() >= IN_GAME) {
			printf("Error: This client has already entered game, but sent another Entered Game Notification packet.\n");
			return false;
		}

		if (nDataSize != 0) return false;		// Check packet size
		else {
			pConnection->SetJoinStatus(IN_GAME);
			printf("Player #%d (name '%s') has entered the game.\n", pConnection->GetPlayerID(), pConnection->GetPlayer()->GetName().c_str());

			// Schedule the sending of Server Update packets for this client
			pConnection->GetPlayer()->m_dNextUpdateTime = glfwGetTime();
		}
		break;
	// Send Text Message
	case 10:
		// Check if this client has NOT entered game
		if (pConnection->GetJoinStatus() < IN_GAME) {
			printf("Error: This client hasn't yet entered game, ignoring TCP packet.\n");
			return false;
		}

		if (nDataSize < 1) return false;		// Check packet size
		else {
			string sTextMessage;
			oPacket.unpack("et", nDataSize, &sTextMessage);

			// Valid Send Text Message packet
			printf("%s\n", (pConnection->GetPlayer()->GetName() + ": " + sTextMessage).c_str());

			// Create a Broadcast Text Message packet
			CPacket oBroadcastMessagePacket(CPacket::BOTH);
			oBroadcastMessagePacket.pack("hcct", 0, (u_char)11, (u_char)pConnection->GetPlayerID(), &sTextMessage);
			oBroadcastMessagePacket.CompleteTpcPacketSize();

			// Broadcast the text message to all players that are IN_GAME
			ClientConnection::BroadcastTcp(oBroadcastMessagePacket);
		}
		break;
	// Join Team Request
	case 27:
		// Check if this client has NOT entered game
		if (pConnection->GetJoinStatus() < IN_GAME) {
			printf("Error: This client hasn't yet entered game.\n");
			return false;
		}

		// TODO: Make this more formal... i.e. for now, only local connections are allowed to be multiplayer
		eX0_assert(pConnection->IsLocal() || !pConnection->IsMultiPlayer(), "pConnection->IsLocal() || !pConnection->IsMultiPlayer()");

		if (!pConnection->IsMultiPlayer() && nDataSize != 1) return false;			// Check packet size
		else if (pConnection->IsMultiPlayer() && nDataSize != 2) return false;		// Check packet size
		else {
			u_char cPlayerNumber = 0;
			u_char cTeam;
			if (pConnection->IsMultiPlayer()) oPacket.unpack("c", &cPlayerNumber);
			oPacket.unpack("c", &cTeam);

glfwLockMutex(oPlayerTick);
			pConnection->GetPlayer(cPlayerNumber)->SetTeam((int)cTeam);
			printf("Player #%d (name '%s') joined team %d.\n", pConnection->GetPlayerID(cPlayerNumber), pConnection->GetPlayer(cPlayerNumber)->GetName().c_str(), cTeam);

			// Create a Player Joined Team packet
			CPacket oPlayerJoinedTeamPacket(CPacket::BOTH);
			oPlayerJoinedTeamPacket.pack("hcc", 0, (u_char)28, pConnection->GetPlayerID(cPlayerNumber));
			oPlayerJoinedTeamPacket.pack("c", (u_char)pConnection->GetPlayer(cPlayerNumber)->GetTeam());

			if (pConnection->GetPlayer(cPlayerNumber)->GetTeam() != 2)
			{
				pConnection->GetPlayer(cPlayerNumber)->RespawnReset();

				// DEBUG: Randomly position the player
				float x, y;
				do {
					x = static_cast<float>(rand() % 2000 - 1000);
					y = static_cast<float>(rand() % 2000 - 1000);
				} while (ColHandIsPointInside((int)x, (int)y) || !ColHandCheckPlayerPos(&x, &y));
				pConnection->GetPlayer(cPlayerNumber)->Position(x, y, 0.001f * (rand() % 1000) * Math::TWO_PI);
				printf("Positioning player %d at %f, %f, %f.\n", pConnection->GetPlayerID(cPlayerNumber), x, y, pConnection->GetPlayer()->GetZ());

				// DEBUG: Perform the cLastRecvedCommandSequenceNumber synchronization to time
				double d = glfwGetTime() / (256.0 / g_cCommandRate);
				d -= floor(d);
				d *= 256.0;
				if (false == pConnection->IsLocal()) {
					static_cast<NetworkController *>(pConnection->GetPlayer(cPlayerNumber)->m_pController)->cLastRecvedCommandSequenceNumber = (u_char)d;
				}
				pConnection->GetPlayer(cPlayerNumber)->cLatestAuthStateSequenceNumber = (u_char)d;

				pConnection->GetPlayer(cPlayerNumber)->m_oInputCmdsTEST.clear();			// DEBUG: Is this the right thing to do? Right place to do it?
				pConnection->GetPlayer(cPlayerNumber)->m_oAuthUpdatesTEST.clear();		// DEBUG: Is this the right thing to do? Right place to do it?

				if (false == pConnection->IsLocal()) {
					// Start expecting the first command packet from this player
					static_cast<NetworkController *>(pConnection->GetPlayer(cPlayerNumber)->m_pController)->cCurrentCommandSeriesNumber += 1;
					static_cast<NetworkController *>(pConnection->GetPlayer(cPlayerNumber)->m_pController)->bFirstCommand = true;
				}

				oPlayerJoinedTeamPacket.pack("c", pConnection->GetPlayer(cPlayerNumber)->cLatestAuthStateSequenceNumber);
				oPlayerJoinedTeamPacket.pack("fff", pConnection->GetPlayer(cPlayerNumber)->GetX(),
					pConnection->GetPlayer(cPlayerNumber)->GetY(), pConnection->GetPlayer(cPlayerNumber)->GetZ());
			}
glfwUnlockMutex(oPlayerTick);

			oPlayerJoinedTeamPacket.CompleteTpcPacketSize();
			ClientConnection::BroadcastTcp(oPlayerJoinedTeamPacket, PUBLIC_CLIENT);
		}
		break;
	// Local Player Info
	case 30:
		// Check if this client has already established a UDP connection
		if (pConnection->GetJoinStatus() < UDP_CONNECTED) {
			printf("Error: This client hasn't yet established a UDP connection, not expecting this TCP packet.\n");
			return false;
		} else if (pConnection->GetJoinStatus() >= PUBLIC_CLIENT) {
			printf("Error: This client is already a PUBLIC_CLIENT, but got a Local Player Info (30) packet packet.\n");
			return false;
		}

		if (nDataSize < 4) return false;		// Check packet size
		else {
			u_char cNameLength;
			string sName;
			u_char cCommandRate, cUpdateRate;
			oPacket.unpack("c", &cNameLength); if ((u_short)cNameLength > nDataSize - 3) cNameLength = static_cast<u_char>(nDataSize - 3);
			oPacket.unpack("et", (int)cNameLength, &sName);
			oPacket.unpack("cc", &cCommandRate, &cUpdateRate);
			PlayerGet(pConnection->GetPlayerID())->SetName(sName);
			if (g_cCommandRate == 0 && g_cUpdateRate == 0 && cCommandRate != 0 && cUpdateRate != 0) {
				g_cCommandRate = cCommandRate;
				g_cUpdateRate = cUpdateRate;
			}

			if (g_cCommandRate < 1 || g_cCommandRate > 100 ||
				g_cUpdateRate < 1 || g_cUpdateRate > 100)
			{
				printf("Client rates (%d, %d) for player %d are out of range, sending refuse packet.\n",
					g_cCommandRate, g_cUpdateRate, pConnection->GetPlayerID());

				// Refuse with reason 2
				CPacket oJoinServerRefusePacket;
				oJoinServerRefusePacket.pack("hcc", 0, (u_char)3, 2);
				oJoinServerRefusePacket.CompleteTpcPacketSize();
				pConnection->SendTcp(oJoinServerRefusePacket, TCP_CONNECTED);
			}
			else
			{
				// The client now has the Public join status
				pConnection->SetJoinStatus(PUBLIC_CLIENT);

				// Set team to Spectator by default
				pConnection->GetPlayer()->SetTeam(2);

				// Set the player tick time
				// TODO: Do something about this, see if it's needed, and remove it if not, or make it useful, etc.
				pConnection->GetPlayer()->fTickTime = 1.0f / g_cCommandRate;

				// Send a Load Level packet to load the current level
				CPacket oLoadLevelPacket;
				oLoadLevelPacket.pack("hct", 0, (u_char)20, &sLevelName);
				oLoadLevelPacket.CompleteTpcPacketSize();
				pConnection->SendTcp(oLoadLevelPacket, PUBLIC_CLIENT);

				// Send a Current Players Info to the new client
				CPacket oCurrentPlayersInfoPacket;
				oCurrentPlayersInfoPacket.pack("hc", 0, (u_char)21);
				for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
				{
					// Include the client who's connecting and all clients with at least Public status
					if ((PlayerGet(nPlayer) != NULL && (PlayerGet(nPlayer)->pConnection == NULL || PlayerGet(nPlayer)->pConnection->GetJoinStatus() >= PUBLIC_CLIENT))
						|| nPlayer == pConnection->GetPlayerID())
					{
						oCurrentPlayersInfoPacket.pack("c", (u_char)PlayerGet(nPlayer)->GetName().length());
						oCurrentPlayersInfoPacket.pack("t", &PlayerGet(nPlayer)->GetName());
						oCurrentPlayersInfoPacket.pack("c", (u_char)PlayerGet(nPlayer)->GetTeam());
						if (PlayerGet(nPlayer)->GetTeam() != 2)
						{
							oCurrentPlayersInfoPacket.pack("c", PlayerGet(nPlayer)->cLatestAuthStateSequenceNumber);
							oCurrentPlayersInfoPacket.pack("fff", PlayerGet(nPlayer)->GetX(),
								PlayerGet(nPlayer)->GetY(), PlayerGet(nPlayer)->GetZ());
						}
					} else {
						oCurrentPlayersInfoPacket.pack("c", (char)0);
					}
				}
				oCurrentPlayersInfoPacket.CompleteTpcPacketSize();
				pConnection->SendTcp(oCurrentPlayersInfoPacket, PUBLIC_CLIENT);

				// Send a Player Joined Server packet to all the other clients with Public status
				CPacket oPlayerJoinedServerPacket(CPacket::BOTH);
				oPlayerJoinedServerPacket.pack("hc", 0, (u_char)25);
				oPlayerJoinedServerPacket.pack("c", (u_char)pConnection->GetPlayerID());
				oPlayerJoinedServerPacket.pack("c", (u_char)pConnection->GetPlayer()->GetName().length());
				oPlayerJoinedServerPacket.pack("t", &pConnection->GetPlayer()->GetName());
				oPlayerJoinedServerPacket.CompleteTpcPacketSize();
				ClientConnection::BroadcastTcpExcept(oPlayerJoinedServerPacket, pConnection, PUBLIC_CLIENT);

				// Send a Enter Game Permission packet
				CPacket oEnterGamePermissionPacket;
				oEnterGamePermissionPacket.pack("hc", 0, (u_char)6);
				oEnterGamePermissionPacket.CompleteTpcPacketSize();
				pConnection->SendTcp(oEnterGamePermissionPacket, PUBLIC_CLIENT);
			}
		}
		break;
	default:
		printf("Error: Got unknown TCP packet of type %d and data size %d.\n", cPacketType, nDataSize);
		return false;
		break;
	}

	return true;
}

// Process a received UDP packet
bool NetworkProcessUdpPacket(CPacket & oPacket, ClientConnection * pConnection)
{
	u_int nDataSize = oPacket.size() - 1;
	u_char cPacketType; oPacket.unpack("c", &cPacketType);

	switch (cPacketType) {
	// UDP Handshake
	case 100:
		// Check if this client has already established a UDP connection
		if (pConnection->GetJoinStatus() < UDP_CONNECTED) {
			printf("Error: NetworkProcessUdpPacket() received a UDP Handshake packet (how did this happen!?), ignoring.\n");
			return false;
		}

		printf("Warning: This client has already connected on UDP, but received another (duplicate or slow?) UDP Handshake packet.\n");
		break;
	// Time Request Packet
	case 105:
		// Check if this client has already established a UDP connection
		if (pConnection->GetJoinStatus() < UDP_CONNECTED) {
			printf("Error: This client hasn't yet connected on UDP, ignoring UDP packet.\n");
			return false;
		}

		if (nDataSize != 1) return false;		// Check packet size
		else {
			u_char cSequenceNumber;
			oPacket.unpack("c", &cSequenceNumber);

			CPacket oTimeResponsePacket;
			oTimeResponsePacket.pack("ccd", (u_char)106, cSequenceNumber, glfwGetTime());
			pConnection->SendUdp(oTimeResponsePacket, UDP_CONNECTED);
		}
		break;
	// Pong Packet
	case 11:
		// Check if this client has NOT entered game
		if (pConnection->GetJoinStatus() < IN_GAME) {
			printf("Error: This client hasn't yet entered game.\n");
			return false;
		}

		if (nDataSize != 4) return false;		// Check packet size
		else {
			PingData_t oPingData;
			oPacket.unpack("cccc", &oPingData.cPingData[0], &oPingData.cPingData[1], &oPingData.cPingData[2], &oPingData.cPingData[3]);

			// Reply with a Pung packet immediately
			CPacket oPungPacket;
			oPungPacket.pack("cccccd", (u_char)12, oPingData.cPingData[0], oPingData.cPingData[1], oPingData.cPingData[2], oPingData.cPingData[3], glfwGetTime());
			pConnection->SendUdp(oPungPacket);

			// TODO: Mutex
			try {
				double dLatency = glfwGetTime() - pConnection->GetPingSentTimes().MatchAndRemoveAfter(oPingData);
				pConnection->SetLastLatency(static_cast<u_short>(ceil(dLatency * 10000)));
				//printf("#%d RTT = %.4lf ms -> %d\n", pConnection->GetPlayerID(), dLatency * 1000, pConnection->GetLastLatency());
			} catch (int) {
				// TODO: Investigate why I get this error a few times in a 5-min LAN session (perhaps duplicate packet? - yep, most likely, there were many duplicate command packets)
				printf("DEBUG: Couldn't find a ping sent time match.\n");
			}
			/*if (pConnection->GetPingSentTimes().MatchAndRemoveAfter(oPingData)) {
				double dLatency = glfwGetTime() - pConnection->GetPingSentTimes().GetLastMatchedValue();
				pConnection->SetLastLatency(static_cast<u_short>(ceil(dLatency * 10000)));
				//printf("#%d RTT = %.4lf ms -> %d\n", pConnection->GetPlayerID(), dLatency * 1000, pConnection->GetLastLatency());
			}*/
		}
		break;
	// Client Command Packet
	case 1:
		// Check if this client has NOT entered game
		if (pConnection->GetJoinStatus() < IN_GAME) {
			printf("Error: This client hasn't yet entered game, ignoring UDP packet.\n");
			return false;
		}

		if (nDataSize < 9) return false;		// Check packet size
		else {
glfwLockMutex(oPlayerTick);
//double t1 = glfwGetTime();

			static_cast<NetworkController *>(pConnection->GetPlayer()->m_pController)->ProcessCommand(oPacket);

//static u_int i = 0; if (i++ % 50 == 0) printf("processed Client Command packet in %.5lf ms\n", (glfwGetTime() - t1) * 1000);
glfwUnlockMutex(oPlayerTick);
		}
		break;
	default:
		printf("Error: Got unknown UDP packet of type %d and data size %d.\n", cPacketType, nDataSize);
		return false;
		break;
	}

	return true;
}

// Process a potential UDP Handshake packet
bool NetworkProcessUdpHandshakePacket(u_char * cPacket, u_short nPacketSize, sockaddr_in & oSenderAddress, SOCKET nUdpSocket)
{
	// Check if the given packet is a UDP Handshake packet
	if (nPacketSize == 1 + NetworkConnection::m_knSignatureSize && cPacket[0] == (u_char)100)
	{
		ClientConnection * pConnection = ClientConnection::GetFromSignature(&cPacket[1]);

		if (pConnection == NULL)
		{
			printf("Warning: Invalid UDP signature, no matching client.\n");
			return true;
		}
		else
		{
			pConnection->SetUdpAddress(oSenderAddress);
			pConnection->SetUdpSocket(nUdpSocket);
			pConnection->SetJoinStatus(UDP_CONNECTED);
			printf("Established UDP connection with player %d (%s:%d on socket %d).\n", pConnection->GetPlayerID(),
				inet_ntoa(oSenderAddress.sin_addr), ntohs(oSenderAddress.sin_port), pConnection->GetTcpSocket());

			// Send a UDP Connection Established packet
			CPacket oUDPConnectionEstablishedPacket;
			oUDPConnectionEstablishedPacket.pack("hc", 0, (u_char)5);
			oUDPConnectionEstablishedPacket.CompleteTpcPacketSize();
			pConnection->SendTcp(oUDPConnectionEstablishedPacket, UDP_CONNECTED);

			return true;
		}
	}
	else
		return false;
}

#ifndef EX0_CLIENT
// Shutdown the networking component
void NetworkDeinit()
{
#ifdef WIN32
	WSACleanup();
#else // Linux
	// Nothing to be done on Linux
#endif

	glfwDestroyMutex(oTcpSendMutex); oTcpSendMutex = NULL;
	glfwDestroyMutex(oUdpSendMutex); oUdpSendMutex = NULL;
	glfwDestroyMutex(oPlayerTick); oPlayerTick = NULL;
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
#endif // EX0_CLIENT
