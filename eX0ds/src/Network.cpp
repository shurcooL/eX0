#include "globals.h"

GLFWmutex		oTcpSendMutex;
GLFWmutex		oUdpSendMutex;
GLFWmutex		oPlayerTick;

// Initialize the networking component
bool NetworkInit(void)
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

	int nResult = sendto(s, buf, len, flags, to, tolen);

	glfwUnlockMutex(oUdpSendMutex);

	return nResult;
}
int sendudp(SOCKET s, const char *buf, int len, int flags)
{
	glfwLockMutex(oUdpSendMutex);

	int nResult = send(s, buf, len, flags);

	glfwUnlockMutex(oUdpSendMutex);

	return nResult;
}

void NetworkSendServerUpdate(void *p)
{
glfwLockMutex(oPlayerTick);

	CClient *pClient = reinterpret_cast<CClient *>(p);

	++pClient->cCurrentUpdateSequenceNumber;		// First update is sent with cCurrentUpdateSequenceNumber == 1

	// Send the Update Others Position packet
	CPacket oServerUpdatePacket;
	oServerUpdatePacket.pack("cc", (u_char)2, pClient->cCurrentUpdateSequenceNumber);
	for (int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
	{
		if (PlayerGet(nPlayer)->pClient != NULL && PlayerGet(nPlayer)->pClient->GetJoinStatus() == IN_GAME
			&& PlayerGet(nPlayer)->GetTeam() != 2)
		{
			oServerUpdatePacket.pack("c", (u_char)1);

			oServerUpdatePacket.pack("c", PlayerGet(nPlayer)->pClient->cLastCommandSequenceNumber);
			oServerUpdatePacket.pack("fff", PlayerGet(nPlayer)->GetX(),
				PlayerGet(nPlayer)->GetY(), PlayerGet(nPlayer)->GetZ());
		} else {
			oServerUpdatePacket.pack("c", (u_char)0);
		}
	}
	if ((rand() % 100) >= 0 || pClient->GetPlayerID() != 0) // DEBUG: Simulate packet loss
		oServerUpdatePacket.SendUdp(pClient);

glfwUnlockMutex(oPlayerTick);
}

// Process a received TCP packet
bool NetworkProcessTcpPacket(CPacket & oPacket, CClient *& pClient)
{
	u_short nDataSize; oPacket.unpack("h", &nDataSize);
	u_char cPacketType; oPacket.unpack("c", &cPacketType);

	switch (cPacketType) {
	// Join Server Request
	case 1:
		// Check if this client has already joined
		if (pClient->GetJoinStatus() >= ACCEPTED) {
			printf("Error: This client has already joined, but sent another Join Server Request packet.\n");
			return false;
		}

		if (nDataSize != (18 + SIGNATURE_SIZE)) return false;		// Check packet size
		else {
			short snVersion;
			char szPassphrase[17];
			u_char cSignature[SIGNATURE_SIZE];
			oPacket.unpack("h16s", &snVersion, szPassphrase);
			for (int nSignatureByte = 0; nSignatureByte < SIGNATURE_SIZE; ++nSignatureByte)
				oPacket.unpack("c", &cSignature[nSignatureByte]);

			if (snVersion == 1 && memcmp(szPassphrase, "somerandompass01", 16) == 0)
			{
				// Valid Join Server Request packet
				printf("Got a valid Join Server Request packet! yay.. yay... ;/\n");
			} else {
				// Drop the connection to this unsupported client
				printf("Error: Got an INVALID Join Server Request packet, dropping connection.\n");
				delete pClient; pClient = NULL;
				return true;
			}

			// Get a free player slot, if one exists, for this new client
			int nPlayerID = PlayerGetFreePlayerID();
			if (nPlayerID != -1)
			{
				// Assign this client the avaliable player id
				pClient->SetSignature(cSignature);
				pClient->SetPlayerID(nPlayerID);
				pClient->SetJoinStatus(ACCEPTED);

				// Accept the connection, send a Join Server Accept packet
				CPacket oJoinServerAcceptPacket;
				oJoinServerAcceptPacket.pack("hccc", 0, (u_char)2, (unsigned char)nPlayerID, (unsigned char)nPlayerCount);
				oJoinServerAcceptPacket.CompleteTpcPacketSize();
				oJoinServerAcceptPacket.SendTcp(pClient, TCP_CONNECTED);
			} else {
				// Refuse join server request with reason 1
				CPacket oJoinServerRefusePacket;
				oJoinServerRefusePacket.pack("hcc", 0, (u_char)3, 1);
				oJoinServerRefusePacket.CompleteTpcPacketSize();
				oJoinServerRefusePacket.SendTcp(pClient, TCP_CONNECTED);

				// Disconnect this client
				delete pClient; pClient = NULL;
			}
		}
		break;
	// Entered Game Notification
	case 7:
		// Check if this client has already established a UDP connection
		if (pClient->GetJoinStatus() < PUBLIC_CLIENT) {
			printf("Error: This client hasn't yet joined, ignoring TCP packet.\n");
			return false;
		} else if (pClient->GetJoinStatus() >= IN_GAME) {
			printf("Error: This client has already entered game, but sent another Entered Game Notification packet.\n");
			return false;
		}

		if (nDataSize != 0) return false;		// Check packet size
		else {
			pClient->SetJoinStatus(IN_GAME);
			printf("Player #%d (name '%s') has entered the game.\n", pClient->GetPlayerID(), pClient->GetPlayer()->GetName().c_str());

			// Schedule the sending of Server Update packets for this client
			CTimedEvent oEvent(glfwGetTime(), 1.0 / pClient->cUpdateRate, &NetworkSendServerUpdate, (void *)pClient);
			pTimedEventScheduler->ScheduleEvent(oEvent);
			pClient->nUpdateEventId = oEvent.GetId();
		}
		break;
	// Send Text Message
	case 10:
		// Check if this client has NOT entered game
		if (pClient->GetJoinStatus() < IN_GAME) {
			printf("Error: This client hasn't yet entered game, ignoring TCP packet.\n");
			return false;
		}

		if (nDataSize < 1) return false;		// Check packet size
		else {
			string sTextMessage;
			oPacket.unpack("et", nDataSize, &sTextMessage);

			// Valid Send Text Message packet
			printf("Player %d sends msg '%s'.\n", pClient->GetPlayerID(), sTextMessage.c_str());

			// Create a Broadcast Text Message packet
			CPacket oBroadcastMessagePacket;
			oBroadcastMessagePacket.pack("hcct", 0, (u_char)11, (u_char)pClient->GetPlayerID(), &sTextMessage);
			oBroadcastMessagePacket.CompleteTpcPacketSize();

			// Broadcast the text message to all players that are IN_GAME
			oBroadcastMessagePacket.BroadcastTcp();
		}
		break;
	// Join Team Request
	case 27:
		// Check if this client has NOT entered game
		if (pClient->GetJoinStatus() < IN_GAME) {
			printf("Error: This client hasn't yet entered game.\n");
			return false;
		}

		if (nDataSize != 1) return false;		// Check packet size
		else {
			u_char cTeam;
			oPacket.unpack("c", &cTeam);

glfwLockMutex(oPlayerTick);
			pClient->GetPlayer()->SetTeam((int)cTeam);
			printf("Player #%d (name '%s') joined team %d.\n", pClient->GetPlayerID(), pClient->GetPlayer()->GetName().c_str(), cTeam);

			// Create a Player Joined Team packet
			CPacket oPlayerJoinedTeam;
			oPlayerJoinedTeam.pack("hcc", 0, (u_char)28, pClient->GetPlayerID());
			oPlayerJoinedTeam.pack("c", (u_char)pClient->GetPlayer()->GetTeam());

			if (pClient->GetPlayer()->GetTeam() != 2)
			{
				// DEBUG: Randomly position the player
				float x, y;
				do {
					x = static_cast<float>(rand() % 2000 - 1000);
					y = static_cast<float>(rand() % 2000 - 1000);
				} while (ColHandIsPointInside((int)x, (int)y) || !ColHandCheckPlayerPos(&x, &y));
				pClient->GetPlayer()->Position(x, y, 0.001f * (rand() % 1000) * Math::TWO_PI);
				printf("Positioning player %d at %f, %f.\n", pClient->GetPlayerID(), x, y);

				// DEBUG: Perform the cLastCommandSequenceNumber synchronization to time
				double d = glfwGetTime() / (256.0 / pClient->cCommandRate);
				d -= floor(d);
				d *= 256.0;
				pClient->cLastCommandSequenceNumber = (u_char)d;

				// Start expecting the first command packet from this player
				pClient->cCurrentCommandSeriesNumber += 1;
				pClient->bFirstCommand = true;

				oPlayerJoinedTeam.pack("c", pClient->cLastCommandSequenceNumber);
				oPlayerJoinedTeam.pack("fff", pClient->GetPlayer()->GetX(),
					pClient->GetPlayer()->GetY(), pClient->GetPlayer()->GetZ());
//glfwSleep(0.055);
			}
glfwUnlockMutex(oPlayerTick);

			oPlayerJoinedTeam.CompleteTpcPacketSize();
			oPlayerJoinedTeam.BroadcastTcp(PUBLIC_CLIENT);
//glfwSleep(0.055);
//glfwUnlockMutex(oPlayerTick);
		}
		break;
	// Local Player Info
	case 30:
		// Check if this client has already established a UDP connection
		if (pClient->GetJoinStatus() < UDP_CONNECTED) {
			printf("Error: This client hasn't yet joined, ignoring TCP packet.\n");
			return false;
		} else if (pClient->GetJoinStatus() >= PUBLIC_CLIENT) {
			return false;
		}

		if (nDataSize < 4) return false;		// Check packet size
		else {
			u_char cNameLength;
			string sName;
			oPacket.unpack("c", &cNameLength); if ((u_short)cNameLength > nDataSize - 3) cNameLength = (nDataSize - 3);
			oPacket.unpack("et", (int)cNameLength, &sName);
			PlayerGet(pClient->GetPlayerID())->SetName(sName);
			oPacket.unpack("cc", &pClient->cCommandRate, &pClient->cUpdateRate);

			if (pClient->cCommandRate < 1 || pClient->cCommandRate > 100 ||
				pClient->cUpdateRate < 1 || pClient->cUpdateRate > 100)
			{
				printf("Client rates (%d, %d) for player %d are out of range, sending refuse packet.\n",
					pClient->cCommandRate, pClient->cUpdateRate, pClient->GetPlayerID());

				// Refuse with reason 2
				CPacket oJoinServerRefusePacket;
				oJoinServerRefusePacket.pack("hcc", 0, (u_char)3, 2);
				oJoinServerRefusePacket.CompleteTpcPacketSize();
				oJoinServerRefusePacket.SendTcp(pClient, TCP_CONNECTED);
			}
			else
			{
				// The client now has the Public join status
				pClient->SetJoinStatus(PUBLIC_CLIENT);

				// Set team to Spectator by default
				pClient->GetPlayer()->SetTeam(2);

				// Set the player tick time
				// TODO: Do something about this, see if it's needed, and remove it if not, or make it useful, etc.
				pClient->GetPlayer()->fTickTime = 1.0f / pClient->cCommandRate;

				// Send a Load Level packet to load the current level
				CPacket oLoadLevelPacket;
				oLoadLevelPacket.pack("hct", 0, (u_char)20, &sLevelName);
				oLoadLevelPacket.CompleteTpcPacketSize();
				oLoadLevelPacket.SendTcp(pClient, PUBLIC_CLIENT);

				// Send a Current Players Info to the new client
				CPacket oCurrentPlayersInfoPacket;
				oCurrentPlayersInfoPacket.pack("hc", 0, (u_char)21);
				for (int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
				{
					// Include the client who's connecting and all clients with at least Public status
					if ((PlayerGet(nPlayer)->pClient != NULL && PlayerGet(nPlayer)->pClient->GetJoinStatus() >= PUBLIC_CLIENT)
						|| nPlayer == pClient->GetPlayerID())
					{
						oCurrentPlayersInfoPacket.pack("c", (u_char)PlayerGet(nPlayer)->GetName().length());
						oCurrentPlayersInfoPacket.pack("t", &PlayerGet(nPlayer)->GetName());
						oCurrentPlayersInfoPacket.pack("c", (u_char)PlayerGet(nPlayer)->GetTeam());
						if (PlayerGet(nPlayer)->GetTeam() != 2)
						{
							oCurrentPlayersInfoPacket.pack("c", PlayerGet(nPlayer)->pClient->cLastCommandSequenceNumber);
							oCurrentPlayersInfoPacket.pack("fff", PlayerGet(nPlayer)->GetX(),
								PlayerGet(nPlayer)->GetY(), PlayerGet(nPlayer)->GetZ());
						}
					} else {
						oCurrentPlayersInfoPacket.pack("c", (char)0);
					}
				}
				oCurrentPlayersInfoPacket.CompleteTpcPacketSize();
				oCurrentPlayersInfoPacket.SendTcp(pClient, PUBLIC_CLIENT);

				// Send a Player Joined Server packet to all the other clients with Public status
				CPacket oPlayerJoinedServerPacket;
				oPlayerJoinedServerPacket.pack("hc", 0, (u_char)25);
				oPlayerJoinedServerPacket.pack("c", (u_char)pClient->GetPlayerID());
				oPlayerJoinedServerPacket.pack("c", (u_char)pClient->GetPlayer()->GetName().length());
				oPlayerJoinedServerPacket.pack("t", &pClient->GetPlayer()->GetName());
				//oPlayerJoinedServerPacket.pack("c", pClient->cLastCommandSequenceNumber);
				//oPlayerJoinedServerPacket.pack("fff", pClient->GetPlayer()->GetX(),
				//	pClient->GetPlayer()->GetY(), pClient->GetPlayer()->GetZ());
				oPlayerJoinedServerPacket.CompleteTpcPacketSize();
				oPlayerJoinedServerPacket.BroadcastTcpExcept(pClient, PUBLIC_CLIENT);

				// Send a Enter Game Permission packet
				CPacket oEnterGamePermissionPacket;
				oEnterGamePermissionPacket.pack("hc", 0, (u_char)6);
				oEnterGamePermissionPacket.CompleteTpcPacketSize();
				oEnterGamePermissionPacket.SendTcp(pClient, PUBLIC_CLIENT);
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
bool NetworkProcessUdpPacket(CPacket & oPacket, CClient * pClient)
{
	u_int nDataSize = oPacket.size() - 1;
	u_char cPacketType; oPacket.unpack("c", &cPacketType);

	switch (cPacketType) {
	// UDP Handshake
	case 100:
		// Check if this client has already established a UDP connection
		if (pClient->GetJoinStatus() >= UDP_CONNECTED) {
			printf("Warning: This client has already connected on UDP, but received another (duplicate or slow?) UDP Handshake packet.\n");
			return true;
		} else {
			printf("Error: NetworkProcessUdpPacket() received a UDP Handshake packet, ignoring UDP packet.\n");
			return false;
		}
		break;
	// Time Request Packet
	case 105:
		// Check if this client has already established a UDP connection
		if (pClient->GetJoinStatus() < UDP_CONNECTED) {
			printf("Error: This client hasn't yet connected on UDP, ignoring UDP packet.\n");
			return false;
		}

		if (nDataSize != 1) return false;		// Check packet size
		else {
			u_char cSequenceNumber;
			oPacket.unpack("c", &cSequenceNumber);

			CPacket oTimeResponsePacket;
			oTimeResponsePacket.pack("ccd", (u_char)106, cSequenceNumber, glfwGetTime());
			oTimeResponsePacket.SendUdp(pClient, UDP_CONNECTED);
		}
		break;
	// Pong Packet
	case 11:
		// Check if this client has NOT entered game
		if (pClient->GetJoinStatus() < IN_GAME) {
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
			oPungPacket.SendUdp(pClient);

			// TODO: Mutex
			if (pClient->GetPingSentTimes().MatchAndRemoveAfter(oPingData)) {
				double dLatency = glfwGetTime() - pClient->GetPingSentTimes().GetLastMatchedValue();
				pClient->SetLastLatency(static_cast<u_short>(ceil(dLatency * 10000)));
				//printf("#%d RTT = %.4lf ms -> %d\n", pClient->GetPlayerID(), dLatency * 1000, pClient->GetLastLatency());
			}
		}
		break;
	// Client Command Packet
	case 1:
		// Check if this client has NOT entered game
		if (pClient->GetJoinStatus() < IN_GAME) {
			printf("Error: This client hasn't yet entered game, ignoring UDP packet.\n");
			return false;
		}

		if (nDataSize < 8) return false;		// Check packet size
		else {
glfwLockMutex(oPlayerTick);

			u_char cCommandSequenceNumber;
			u_char cCommandSeriesNumber;
			u_char cMovesCount;
			char cMoveDirection;
			float fZ;
			oPacket.unpack("ccc", &cCommandSequenceNumber, &cCommandSeriesNumber, &cMovesCount);
			cMovesCount += 1;

			if (cCommandSeriesNumber != pClient->cCurrentCommandSeriesNumber) {
				printf("Got a Command with mismatching series number, cCommandSequenceNumber = %d, cMovesCount = %d, ignoring.\n", cCommandSequenceNumber, cMovesCount);
				break;
			}

			// A special case for the first command we receive from this client in this new series
			if (pClient->bFirstCommand) {
				pClient->cLastCommandSequenceNumber = (u_char)(cCommandSequenceNumber - cMovesCount);
printf("Got first command from client %d, last %d, with %d moves\n", cCommandSequenceNumber, pClient->cLastCommandSequenceNumber, cMovesCount);
				pClient->bFirstCommand = false;
			}

			if (cCommandSequenceNumber == pClient->cLastCommandSequenceNumber) {
				printf("Got a duplicate UDP command packet from player %d, discarding.\n", pClient->GetPlayerID());
			} else if ((char)(cCommandSequenceNumber - pClient->cLastCommandSequenceNumber) < 0) {
				printf("Got an out of order UDP command packet from player %d, discarding.\n", pClient->GetPlayerID());
			} else
			{
				int nMove = (int)cMovesCount - (char)(cCommandSequenceNumber - pClient->cLastCommandSequenceNumber);
				if (nMove < 0) printf("!!MISSING!! %d command move(s) from player #%d, due to lost packets:\n", -nMove, pClient->GetPlayerID());
				if (nMove < 0) nMove = 0;

				++pClient->cLastCommandSequenceNumber;
				if (cCommandSequenceNumber != pClient->cLastCommandSequenceNumber) {
					printf("Lost %d UDP command packet(s) from player #%d!\n", (u_char)(cCommandSequenceNumber - pClient->cLastCommandSequenceNumber), pClient->GetPlayerID());
				}
				pClient->cLastCommandSequenceNumber = cCommandSequenceNumber;

				for (int nSkip = 0; nSkip < nMove; ++nSkip)
					oPacket.unpack("cf", &cMoveDirection, &fZ);
				for (; nMove < (int)cMovesCount; ++nMove)
				{
					oPacket.unpack("cf", &cMoveDirection, &fZ);
					//printf("execing command %d\n", cCommandSequenceNumber - (cMovesCount - 1) + nMove);

					// Set the inputs
					pClient->GetPlayer()->MoveDirection(cMoveDirection);
					pClient->GetPlayer()->SetZ(fZ);

					// Player tick
					pClient->GetPlayer()->CalcTrajs();
					pClient->GetPlayer()->CalcColResp();
				}

				// Remember the time of the last update
				//pClient->GetPlayer()->fTicks = (float)glfwGetTime();

				// Send the Update Others Position packet
				/*CPacket oUpdateOthersPositionPacket;
				oUpdateOthersPositionPacket.pack("cc", (u_char)2, pClient->cLastCommandSequenceNumber);
				for (int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
				{
					if (PlayerGet(nPlayer)->pClient != NULL && PlayerGet(nPlayer)->pClient->GetJoinStatus() == IN_GAME) {
						oUpdateOthersPositionPacket.pack("c", (u_char)1);
						oUpdateOthersPositionPacket.pack("fff", PlayerGet(nPlayer)->GetX(),
							PlayerGet(nPlayer)->GetY(), PlayerGet(nPlayer)->GetZ());
					} else {
						oUpdateOthersPositionPacket.pack("c", (u_char)0);
					}
				}
				if ((rand() % 100) >= 0 || pClient->GetPlayerID() != 0) // DEBUG: Simulate packet loss
					oUpdateOthersPositionPacket.SendUdp(pClient);*/
			}
		}

glfwUnlockMutex(oPlayerTick);
		break;
	default:
		printf("Error: Got unknown UDP packet of type %d and data size %d.\n", cPacketType, nDataSize);
		return false;
		break;
	}

	return true;
}

// Process a potential UDP Handshake packet
bool NetworkProcessUdpHandshakePacket(u_char * cPacket, u_short nPacketSize, struct sockaddr_in & oSenderAddress)
{
	// Check if the given packet is a UDP Handshake packet
	if (nPacketSize == 1 + SIGNATURE_SIZE && cPacket[0] == (u_char)100)
	{
		CClient *pClient = ClientGetFromSignature(&cPacket[1]);

		if (pClient == NULL)
		{
			printf("Warning: Invalid UDP signature, no matching client.\n");
			return true;
		}
		else
		{
			pClient->SetAddress(oSenderAddress);
			pClient->SetJoinStatus(UDP_CONNECTED);
			printf("Established UDP connection with player %d (%s:%d on socket %d).\n", pClient->GetPlayerID(),
				inet_ntoa(oSenderAddress.sin_addr), ntohs(oSenderAddress.sin_port), pClient->GetSocket());

			// Send a UDP Connection Established packet
			CPacket oUDPConnectionEstablishedPacket;
			oUDPConnectionEstablishedPacket.pack("hc", 0, (u_char)5);
			oUDPConnectionEstablishedPacket.CompleteTpcPacketSize();
			oUDPConnectionEstablishedPacket.SendTcp(pClient, UDP_CONNECTED);

			return true;
		}
	}
	else
		return false;
}

// Shutdown the networking component
void NetworkDeinit()
{
#ifdef WIN32
	WSACleanup();
#else // Linux
	// Nothing to be done on Linux
#endif

	glfwDestroyMutex(oTcpSendMutex);
	glfwDestroyMutex(oUdpSendMutex);
	glfwDestroyMutex(oPlayerTick);
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
