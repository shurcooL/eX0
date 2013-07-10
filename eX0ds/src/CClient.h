#pragma once
#ifndef __CClient_H__
#define __CClient_H__

class CClient
{
public:
	CClient(SOCKET nTcpSocket);
	~CClient();

	SOCKET GetSocket() const;
	const struct sockaddr_in & GetAddress() const;
	void SetAddress(struct sockaddr_in &oUdpAddress);

	const u_char * GetSignature(void) const;
	void SetSignature(u_char cSignature[SIGNATURE_SIZE]);

	JoinStatus GetJoinStatus(void) const;
	void SetJoinStatus(JoinStatus nJoinStatus);

	u_short GetLastLatency() const;
	void SetLastLatency(u_short nLastLatency);

	HashMatcher<PingData_t, double> & GetPingSentTimes();

	int GetPlayerID(void) const;
	void SetPlayerID(int nPlayerID);

	CPlayer * GetPlayer(void);

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

private:
	SOCKET		m_nTcpSocket;
	struct sockaddr_in	m_oUdpAddress;
	u_char		m_cSignature[SIGNATURE_SIZE];
	JoinStatus	m_nJoinStatus;

	u_short		m_nLastLatency;
	HashMatcher<PingData_t, double>	m_oPingSentTimes;

	int			m_nPlayerID;
};

// Returns a client from their socket number
CClient * ClientGetFromSocket(SOCKET nSocket);

// Returns a client from their address
CClient * ClientGetFromAddress(struct sockaddr_in &oAddress);

CClient * ClientGetFromSignature(u_char cSignature[SIGNATURE_SIZE]);

// Returns a player from their socket number
//CPlayer * ClientGetPlayerFromSocket(SOCKET nSocket);

// Returns a player from their address
//CPlayer * ClientGetPlayerFromAddress(struct sockaddr_in & oAddress);

void ClientDeinit(void);

#endif // __CClient_H__
