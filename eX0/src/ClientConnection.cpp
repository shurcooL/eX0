// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

std::list<ClientConnection *> ClientConnection::m_oConnections;

ClientConnection::ClientConnection(SOCKET nTcpSocket)
	: NetworkConnection(nTcpSocket),
	  cCurrentUpdateSequenceNumber(0),
	  oTcpPacketBuffer(),
	  m_nLastLatency(0),
	  m_oPingSentTimes(PING_SENT_TIMES_HISTORY),
	  m_pPlayer(NULL),
	  m_oPlayers(),
	  m_nNonClientTimeoutEventId(0)
{
	printf("ClientConnection(%d) Ctor.\n", GetTcpSocket());

	// Schedule the non-client timeout event
	CTimedEvent oEvent = CTimedEvent(NON_CLIENT_TIMEOUT, 0, &ClientConnection::NonClientTimeout, this);
	m_nNonClientTimeoutEventId = oEvent.GetId();
	pTimedEventScheduler->ScheduleEvent(oEvent);

	m_oConnections.push_back(this);
}

ClientConnection::ClientConnection()
	: NetworkConnection(),
	  cCurrentUpdateSequenceNumber(0),
	  oTcpPacketBuffer(),
	  m_nLastLatency(0),
	  m_oPingSentTimes(PING_SENT_TIMES_HISTORY),
	  m_pPlayer(NULL),
	  m_oPlayers(),
	  m_nNonClientTimeoutEventId(0)
{
	printf("ClientConnection() Ctor.\n");

	m_oConnections.push_back(this);
}

ClientConnection::~ClientConnection()
{
	if (m_nNonClientTimeoutEventId != 0)
		CancelNonClientTimeout();

	while (IsMultiPlayer()) {
		delete m_oPlayers.at(m_oPlayers.size() - 1);
		m_oPlayers.pop_back();
	}
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
	for (std::list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1)
	{
		// Broadcast the packet to all players that are connected
		if ((*it1)->GetJoinStatus() >= nMinimumJoinStatus
			&& (oPacket.GetSendMode() == CPacket::BOTH || (oPacket.GetSendMode() == CPacket::NETWORK && !(*it1)->IsLocal()))) {
			/*if (sendall((*it1)->GetTcpSocket(), (char *)oPacket.GetPacket(), oPacket.size(), 0) == SOCKET_ERROR) {
				NetworkPrintError("sendall");
				return false;
			}*/
			(*it1)->SendTcp(oPacket, nMinimumJoinStatus);
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
	for (std::list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1)
	{
		// Broadcast the packet to all players that are connected, except the specified one
		if (*it1 != pConnection && (*it1)->GetJoinStatus() >= nMinimumJoinStatus
			&& (oPacket.GetSendMode() == CPacket::BOTH || (oPacket.GetSendMode() == CPacket::NETWORK && !(*it1)->IsLocal()))) {
			/*if (sendall((*it1)->GetTcpSocket(), (char *)oPacket.GetPacket(), oPacket.size(), 0) == SOCKET_ERROR) {
				NetworkPrintError("sendall");
				return false;
			}*/
			(*it1)->SendTcp(oPacket, nMinimumJoinStatus);
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
	for (std::list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1)
	{
		// Broadcast the packet to all players that are connected
		if ((*it1)->GetJoinStatus() >= nMinimumJoinStatus
			&& (oPacket.GetSendMode() == CPacket::BOTH || (oPacket.GetSendMode() == CPacket::NETWORK && !(*it1)->IsLocal()))) {
			/*if (sendudp((*it1)->GetUdpSocket(), (char *)oPacket.GetPacket(), oPacket.size(), 0,
				(sockaddr *)&(*it1)->GetUdpAddress(), sizeof((*it1)->GetUdpAddress())) != static_cast<int>(oPacket.size()))
			{
				NetworkPrintError("sendudp (sendto)");
				return false;
			}*/
			(*it1)->SendUdp(oPacket, nMinimumJoinStatus);
		}
	}
	return true;
}

bool ClientConnection::BroadcastUdpExcept(CPacket & oPacket, ClientConnection * pConnection, JoinStatus nMinimumJoinStatus)
{
	for (std::list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1)
	{
		// Broadcast the packet to all players that are connected, except the specified one
		if (*it1 != pConnection && (*it1)->GetJoinStatus() >= nMinimumJoinStatus
			&& (oPacket.GetSendMode() == CPacket::BOTH || (oPacket.GetSendMode() == CPacket::NETWORK && !(*it1)->IsLocal()))) {
			/*if (sendudp((*it1)->GetUdpSocket(), (char *)oPacket.GetPacket(), oPacket.size(), 0,
				(sockaddr *)&(*it1)->GetUdpAddress(), sizeof((*it1)->GetUdpAddress())) != static_cast<int>(oPacket.size()))
			{
				NetworkPrintError("sendudp (sendto)");
				return false;
			}*/
			(*it1)->SendUdp(oPacket, nMinimumJoinStatus);
		}
	}
	return true;
}

uint16 ClientConnection::GetLastLatency() const
{
	//return std::max<uint16>(m_nLastLatency, static_cast<uint16>(GetTimeSinceLastReceive() * 10000));
	return m_nLastLatency;
}
void ClientConnection::SetLastLatency(u_short nLastLatency) { m_nLastLatency = nLastLatency; }

HashMatcher<PingData_t, double> & ClientConnection::GetPingSentTimes() { return m_oPingSentTimes; }

uint8 ClientConnection::GetPlayerID() const { return (m_pPlayer == NULL) ? 123 : m_pPlayer->iID; }
void ClientConnection::SetPlayer(CPlayer * pPlayer)
{
	eX0_assert(pPlayer != NULL, "null pPlayer parameter");
	eX0_assert(GetPlayer() == NULL, "SetPlayer() should only be called once per connection.");

	m_pPlayer = pPlayer;
	if (m_pPlayer != NULL)
		m_pPlayer->pConnection = this;

	printf("Associated player id %d with socket %d.\n", GetPlayerID(), GetTcpSocket());
}

void ClientConnection::RemovePlayer()
{
	m_pPlayer = NULL;
}

bool ClientConnection::HasPlayer() const { return m_pPlayer != NULL; }
CPlayer * ClientConnection::GetPlayer() { return m_pPlayer; }

void ClientConnection::AddPlayer(CPlayer * pPlayer)
{
	eX0_assert(true == HasPlayer(), "true == HasPlayer()");
	eX0_assert(true == IsLocal(), "true == IsLocal() failed, means Non-Local client wants to be multiplayer");	// DEBUG: This is a temporary restriction, in future allow network clients to be multiplayer

	pPlayer->pConnection = this;

	m_oPlayers.push_back(pPlayer);
}

void ClientConnection::RemovePlayer(CPlayer * pPlayer)
{
	eX0_assert(true == HasPlayer(), "true == HasPlayer()");		// DEBUG: Should include the 1st player in the array also, or something
	eX0_assert(false == m_oPlayers.empty(), "false == m_oPlayers.empty()");
	eX0_assert(true == IsLocal(), "true == IsLocal() failed, means Non-Local client wants to be multiplayer");	// DEBUG: This is a temporary restriction, in future allow network clients to be multiplayer

	for (std::vector<CPlayer *>::iterator it1 = m_oPlayers.begin(); it1 != m_oPlayers.end(); ++it1)
	{
		if (*it1 == pPlayer) {
			if (NULL != pPlayer) pPlayer->pConnection = NULL;

			m_oPlayers.erase(it1);

			return;
		}
	}
}

CPlayer * ClientConnection::GetPlayer(u_int nPlayerNumber)
{
	eX0_assert(nPlayerNumber < GetPlayerCount(), "nPlayerNumber < GetPlayerCount()");
	eX0_assert(nPlayerNumber == 0 || true == IsLocal(), "nPlayerNumber == 0 || true == IsLocal() failed, means Non-Local client wants to be multiplayer");	// DEBUG: This is a temporary restriction, in future allow network clients to be multiplayer

	if (nPlayerNumber == 0) return GetPlayer();
	else return m_oPlayers.at(nPlayerNumber - 1);
}

u_int ClientConnection::GetPlayerCount() const
{
	return (false == HasPlayer() ? 0 : 1 + m_oPlayers.size());
}

// Returns a connection from its TCP socket number
ClientConnection * ClientConnection::GetFromTcpSocket(SOCKET nTcpSocket)
{
	for (std::list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1) {
		if ((*it1)->GetTcpSocket() == nTcpSocket)
			return *it1;
	}

	return NULL;
}

// Returns a connection from its UDP address
ClientConnection * ClientConnection::GetFromUdpAddress(sockaddr_in & oUdpAddress)
{
	for (std::list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1) {
		if ((*it1)->GetJoinStatus() >= UDP_CONNECTED
		  && memcmp(&(*it1)->GetUdpAddress().sin_addr, &oUdpAddress.sin_addr, sizeof(oUdpAddress.sin_addr)) == 0
		  && (*it1)->GetUdpAddress().sin_port == oUdpAddress.sin_port)
			return *it1;
	}

	return NULL;
}

// Returns a connection from its signature
ClientConnection * ClientConnection::GetFromSignature(u_char cSignature[m_knSignatureSize])
{
	for (std::list<ClientConnection *>::iterator it1 = m_oConnections.begin(); it1 != m_oConnections.end(); ++it1) {
		if ((*it1)->GetJoinStatus() == ACCEPTED && memcmp((*it1)->GetSignature(), cSignature, m_knSignatureSize) == 0)
			return *it1;
	}

	return NULL;
}

void ClientConnection::CancelNonClientTimeout()
{
	if (pTimedEventScheduler != NULL) {
		pTimedEventScheduler->RemoveEventById(m_nNonClientTimeoutEventId);
	}
	m_nNonClientTimeoutEventId = 0;
}

void ClientConnection::NonClientTimeout(void * pClientConnection)
{
	glfwLockMutex(oPlayerTick);

	// TODO: Perfect the 'ass dropping' mechanism...
	printf("drop his ass (socket %d)\n", reinterpret_cast<ClientConnection *>(pClientConnection)->GetTcpSocket());
	shutdown(reinterpret_cast<ClientConnection *>(pClientConnection)->GetTcpSocket(), SD_BOTH);
	//delete pClientConnection;

	glfwUnlockMutex(oPlayerTick);
}

void ClientConnection::CloseAll()
{
	while (!m_oConnections.empty())
		delete *(m_oConnections.begin());
}
