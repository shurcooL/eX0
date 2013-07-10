#include "globals.h"

// Initialize the networking component
bool NetworkInit()
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
#endif
}

void NetworkPrintError(const char *szMessage)
{
#ifdef WIN32
	printf("%s: %d\n", szMessage, WSAGetLastError());
#else // Linux
	perror(szMessage);
#endif
}

int sendall(SOCKET s, char *buf, int len, int flags)
{
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

	return (n == -1 || total != len) ? -1 : total; // return -1 on failure, bytes sent on success
}

// Process a received packet
bool NetworkProcessPacket(struct TcpPacket_t * oPacket, SOCKET nSocket)
{
	short snPacketSize = ntohs(oPacket->snPacketSize);
	short snPacketType = ntohs(oPacket->snPacketType);

	switch (snPacketType) {
	// Join Game Request
	case 1:
		if (snPacketSize != sizeof(struct TcpPacketJoinGameRequest_t)) return false;		// Check packet size

		if (ntohs(((struct TcpPacketJoinGameRequest_t *)oPacket)->snVersion) == 1
			&& memcmp(((struct TcpPacketJoinGameRequest_t *)oPacket)->chPassphrase, "somerandompass01", 16) == 0)
		{
			// Valid join game request packet
			printf("Got a valid TcpPacketJoinGameRequest_t packet! yay\n");

			int nPlayerID = PlayerGetFreePlayerID();
			if (nPlayerID != -1)
			{
				// Accept the connection, send a Join Game Accept packet
				PlayerGet(nPlayerID)->bConnected = true;
				PlayerGet(nPlayerID)->nTcpSocket = nSocket;

				struct TcpPacketJoinGameAccept_t oAcceptPacket;
				oAcceptPacket.snPacketSize = htons(sizeof(oAcceptPacket));
				oAcceptPacket.snPacketType = htons(2);
				oAcceptPacket.chPlayerID = (char)nPlayerID;
				oAcceptPacket.chMaxPlayerCount = (char)nPlayerCount;

				if (sendall(nSocket, (char *)&oAcceptPacket, sizeof(oAcceptPacket), 0) == SOCKET_ERROR) {
					NetworkPrintError("sendall");
				}
			}
		}
		break;
	// Send Text Message
	case 10:
		if (snPacketSize <= 4) return false;		// Check packet size

		if (true)
		{
			string sTextMessage((char *)&(((struct TcpPacketSendTextMessage_t *)oPacket)->chTextMessage),
				snPacketSize - 4);

			// Valid Send Text Message packet
			printf("Got TcpPacketSendTextMessage_t from %d, msg '%s'\n",
				PlayerGetFromSocket(nSocket)->iID, sTextMessage.c_str());

			struct TcpPacketBroadcastTextMessage_t oBroadcastMessagePacket;
			oBroadcastMessagePacket.snPacketSize = htons((short)(5 + sTextMessage.length()));
			oBroadcastMessagePacket.snPacketType = htons((short)11);
			oBroadcastMessagePacket.chPlayerID = (char)PlayerGetFromSocket(nSocket)->iID;
			memcpy(&oBroadcastMessagePacket.chTextMessage, sTextMessage.c_str(), sTextMessage.length());
			printx((char *)&oBroadcastMessagePacket);

			for (int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
			{
				// Broadcast the text message to everyone who's connected
				if (PlayerGet(nPlayer)->bConnected) {
					printf("Sending msg from %d to %d\n", PlayerGetFromSocket(nSocket)->iID, nPlayer);
					if (sendall(PlayerGet(nPlayer)->nTcpSocket, (char *)&oBroadcastMessagePacket, 5 + sTextMessage.length(), 0) == SOCKET_ERROR) {
						NetworkPrintError("sendall");
					}
				}
			}
		}
		break;
	// Update Own Position (temporary debug packet)
	case 20:
		if (snPacketSize != sizeof(struct TcpPacketUpdateOwnPosition_t)) return false;		// Check packet size

		if (true)
		{
			float fX = ((struct TcpPacketUpdateOwnPosition_t *)oPacket)->fX;
			float fY = ((struct TcpPacketUpdateOwnPosition_t *)oPacket)->fY;
			float fZ = ((struct TcpPacketUpdateOwnPosition_t *)oPacket)->fZ;
			bool bFire = (bool)((struct TcpPacketUpdateOwnPosition_t *)oPacket)->chFire;

			struct TcpPacketUpdateOthersPosition_t oUpdateOthersPosPacket;
			oUpdateOthersPosPacket.snPacketSize = htons((short)(sizeof(oUpdateOthersPosPacket)));
			oUpdateOthersPosPacket.snPacketType = htons((short)21);
			oUpdateOthersPosPacket.chPlayerID = (char)PlayerGetFromSocket(nSocket)->iID;
			oUpdateOthersPosPacket.fX = fX;
			oUpdateOthersPosPacket.fY = fY;
			oUpdateOthersPosPacket.fZ = fZ;
			oUpdateOthersPosPacket.chFire = (char)bFire;

			for (int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
			{
				// Broadcast the position update message to others who are connected
				if (PlayerGet(nPlayer)->bConnected && nPlayer != PlayerGetFromSocket(nSocket)->iID) {
					if (sendall(PlayerGet(nPlayer)->nTcpSocket, (char *)&oUpdateOthersPosPacket, sizeof(oUpdateOthersPosPacket), 0) == SOCKET_ERROR) {
						NetworkPrintError("sendall");
					}
					//printf("Got update own position from player %d, sending to %d.\n",
					//	oUpdateOthersPosPacket.chPlayerID, nPlayer);
				}
			}
		}
		break;

	default:
		return false;
		break;
	}

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
