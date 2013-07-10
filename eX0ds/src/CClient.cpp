#include "globals.h"

list<CClient *>		oClients;

CClient::CClient(SOCKET nTcpSocket)
	: oTcpPacketBuffer(), m_oPingSentTimes(PING_SENT_TIMES_HISTORY)
{
	// Network related
	m_nTcpSocket = nTcpSocket;
	m_nJoinStatus = TCP_CONNECTED;
	cLastCommandSequenceNumber = 0;
	cCurrentCommandSeriesNumber = 0;
	cCurrentUpdateSequenceNumber = 0;
	nUpdateEventId = 0;
	bFirstCommand = true;
	m_nLastLatency = 0;

	m_nPlayerID = -1;

	// Client list management
	oClients.push_back(this);

	printf("Created a client #%d (socket %d opened).\n", oClients.size(), GetSocket());
}

CClient::~CClient()
{
	if (nUpdateEventId != 0 && pTimedEventScheduler != NULL)
		pTimedEventScheduler->RemoveEventById(nUpdateEventId);

	if (GetPlayerID() != -1)
		PlayerGet(GetPlayerID())->pClient = NULL;

	// Close the client socket
	shutdown(GetSocket(), SD_BOTH);
	NetworkCloseSocket(GetSocket());

	// Client list management
	oClients.remove(this);

	printf("Deleted a client (socket %d closed).\n", GetSocket());
}

SOCKET CClient::GetSocket() const { return m_nTcpSocket; }
const struct sockaddr_in & CClient::GetAddress() const { return m_oUdpAddress; }
void CClient::SetAddress(struct sockaddr_in &oUdpAddress) { m_oUdpAddress = oUdpAddress; }

const u_char * CClient::GetSignature(void) const { return m_cSignature; }
void CClient::SetSignature(u_char cSignature[SIGNATURE_SIZE]) {
	memcpy(m_cSignature, cSignature, SIGNATURE_SIZE);
}

JoinStatus CClient::GetJoinStatus(void) const { return m_nJoinStatus; }
void CClient::SetJoinStatus(JoinStatus nJoinStatus) { m_nJoinStatus = nJoinStatus; }

u_short CClient::GetLastLatency() const { return m_nLastLatency; }
void CClient::SetLastLatency(u_short nLastLatency) { m_nLastLatency = nLastLatency; }

HashMatcher<PingData_t, double> & CClient::GetPingSentTimes() { return m_oPingSentTimes; }

int CClient::GetPlayerID(void) const { return m_nPlayerID; }
void CClient::SetPlayerID(int nPlayerID)
{
	eX0_assert(GetPlayerID() == -1, "SetPlayerID() should only be called once per client.");

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
CClient * ClientGetFromAddress(struct sockaddr_in &oAddress)
{
	for (list<CClient *>::iterator it1 = oClients.begin(); it1 != oClients.end(); ++it1) {
		if ((*it1)->GetJoinStatus() >= UDP_CONNECTED
		  && memcmp(&(*it1)->GetAddress().sin_addr, &oAddress.sin_addr, sizeof(struct in_addr)) == 0
		  && (*it1)->GetAddress().sin_port == oAddress.sin_port)
			return *it1;
	}

	return NULL;
}

CClient * ClientGetFromSignature(u_char cSignature[SIGNATURE_SIZE])
{
	for (list<CClient *>::iterator it1 = oClients.begin(); it1 != oClients.end(); ++it1) {
		if ((*it1)->GetJoinStatus() == ACCEPTED
		  && memcmp((*it1)->GetSignature(), cSignature, SIGNATURE_SIZE) == 0)
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
CPlayer * ClientGetPlayerFromAddress(struct sockaddr_in &oAddress)
{
	// TODO

	return NULL;
}*/

void ClientDeinit()
{
	while (!oClients.empty()) {
		delete *(oClients.begin());
	}
}
