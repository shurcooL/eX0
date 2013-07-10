#pragma once
#ifndef __NetworkConnection_H__
#define __NetworkConnection_H__

enum JoinStatus {
	DISCONNECTED = 0,
	TCP_CONNECTED,
	ACCEPTED,
	UDP_CONNECTED,
	PUBLIC_CLIENT,		// This state means that all clients are now aware (or should be aware) of this client, so we'll need to notify them again if he leaves/changes team, etc.
	IN_GAME
};

class NetworkConnection
{
protected:
	NetworkConnection();
	NetworkConnection(const SOCKET nTcpSocket);
	virtual ~NetworkConnection();

public:
	void SetJoinStatus(const JoinStatus nJoinStatus);
	const JoinStatus GetJoinStatus() const;

	void SetTcpSocket(const SOCKET nTcpSocket);
	const SOCKET GetTcpSocket() const;

	void SetUdpSocket(const SOCKET nUdpSocket);
	const SOCKET GetUdpSocket() const;
	void SetUdpAddress(const sockaddr_in & oUdpAddress);
	const sockaddr_in & GetUdpAddress() const;

	static const u_char m_knSignatureSize = 8;
	void SetSignature(const u_char cSignature[m_knSignatureSize]);
	const u_char * GetSignature() const;

	void NotifyDataReceived();
	double GetTimeSinceLastReceive() const;

	virtual bool SendTcp(CPacket & oPacket, JoinStatus nMinimumJoinStatus = IN_GAME);
	virtual bool SendUdp(CPacket & oPacket, JoinStatus nMinimumJoinStatus = IN_GAME);

	virtual bool IsLocal() { return false; }

private:
	NetworkConnection(const NetworkConnection &);
	NetworkConnection & operator =(const NetworkConnection &);

	volatile JoinStatus		m_nJoinStatus;

	SOCKET		m_nTcpSocket;

	SOCKET		m_nUdpSocket;		// TODO: Create this socket in ServerConnection when SetUdpAddress is called... and use it in (to be added) SendUdp(), etc.
	sockaddr_in	m_oUdpAddress;

	u_char		m_cSignature[m_knSignatureSize];

	double		m_dLastReceivedTime;		// This contains the time the last packet was received
};

#endif // __NetworkConnection_H__
