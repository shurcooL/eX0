#include "globals.h"

ServerConnection::ServerConnection()
	: NetworkConnection(),
	  cLastUpdateSequenceNumber(0)
{
	printf("ServerConnection(DISCONNECTED) Ctor.\n");
}

ServerConnection::~ServerConnection()
{
	printf("ServerConnection(%d) ~Dtor.\n", GetTcpSocket());
}

bool ServerConnection::Connect(const char * szHostname, u_short nPort)
{
	hostent *he;
	sockaddr_in	oServerAddress;
	SOCKET nTcpSocket, nUdpSocket;

	if ((he = gethostbyname(szHostname)) == NULL) {		// get the host info
		NetworkPrintError("gethostbyname");		// herror
		Terminate(1);
	}

	if ((nTcpSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
	    NetworkPrintError("socket");
	    Terminate(1);
	}
	SetTcpSocket(nTcpSocket);
	if ((nUdpSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
	    NetworkPrintError("socket");
	    Terminate(1);
	}
	SetUdpSocket(nUdpSocket);

	/* This is the default behaviour already (graceful shutdown, don't linger)
	linger oLinger;
	oLinger.l_onoff = 0;
	oLinger.l_linger = 0;
	if (setsockopt(nTcpSocket, SOL_SOCKET, SO_LINGER, (char *)&oLinger, sizeof(oLinger)) == SOCKET_ERROR) {
		NetworkPrintError("setsockopt");
		Terminate(1);
	}*/
	// Disable the Nagle algorithm for send coalescing
	int nNoDelay = 1;
	if (setsockopt(nTcpSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&nNoDelay, sizeof(nNoDelay)) == SOCKET_ERROR) {
		NetworkPrintError("setsockopt");
		Terminate(1);
	}

	oServerAddress.sin_family = AF_INET;
	oServerAddress.sin_addr = *((in_addr *)he->h_addr);
	oServerAddress.sin_port = htons(nPort);
	memset(oServerAddress.sin_zero, 0, sizeof(oServerAddress.sin_zero));
	SetUdpAddress(oServerAddress);

	printf("Connecting to %s.\n", szHostname);
	if (connect(nTcpSocket, (sockaddr *)&oServerAddress, sizeof(oServerAddress)) == SOCKET_ERROR) {
	    NetworkPrintError("connect");
	    Terminate(1);
	}
	sockaddr_in	oLocalTcpAddress;
	socklen_t			nLocalTcpAddressLength = sizeof(oLocalTcpAddress);
	if (getsockname(nTcpSocket, (sockaddr *)&oLocalTcpAddress, &nLocalTcpAddressLength) == SOCKET_ERROR) {
		NetworkPrintError("getsockname");
		Terminate(1);
	}

	// Successfully connected (TCP) to the server
	SetJoinStatus(TCP_CONNECTED);
	printf("Established a TCP connection (local port %d), attempting to join the game.\n", ntohs(oLocalTcpAddress.sin_port));

	return true;
}

void ServerConnection::GenerateSignature()
{
	double dSignature = glfwGetTime();
	//memcpy(m_cSignature, (void *)&dSignature, m_knSignatureSize);
	SetSignature(reinterpret_cast<u_char *>(&dSignature));
}
