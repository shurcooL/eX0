class CPacket;

#define DEFAULT_PORT			9034

#define SIGNATURE_SIZE			8

// Packets
#define MAX_TCP_PACKET_SIZE		1448
#define MAX_UDP_PACKET_SIZE		1448
#define MAX_PACKET_SIZE			__max(MAX_TCP_PACKET_SIZE, MAX_UDP_PACKET_SIZE)

enum JoinStatus {
	DISCONNECTED = 0,
	TCP_CONNECTED,
	ACCEPTED,
	UDP_CONNECTED,
	IN_GAME
};

// Initialize the networking component
bool NetworkInit(void);

// Prints the last error code
void NetworkPrintError(const char *szMessage);

int sendall(SOCKET s, char *buf, int len, int flags);

// Process a received TCP packet
bool NetworkProcessTcpPacket(CPacket & oPacket, CClient * pClient);

// Process a received UDP packet
bool NetworkProcessUdpPacket(CPacket & oPacket, int nPacketSize, CClient * pClient);

// Process a potential UDP Handshake packet
bool NetworkProcessUdpHandshakePacket(char * oPacket, int nPacketSize, struct sockaddr_in & oSenderAddress);

// Shutdown the networking component
void NetworkDeinit();

// Closes a socket
void NetworkCloseSocket(SOCKET nSocket);
