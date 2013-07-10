#include "globals.h"

// Initialize the networking component
bool NetworkInit(void)
{
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

int sendall(SOCKET s, char *buf, int len, int flags)
{
	int total = 0;        // how many bytes we've sent
	int bytesleft = len; // how many we have left to send
	int n = 0;

	while(total < len) {
		n = send(s, buf+total, bytesleft, flags);
		if (n == SOCKET_ERROR) { break; }
		total += n;
		bytesleft -= n;
	}

	//*len = total; // return number actually sent here

	return (n == SOCKET_ERROR || total != len) ? SOCKET_ERROR : total; // return SOCKET_ERROR on failure, bytes sent on success
}

// Process a received TCP packet
bool NetworkProcessTcpPacket(CPacket & oPacket, CClient * pClient)
{
	u_short snPacketSize; oPacket.unpack("h", &snPacketSize);
	if (snPacketSize < 4) return false;
	u_short snPacketType; oPacket.unpack("h", &snPacketType);

	switch (snPacketType) {
	// Join Game Request
	case 1:
		// Check if this client has already joined
		if (pClient->GetJoinStatus() >= ACCEPTED) {
			printf("Error: This client has already joined, but sent another Join Game Request packet.\n");
			return false;
		}

		if (snPacketSize != 26) return false;		// Check packet size
		else {
			short snVersion;
			char szPassphrase[17];
			char cSignature[4];
			oPacket.unpack("h16s", &snVersion, szPassphrase);
			for (int nSignatureByte = 0; nSignatureByte < 4; ++nSignatureByte)
				oPacket.unpack("c", &cSignature[nSignatureByte]);

			if (snVersion == 1 && memcmp(szPassphrase, "somerandompass01", 16) == 0)
			{
				// Valid join game request packet
				printf("Got a valid TcpPacketJoinGameRequest_t packet! yay\n");
			} else {
				// Drop the connection to this unsupported client
				printf("Error: Got an INVALID TcpPacketJoinGameRequest_t packet, dropping connection.\n");
				delete pClient;
			}

			// Get a free player slot, if one exists, for this new client
			int nPlayerID = PlayerGetFreePlayerID();
			if (nPlayerID != -1)
			{
				// Assign this client the avaliable player id
				pClient->SetSignature(cSignature);
				pClient->SetPlayerID(nPlayerID);
				pClient->SetJoinStatus(ACCEPTED);

				// DEBUG: Randomly position the player
				float x, y;
				do {
					x = static_cast<float>(rand() % 2000 - 1000);
					y = static_cast<float>(rand() % 2000 - 1000);
				} while (ColHandIsPointInside((int)x, (int)y) || !ColHandCheckPlayerPos(&x, &y));
				PlayerGet(nPlayerID)->Position(x, y);
				PlayerGet(nPlayerID)->SetZ(0.001f * (rand() % 1000) * Math::TWO_PI);

				// Accept the connection, send a Join Game Accept packet
				CPacket oAcceptPacket;
				oAcceptPacket.pack("hhcc", 0, (u_short)2, (unsigned char)nPlayerID, (unsigned char)nPlayerCount);
				oAcceptPacket.CompleteTpcPacketSize();
				oAcceptPacket.SendTcp(pClient, TCP_CONNECTED);
			} else {
				// Refuse join game request
				CPacket oRefusePacket;
				oRefusePacket.pack("hhc", 0, (u_short)3, 1);
				oRefusePacket.CompleteTpcPacketSize();
				oRefusePacket.SendTcp(pClient, TCP_CONNECTED);
			}
		}
		break;
	// Client Entered Game
	case 7:
		// Check if this client has NOT joined
		if (pClient->GetJoinStatus() < UDP_CONNECTED) {
			printf("Error: This client hasn't yet joined, ignoring TCP packet.\n");
			return false;
		} else if (pClient->GetJoinStatus() >= IN_GAME) {
			printf("Error: This client has already entered game, but sent another Client Entered Game packet.\n");
			return false;
		}

		if (snPacketSize != 4) return false;		// Check packet size
		else {
			pClient->SetJoinStatus(IN_GAME);
			printf("Player #%d (name '%s') has entered the game.\n", pClient->GetPlayerID(), pClient->GetPlayer()->GetName().c_str());

			// Send a Player Entered Game to all the other clients
			CPacket oPlayerEnteredGamePacket;
			oPlayerEnteredGamePacket.pack("hh", 0, (u_short)25);
			oPlayerEnteredGamePacket.pack("c", (u_char)pClient->GetPlayerID());
			oPlayerEnteredGamePacket.pack("c", (u_char)pClient->GetPlayer()->GetName().length());
			oPlayerEnteredGamePacket.pack("t", &pClient->GetPlayer()->GetName());
			oPlayerEnteredGamePacket.pack("fff", pClient->GetPlayer()->GetX(),
				pClient->GetPlayer()->GetY(), pClient->GetPlayer()->GetZ());
			oPlayerEnteredGamePacket.CompleteTpcPacketSize();
			oPlayerEnteredGamePacket.BroadcastTcpExcept(pClient, UDP_CONNECTED);
		}
		break;
	// Send Text Message
	case 10:
		// Check if this client has NOT entered game
		if (pClient->GetJoinStatus() < IN_GAME) {
			printf("Error: This client hasn't yet entered game, ignoring TCP packet.\n");
			return false;
		}

		if (snPacketSize < 5) return false;		// Check packet size
		else {
			string sTextMessage;
			oPacket.unpack("et", snPacketSize - 4, &sTextMessage);

			// Valid Send Text Message packet
			printf("Got TcpPacketSendTextMessage_t from player id %d, msg '%s'\n",
				pClient->GetPlayerID(), sTextMessage.c_str());

			// Create a Broadcast Text Message packet
			CPacket oBroadcastMessagePacket;
			oBroadcastMessagePacket.pack("hhct", 0, (u_short)11, (unsigned char)pClient->GetPlayerID(), &sTextMessage);
			oBroadcastMessagePacket.CompleteTpcPacketSize();

			// Broadcast the text message to all players that are connected
			oBroadcastMessagePacket.BroadcastTcp();
		}
		break;
	// Local Player Info
	case 30:
		// Check if this client has NOT joined
		if (pClient->GetJoinStatus() < UDP_CONNECTED) {
			printf("Error: This client hasn't yet joined, ignoring TCP packet.\n");
			return false;
		}

		if (snPacketSize < 6) return false;		// Check packet size
		else {
			u_char cNameLength;
			string sName;
			oPacket.unpack("c", &cNameLength); cNameLength = __min(cNameLength, snPacketSize - 5);
			oPacket.unpack("et", (int)cNameLength, &sName);
			PlayerGet(pClient->GetPlayerID())->SetName(sName);

			// Send a Load Level packet to load the current level
			CPacket oLoadLevelPacket;
			oLoadLevelPacket.pack("hht", 0, (u_short)20, &sLevelName);
			oLoadLevelPacket.CompleteTpcPacketSize();
			oLoadLevelPacket.SendTcp(pClient, UDP_CONNECTED);

			// Send a Current Players Info to the new client
			CPacket oCurrentPlayersInfoPacket;
			oCurrentPlayersInfoPacket.pack("hh", 0, (u_short)21);
			for (int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
			{
				// Include the client who's connecting and all IN_GAME clients
				if (nPlayer == pClient->GetPlayerID() || (PlayerGet(nPlayer)->pClient != NULL
				  && PlayerGet(nPlayer)->pClient->GetJoinStatus() == IN_GAME)) {
					oCurrentPlayersInfoPacket.pack("c", (u_char)PlayerGet(nPlayer)->GetName().length());
					oCurrentPlayersInfoPacket.pack("t", &PlayerGet(nPlayer)->GetName());
					oCurrentPlayersInfoPacket.pack("fff", PlayerGet(nPlayer)->GetX(),
						PlayerGet(nPlayer)->GetY(), PlayerGet(nPlayer)->GetZ());
				} else {
					oCurrentPlayersInfoPacket.pack("c", (char)0);
				}
			}
			oCurrentPlayersInfoPacket.CompleteTpcPacketSize();
			oCurrentPlayersInfoPacket.SendTcp(pClient, UDP_CONNECTED);

			// Send a Permission To Join packet
			CPacket oPermissionToJoinPacket;
			oPermissionToJoinPacket.pack("hh", 0, (u_short)6);
			oPermissionToJoinPacket.CompleteTpcPacketSize();
			oPermissionToJoinPacket.SendTcp(pClient, UDP_CONNECTED);
		}
		break;
	default:
		printf("Error: Got unknown TCP packet of type %d and size %d.\n", snPacketType, snPacketSize);
		return false;
		break;
	}

	return true;
}

// Process a received UDP packet
bool NetworkProcessUdpPacket(CPacket & oPacket, int nPacketSize, CClient * pClient)
{
	if (nPacketSize < 1) return false;
	u_char cPacketType; oPacket.unpack("c", &cPacketType);

	switch (cPacketType) {
	// UDP Handshake
	case 100:
		// Check if this client has already established a UDP connection
		if (pClient->GetJoinStatus() >= UDP_CONNECTED) {
			printf("Warning: This client has already connected on UDP (duplicate packet?), ignoring UDP packet.\n");
			return true;
		} else {
			printf("Error: NetworkProcessUdpPacket() received a UDP Handshake packet, ignoring UDP packet.\n");
			return false;
		}
		break;
	// Update Own Position test packet
	case 1:
		// Check if this client has NOT entered game
		if (pClient->GetJoinStatus() < IN_GAME) {
			printf("Error: This client hasn't yet entered game, ignoring UDP packet.\n");
			return false;
		}

		if (nPacketSize != 7) return false;		// Check packet size
		else {
			u_char cSequenceNumber;
			char cMoveDirection;
			float fZ;
			oPacket.unpack("ccf", &cSequenceNumber, &cMoveDirection, &fZ);

			if (pClient->cLastMovementSequenceNumber == cSequenceNumber) {
				printf("Got a duplicate UDP update packet from a client, discarding.\n");
			} else
			{
				++pClient->cLastMovementSequenceNumber;
				if (pClient->cLastMovementSequenceNumber != cSequenceNumber) {
					printf("Lost %d UDP update packet(s) from a client!\n", (u_char)(cSequenceNumber - pClient->cLastMovementSequenceNumber));
				}
				pClient->cLastMovementSequenceNumber = cSequenceNumber;

				//PlayerGet(pClient->GetPlayerID())->Position(fX, fY);
				//PlayerGet(pClient->GetPlayerID())->SetZ(fZ);
				pClient->GetPlayer()->MoveDirection(cMoveDirection);
				pClient->GetPlayer()->SetZ(fZ);

				// Player tick
				pClient->GetPlayer()->CalcTrajs();
				pClient->GetPlayer()->CalcColResp();

				// Send the Update Others Position packet
				CPacket oUpdateOthersPositionPacket;
				oUpdateOthersPositionPacket.pack("cc", (u_char)2, cSequenceNumber);
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
					oUpdateOthersPositionPacket.SendUdp(pClient);
			}
		}
		break;
	default:
		printf("Error: Got unknown UDP packet of type %d and size %d.\n", (int)cPacketType, nPacketSize);
		return false;
		break;
	}

	return true;
}

// Process a potential UDP Handshake packet
bool NetworkProcessUdpHandshakePacket(char * oPacket, int nPacketSize, struct sockaddr_in & oSenderAddress)
{
	// Check if the given packet is a UDP Handshake packet
	if (nPacketSize == 1 + 4 && oPacket[0] == (u_char)100)
	{
		CClient * pClient = ClientGetFromSignature(&oPacket[1]);

		if (pClient == NULL)
		{
			printf("Error: Invalid UDP signature, no matching client.\n");
			return false;
		}
		else
		{
			pClient->SetAddress(oSenderAddress);
			pClient->SetJoinStatus(UDP_CONNECTED);
			printf("Established UDP connection with player %d (%s:%d on socket %d).\n", pClient->GetPlayerID(),
				inet_ntoa(oSenderAddress.sin_addr), ntohs(oSenderAddress.sin_port), pClient->GetSocket());

			// Send a UDP Connection Established packet
			CPacket oUDPConnectionEstablishedPacket;
			oUDPConnectionEstablishedPacket.pack("hh", 0, (u_short)5);
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
