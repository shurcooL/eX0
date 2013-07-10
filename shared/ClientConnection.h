#pragma once
#ifndef __ClientConnection_H__
#define __ClientConnection_H__

class ClientConnection
	: public NetworkConnection
{
public:
	ClientConnection(SOCKET nTcpSocket);
	~ClientConnection();

	virtual u_short GetLastLatency() const;
	void SetLastLatency(u_short nLastLatency);

	HashMatcher<PingData_t, double> & GetPingSentTimes();

	u_int GetPlayerID() const;
	void SetPlayer(CPlayer * pPlayer);
	void RemovePlayer();
	bool HasPlayer() const;
	CPlayer * GetPlayer();

	void AddPlayer(CPlayer * pPlayer);
	void RemovePlayer(CPlayer * pPlayer);
	CPlayer * GetPlayer(u_int nPlayerNumber);
	u_int GetPlayerID(u_int nPlayerNumber) { return GetPlayer(nPlayerNumber)->iID; }
	u_int GetPlayerCount() const;
	bool IsMultiPlayer() const { return GetPlayerCount() > 1; }

	void CancelNonClientTimeout();

	u_char		cCurrentUpdateSequenceNumber;

	struct TcpPacketBuffer_t {
		u_char		cTcpPacketBuffer[2 * MAX_TCP_PACKET_SIZE - 1];	// Buffer for incoming TCP packets
		u_short		nCurrentPacketSize;

		TcpPacketBuffer_t() : nCurrentPacketSize(0) {}
	} oTcpPacketBuffer;

	static bool BroadcastTcp(CPacket & oPacket, JoinStatus nMinimumJoinStatus = IN_GAME);
	static bool BroadcastTcpExcept(CPacket & oPacket, ClientConnection * pConnection, JoinStatus nMinimumJoinStatus = IN_GAME);

	static bool BroadcastUdp(CPacket & oPacket, JoinStatus nMinimumJoinStatus = IN_GAME);
	static bool BroadcastUdpExcept(CPacket & oPacket, ClientConnection * pConnection, JoinStatus nMinimumJoinStatus = IN_GAME);

	static ClientConnection * GetFromTcpSocket(SOCKET nTcpSocket);
	static ClientConnection * GetFromUdpAddress(sockaddr_in & oUdpAddress);
	static ClientConnection * GetFromSignature(u_char cSignature[m_knSignatureSize]);

	static u_int size() { return m_oConnections.size(); }
	static void CloseAll();

protected:
	ClientConnection();

private:
	u_short		m_nLastLatency;
	HashMatcher<PingData_t, double>		m_oPingSentTimes;

	CPlayer *	m_pPlayer;
	std::vector<CPlayer *>	m_oPlayers;

	u_int		m_nNonClientTimeoutEventId;

	static std::list<ClientConnection *>		m_oConnections;

	static void NonClientTimeout(void * pClientConnection);
};

#endif // __ClientConnection_H__
