#include "globals.h"

list<ClientConnection *> ClientConnection::m_oConnections;

ClientConnection::ClientConnection(SOCKET nTcpSocket)
	: NetworkConnection(nTcpSocket),
	  oTcpPacketBuffer(), m_oPingSentTimes(PING_SENT_TIMES_HISTORY)
{
	printf("ClientConnection(%d) Ctor.\n", GetTcpSocket());

	cLastCommandSequenceNumber = 0;
	cCurrentCommandSeriesNumber = 0;
	cCurrentUpdateSequenceNumber = 0;
	nUpdateEventId = 0;
	bFirstCommand = true;
	m_nLastLatency = 0;

	m_pPlayer = NULL;

	m_oConnections.push_back(this);
}

ClientConnection::~ClientConnection()
{
	if (nUpdateEventId != 0 && pTimedEventScheduler != NULL)
		pTimedEventScheduler->RemoveEventById(nUpdateEventId);

	if (GetPlayer() != NULL)
		delete GetPlayer();

	m_oConnections.remove(this);

	printf("ClientConnection(%d) ~Dtor.\n", GetTcpSocket());
}

bool ClientConnection::BroadcastTcp(CPacket & oPacket, JoinStatus nMinimumJoinStatus)
{
	/*for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
	{
		// Broadcast the packet to all players that are connected
		if (PlayerGet(nPlayer) != NULL
		  && PlayerGet(nPlayer)->pConnection->GetJoinStatus() >= nMinimumJoinStatus) {
			if (sendall(PlayerGet(nPlayer)->pConnection->GetTcpSocket(), (char *)oPacket.GetPacket(), oPacket.size(), 0) == SOCKET_ERROR) {
				NetworkPrintError("sendall");
				return false;
			}
		}
	}
	return true;*/
	for (list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1)
	{
		// Broadcast the packet to all players that are connected
		if ((*it1)->GetJoinStatus() >= nMinimumJoinStatus) {
			if (sendall((*it1)->GetTcpSocket(), (char *)oPacket.GetPacket(), oPacket.size(), 0) == SOCKET_ERROR) {
				NetworkPrintError("sendall");
				return false;
			}
		}
	}
	return true;
}

bool ClientConnection::BroadcastTcpExcept(CPacket & oPacket, ClientConnection * pConnection, JoinStatus nMinimumJoinStatus)
{
	/*for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
	{
		// Broadcast the packet to all players that are connected, except the specified one
		if (PlayerGet(nPlayer) != NULL && pConnection != PlayerGet(nPlayer)->pConnection
		  && PlayerGet(nPlayer)->pConnection->GetJoinStatus() >= nMinimumJoinStatus) {
			if (sendall(PlayerGet(nPlayer)->pConnection->GetTcpSocket(), (char *)oPacket.GetPacket(), oPacket.size(), 0) == SOCKET_ERROR)
			{
				NetworkPrintError("sendall");
				return false;
			}
		}
	}
	return false;*/
	for (list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1)
	{
		// Broadcast the packet to all players that are connected, except the specified one
		if (*it1 != pConnection && (*it1)->GetJoinStatus() >= nMinimumJoinStatus) {
			if (sendall((*it1)->GetTcpSocket(), (char *)oPacket.GetPacket(), oPacket.size(), 0) == SOCKET_ERROR) {
				NetworkPrintError("sendall");
				return false;
			}
		}
	}
	return true;
}

bool ClientConnection::BroadcastUdp(CPacket & oPacket, JoinStatus nMinimumJoinStatus)
{
	/*for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
	{
		// Broadcast the packet to all players that are connected
		if (PlayerGet(nPlayer) != NULL
		  && PlayerGet(nPlayer)->pConnection->GetJoinStatus() >= nMinimumJoinStatus) {
			if (sendudp(nUdpSocket, (char *)oPacket.GetPacket(), oPacket.size(), 0,
				(sockaddr *)&PlayerGet(nPlayer)->pConnection->GetUdpAddress(), sizeof(PlayerGet(nPlayer)->pConnection->GetUdpAddress())) != oPacket.size())
			{
				NetworkPrintError("sendudp (sendto)");
				return false;
			}
		}
	}
	return true;*/
	for (list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1)
	{
		// Broadcast the packet to all players that are connected
		if ((*it1)->GetJoinStatus() >= nMinimumJoinStatus) {
			if (sendudp((*it1)->GetUdpSocket(), (char *)oPacket.GetPacket(), oPacket.size(), 0,
				(sockaddr *)&(*it1)->GetUdpAddress(), sizeof((*it1)->GetUdpAddress())) != static_cast<int>(oPacket.size()))
			{
				NetworkPrintError("sendudp (sendto)");
				return false;
			}
		}
	}
	return true;
}

/*bool ClientConnection::BroadcastUdpExcept(CPacket & oPacket, ClientConnection * pConnection, JoinStatus nMinimumJoinStatus)
{
	// TODO
	printf("BroadcastUdpExcept failed because it's not finished\n");
	throw 0;
}*/

u_short ClientConnection::GetLastLatency() const { return m_nLastLatency; }
void ClientConnection::SetLastLatency(u_short nLastLatency) { m_nLastLatency = nLastLatency; }

HashMatcher<PingData_t, double> & ClientConnection::GetPingSentTimes() { return m_oPingSentTimes; }

u_int ClientConnection::GetPlayerID() const { return (m_pPlayer == NULL) ? 123 : m_pPlayer->iID; }
void ClientConnection::SetPlayer(CPlayer * pPlayer)
{
	eX0_assert(pPlayer != NULL, "null parameter");
	eX0_assert(GetPlayer() == NULL, "SetPlayerID() should only be called once per connection.");

	m_pPlayer = pPlayer;
	if (GetPlayer() != NULL)
		PlayerGet(GetPlayerID())->pConnection = this;

	printf("Associated player id %d with socket %d.\n", GetPlayerID(), GetTcpSocket());
}

bool ClientConnection::HasPlayer() const { return m_pPlayer != NULL; }
CPlayer * ClientConnection::GetPlayer()
{
	return m_pPlayer;
}

// Returns a connection from its TCP socket number
ClientConnection * ClientConnection::GetFromTcpSocket(SOCKET nTcpSocket)
{
	for (list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1) {
		if ((*it1)->GetTcpSocket() == nTcpSocket)
			return *it1;
	}

	return NULL;
}

// Returns a connection from its UDP address
ClientConnection * ClientConnection::GetFromUdpAddress(sockaddr_in & oUdpAddress)
{
	for (list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1) {
		if ((*it1)->GetJoinStatus() >= UDP_CONNECTED
		  && memcmp(&(*it1)->GetUdpAddress().sin_addr, &oUdpAddress.sin_addr, sizeof(in_addr)) == 0
		  && (*it1)->GetUdpAddress().sin_port == oUdpAddress.sin_port)
			return *it1;
	}

	return NULL;
}

// Returns a connection from its signature
ClientConnection * ClientConnection::GetFromSignature(u_char cSignature[m_knSignatureSize])
{
	for (list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1) {
		if ((*it1)->GetJoinStatus() == ACCEPTED && memcmp((*it1)->GetSignature(), cSignature, m_knSignatureSize) == 0)
			return *it1;
	}

	return NULL;
}

void ClientConnection::CloseAll()
{
	while (!m_oConnections.empty())
		delete *(m_oConnections.begin());
}
