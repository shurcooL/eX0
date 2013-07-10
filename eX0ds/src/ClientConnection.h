#pragma once
#ifndef __ClientConnection_H__
#define __ClientConnection_H__

class ClientConnection
	: public NetworkConnection
{
public:
	ClientConnection(SOCKET nTcpSocket);
	~ClientConnection();

	u_short GetLastLatency() const;
	void SetLastLatency(u_short nLastLatency);

	HashMatcher<PingData_t, double> & GetPingSentTimes();

	u_int GetPlayerID() const;
	void SetPlayer(CPlayer * pPlayer);

	bool HasPlayer() const;
	CPlayer * GetPlayer();

	u_char		cLastCommandSequenceNumber;
	u_char		cCurrentCommandSeriesNumber;		// A number that changes on every respawn, team change, etc. and the server will ignore any Commands with mismatching series number
	u_char		cCurrentUpdateSequenceNumber;
	u_int		nUpdateEventId;
	u_char		cCommandRate;
	u_char		cUpdateRate;
	bool		bFirstCommand;		// When true, indicates we are expecting the first command from a client (so far got nothing) and will be set to false when it arrives

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

	static list<ClientConnection *>		m_oConnections;
};

#endif // __ClientConnection_H__
