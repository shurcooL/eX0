#pragma once
#ifndef __Network_H__
#define __Network_H__

class CPacket;

#define NETWORK_PROTOCOL_VERSION	1
#define NETWORK_PROTOCOL_PASSPHRASE	"somerandompass01"

#define DEFAULT_PORT				25035

#define BROADCAST_PING_PERIOD		2.5		// How often to broadcast the Ping packet on the server
#define PING_SENT_TIMES_HISTORY		5		// How many Ping or Pong sent-times to keep track of

#define NON_CLIENT_TIMEOUT			5.0		// Time (in sec) a newly connected client has to introduce itself before it gets dropped (i.e. kick anything but normal clients)
#define UDP_HANDSHAKE_RETRY_TIME	0.1		// Time (in sec) to retransmit UDP Handshake packets until we become UDP_ACCEPTED
#define TIME_REQUEST_SEND_RATE		0.05	// Time (in sec) in between sending of each Time Request packet

// Packets
#define MAX_TCP_PACKET_SIZE			1448
#define MAX_UDP_PACKET_SIZE			1448
#define MAX_PACKET_SIZE				(std::max<u_int>(MAX_TCP_PACKET_SIZE, MAX_UDP_PACKET_SIZE))

typedef struct PingData_st {
	u_char	cPingData[4];

	bool operator ==(const PingData_st & oOther) const
	{
		//return *reinterpret_cast<const u_int *>(cPingData) == *reinterpret_cast<const u_int *>(oOther.cPingData);
		return (0 == memcmp(cPingData, oOther.cPingData, 4));
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
int sendudp(SOCKET s, const char *buf, int len, int flags, const sockaddr *to = NULL, int tolen = 0);

#ifdef EX0_CLIENT
// Connect to a server
bool NetworkConnect(const char * szHostname, u_short nPort);

bool NetworkCreateThread(void);

void GLFWCALL NetworkThread(void * pArgument);
#endif // EX0_CLIENT

#ifdef EX0_CLIENT
// Process a received TCP packet
bool NetworkProcessTcpPacket(CPacket & oPacket);

// Process a received UDP packet
bool NetworkProcessUdpPacket(CPacket & oPacket);
//#else
#endif // EX0_CLIENT
// Process a received TCP packet
bool NetworkProcessTcpPacket(CPacket & oPacket, ClientConnection *& pConnection);

// Process a received UDP packet
bool NetworkProcessUdpPacket(CPacket & oPacket, ClientConnection * pConnection);
//#endif // EX0_CLIENT

#ifdef EX0_CLIENT
void NetworkSendUdpHandshakePacket(void *pArgument);

void NetworkJoinGame();

void NetworkShutdownThread();

void NetworkDestroyThread();
//#else
#endif // EX0_CLIENT
// Process a potential UDP Handshake packet
bool NetworkProcessUdpHandshakePacket(u_char * cPacket, u_short nPacketSize, sockaddr_in & oSenderAddress, SOCKET nUdpSocket);
//#endif // EX0_CLIENT

// Shutdown the networking component
void NetworkDeinit();

// Closes a socket
void NetworkCloseSocket(SOCKET nSocket);

#endif // __Network_H__
