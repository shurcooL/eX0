#include "globals.h"

list<CClient *>		oClients;

CClient::CClient(SOCKET nTcpSocket)
{
	// Network related
	m_nTcpSocket = nTcpSocket;
	m_nJoinStatus = TCP_CONNECTED;
	cLastMovementSequenceNumber = 0;

	m_nPlayerID = -1;

	// Client list management
	oClients.push_back(this);

	printf("Created a client #%d (socket %d).\n", oClients.size(), GetSocket());
}

CClient::~CClient()
{
	if (GetPlayerID() != -1)
		PlayerGet(GetPlayerID())->pClient = NULL;

	// Close the client socket
	shutdown(GetSocket(), SD_BOTH);
	NetworkCloseSocket(GetSocket());

	// Client list management
	oClients.remove(this);

	printf("Deleted a client (socket %d closed).\n", GetSocket());
}

SOCKET CClient::GetSocket() { return m_nTcpSocket; }
struct sockaddr_in & CClient::GetAddress() { return m_oUdpAddress; }
void CClient::SetAddress(struct sockaddr_in & oUdpAddress) { m_oUdpAddress = oUdpAddress; }

char * CClient::GetSignature(void) { return m_cSignature; }
void CClient::SetSignature(char cSignature[4]) {
	memcpy(m_cSignature, cSignature, 4);
}

JoinStatus CClient::GetJoinStatus(void) { return m_nJoinStatus; }
void CClient::SetJoinStatus(JoinStatus nJoinStatus) { m_nJoinStatus = nJoinStatus; }

int CClient::GetPlayerID(void) { return m_nPlayerID; }
void CClient::SetPlayerID(int nPlayerID)
{
	if (GetPlayerID() != -1) {
		//PlayerGet(GetPlayerID())->pClient = NULL;
		assert(false); // should only be called once per client
	}
	m_nPlayerID = nPlayerID;
	if (GetPlayerID() != -1)
		PlayerGet(GetPlayerID())->pClient = this;

	printf("Associated player id %d with socket %d.\n", GetPlayerID(), GetSocket());
}

CPlayer * CClient::GetPlayer(void)
{
	return PlayerGet(GetPlayerID());
}

// Returns a client from their socket number
CClient * ClientGetFromSocket(SOCKET nSocket)
{
	for (list<CClient *>::iterator it1 = oClients.begin(); it1 != oClients.end(); ++it1) {
		if ((*it1)->GetSocket() == nSocket)
			return *it1;
	}

	return NULL;
}

// Returns a client from their address
CClient * ClientGetFromAddress(struct sockaddr_in & oAddress)
{
	for (list<CClient *>::iterator it1 = oClients.begin(); it1 != oClients.end(); ++it1) {
		if ((*it1)->GetJoinStatus() >= UDP_CONNECTED
		  && memcmp(&(*it1)->GetAddress().sin_addr, &oAddress.sin_addr, sizeof(struct in_addr)) == 0
		  && (*it1)->GetAddress().sin_port == oAddress.sin_port)
			return *it1;
	}

	return NULL;
}

CClient * ClientGetFromSignature(char cSignature[4])
{
	for (list<CClient *>::iterator it1 = oClients.begin(); it1 != oClients.end(); ++it1) {
		if ((*it1)->GetJoinStatus() == ACCEPTED
		  && memcmp((*it1)->GetSignature(), cSignature, 4) == 0)
			return *it1;
	}

	return NULL;
}

// Returns a player from their socket number
/*CPlayer * ClientGetPlayerFromSocket(SOCKET nSocket)
{
	for (int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
		if (PlayerGet(nPlayer)->pClient != NULL
		  && PlayerGet(nPlayer)->pClient->GetSocket() == nSocket)
			return PlayerGet(nPlayer);

	return NULL;
}

// Returns a player from their address
CPlayer * ClientGetPlayerFromAddress(struct sockaddr_in & oAddress)
{
	// TODO

	return NULL;
}*/

void ClientDeinit(void)
{
	while (!oClients.empty()) {
		delete *(oClients.begin());
	}
}
