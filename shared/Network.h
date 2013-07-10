#pragma once
#ifndef __Network_H__
#define __Network_H__

class CPacket;

#define DEFAULT_PORT			25035

#define SIGNATURE_SIZE			8
#define BROADCAST_PING_PERIOD	2.5		// How often to broadcast the Ping packet on the server
#define PING_SENT_TIMES_HISTORY	5		// How many Ping or Pong sent-times to keep track of

// Packets
#define MAX_TCP_PACKET_SIZE		1448
#define MAX_UDP_PACKET_SIZE		1448
#define MAX_PACKET_SIZE			(std::max<int>(MAX_TCP_PACKET_SIZE, MAX_UDP_PACKET_SIZE))

enum JoinStatus {
	DISCONNECTED = 0,
	TCP_CONNECTED,
	ACCEPTED,
	UDP_CONNECTED,
	PUBLIC_CLIENT,		// This state means that all clients are now aware (or should be aware) of this client, so we'll need to notify them again if he leaves/changes team, etc.
	IN_GAME
};

typedef struct PingData_st {
	u_char	cPingData[4];

	bool operator ==(const PingData_st &oOther) const
	{
		return *reinterpret_cast<const u_int *>(cPingData) == *reinterpret_cast<const u_int *>(oOther.cPingData);
	}
} PingData_t;

#ifdef EX0_CLIENT
extern const float	kfInterpolate;
extern const float	kfMaxExtrapolate;
#endif // EX0_CLIENT

// Initialize the networking component
bool NetworkInit();

// Prints the last error code
void NetworkPrintError(const char *szMessage);

int sendall(SOCKET s, const char *buf, int len, int flags);
int sendudp(SOCKET s, const char *buf, int len, int flags, const sockaddr *to, int tolen);
int sendudp(SOCKET s, const char *buf, int len, int flags);

#ifdef EX0_CLIENT
// Connect to a server
bool NetworkConnect(char *szHost, int nPort);

bool NetworkCreateThread(void);

void GLFWCALL NetworkThread(void *pArgument);
#endif // EX0_CLIENT

#ifdef EX0_CLIENT
// Process a received TCP packet
bool NetworkProcessTcpPacket(CPacket & oPacket);

// Process a received UDP packet
bool NetworkProcessUdpPacket(CPacket & oPacket);
#else
// Process a received TCP packet
bool NetworkProcessTcpPacket(CPacket & oPacket, CClient *& pClient);

// Process a received UDP packet
bool NetworkProcessUdpPacket(CPacket & oPacket, CClient * pClient);
#endif // EX0_CLIENT

#ifdef EX0_CLIENT
void NetworkSendUdpHandshakePacket(void *pArgument);

void NetworkJoinGame();

void NetworkShutdownThread();

void NetworkDestroyThread();
#else
// Process a potential UDP Handshake packet
bool NetworkProcessUdpHandshakePacket(u_char * cPacket, u_short nPacketSize, struct sockaddr_in & oSenderAddress);
#endif // EX0_CLIENT

// Shutdown the networking component
void NetworkDeinit();

// Closes a socket
void NetworkCloseSocket(SOCKET nSocket);

#endif // __Network_H__
