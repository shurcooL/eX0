#pragma once
#ifndef __RemoteClientConnection_H__
#define __RemoteClientConnection_H__

class RemoteClientConnection
	: public ClientConnection
{
public:
	RemoteClientConnection();
	~RemoteClientConnection();

	/*bool SendTcp(CPacket & oPacket, JoinStatus nMinimumJoinStatus = IN_GAME);
	bool SendUdp(CPacket & oPacket, JoinStatus nMinimumJoinStatus = IN_GAME);

	bool IsLocal();// { return false; }

	u_short GetLastLatency() const;
	void SetLastLatency(u_short nLastLatency);

	HashMatcher<PingData_t, double> & GetPingSentTimes();

	u_int GetPlayerID() const;
	void SetPlayer(CPlayer * pPlayer);
	bool HasPlayer() const;
	CPlayer * GetPlayer();

	void CancelBadClientTimeout();

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

private:
	u_short		m_nLastLatency;
	HashMatcher<PingData_t, double>		m_oPingSentTimes;

	CPlayer * m_pPlayer;

	u_int		m_nBadClientTimeoutEventId;

	static std::list<ClientConnection *>		m_oConnections;

	static void BadClientTimeout(void * pClientConnection);*/

private:
	RemoteClientConnection(const RemoteClientConnection &);
	RemoteClientConnection & operator =(const RemoteClientConnection &);
};

#endif // __RemoteClientConnection_H__
