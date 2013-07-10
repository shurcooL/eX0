#include "globals.h"

GLFWthread		oServerThread = -1;
volatile bool	bServerThreadRun;

fd_set			master;   // master file descriptor list
fd_set			read_fds; // temp file descriptor list for select()
SOCKET			listener;		// listening socket descriptor
SOCKET			nUdpSocket;
list<SOCKET>	oActiveSockets;

// Initialize the server
bool ServerInit()
{
	return true;
}

// Start the server
bool ServerStart()
{
	struct sockaddr_in myaddr;	 // server address
	int nYes = 1;		// for setsockopt() SO_REUSEADDR, below

	FD_ZERO(&master);	// clear the master and temp sets
	FD_ZERO(&read_fds);
	oActiveSockets.clear();

	// Create the listener socket
	if ((listener = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		NetworkPrintError("socket");
		return false;
	}
	printf("Created TCP listener socket #%d.\n", (int)listener);
	// Create the UDP socket
	if ((nUdpSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		NetworkPrintError("socket");
		return false;
	}
	printf("Created UDP socket #%d.\n", (int)nUdpSocket);

	// lose the pesky "address already in use" error message
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *)&nYes, sizeof(nYes)) == SOCKET_ERROR) {
		NetworkPrintError("setsockopt");
		return false;
	}
	// Disable the Nagle algorithm for send coalescing
	if (setsockopt(listener, IPPROTO_TCP, TCP_NODELAY, (char *)&nYes, sizeof(nYes)) == SOCKET_ERROR) {
		NetworkPrintError("setsockopt(nodelay)");
		return false;
	}

	// Bind the TCP listening socket
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(DEFAULT_PORT);
	memset(myaddr.sin_zero, 0, sizeof(myaddr.sin_zero));
	if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == SOCKET_ERROR) {
		NetworkPrintError("bind");
		return false;
	}
	// Bind the UDP socket
	if (bind(nUdpSocket, (struct sockaddr *)&myaddr, sizeof(myaddr)) == SOCKET_ERROR) {
		NetworkPrintError("bind");
		return false;
	}

	// listen
	if (listen(listener, 10) == SOCKET_ERROR) {
		NetworkPrintError("listen");
		return false;
	}

	// add the listener to the master set
	FD_SET(listener, &master);
	oActiveSockets.push_back(listener);
	FD_SET(nUdpSocket, &master);
	oActiveSockets.push_back(nUdpSocket);

	// Create the server thread
	if (!ServerCreateThread()) {
		printf("Couldn't create the server thread.\n");
		return false;
	}

	return true;
}

bool ServerCreateThread()
{
	bServerThreadRun = true;
	oServerThread = glfwCreateThread(ServerThread, NULL);

	printf("Server thread created.\n");

	return oServerThread >= 0;
}

void GLFWCALL ServerThread(void *pArg)
{
	struct sockaddr_in remoteaddr; // incoming client address
	int fdmax;		// maximum file descriptor number, for Linux select()
	SOCKET newfd;		// newly accept()ed socket descriptor
	struct sockaddr_in	oSenderAddr;
	socklen_t			nSenderAddrLen = sizeof(oSenderAddr);
	char buf[2 * MAX_TCP_PACKET_SIZE];	// Buffer for incoming TCP packets
	char				cUdpBuffer[MAX_UDP_PACKET_SIZE];
	int nYes = 1;		// for setsockopt() SO_REUSEADDR, below
	int nbytes;
	socklen_t addrlen;
	u_short				snCurrentPacketSize = 0;

	// keep track of the biggest file descriptor
	fdmax = __max((int)listener, (int)nUdpSocket); // so far, it's one of these two

	// Main server loop
	while (bServerThreadRun)
	{
		read_fds = master; // copy it
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == SOCKET_ERROR) {
			NetworkPrintError("select");
			Terminate(1);
		}

		if (!bServerThreadRun) {
			break;
		}

		// run through the existing connections looking for data to read
		//for (int nLoop1 = 0; nLoop1 < (int)read_fds.fd_count; ++nLoop1) {
		//for (int nLoop1 = -1; nLoop1 < nPlayerCount; ++nLoop1) {
		for (list<SOCKET>::iterator it1 = oActiveSockets.begin(); it1 != oActiveSockets.end(); )
		{
			bool bGoToNextIt = true;

			/*if (nLoop1 == -1) i = (int)listener;
			else i = (int)PlayerGet(nLoop1)->nTcpSocket;*/
			SOCKET i = *it1;

			// Socket is ready for reading
			if (FD_ISSET(i, &read_fds)) {
				// handle new connections
				if (i == listener)
				{
					addrlen = sizeof(remoteaddr);
					if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr,
														&addrlen)) == INVALID_SOCKET) {
						NetworkPrintError("accept");
					} else {
						FD_SET(newfd, &master); // add to master set
						oActiveSockets.push_back((SOCKET)newfd);
						fdmax = __max((int)newfd, fdmax);		// keep track of the maximum

						new CClient(newfd);

						printf("eX0ds: new connection from %s:%d on socket %d\n",
							inet_ntoa(remoteaddr.sin_addr), ntohs(remoteaddr.sin_port), (int)newfd);

						// Disable the Nagle algorithm for send coalescing
						if (setsockopt(newfd, IPPROTO_TCP, TCP_NODELAY, (char *)&nYes, sizeof(nYes)) == SOCKET_ERROR) {
							NetworkPrintError("setsockopt");
							Terminate(1);
						}
					}
				}
				// Handle UDP data from a client
				else if (i == nUdpSocket)
				{
					if ((nbytes = recvfrom(nUdpSocket, cUdpBuffer, sizeof(cUdpBuffer), 0,
					  (struct sockaddr *)&oSenderAddr, &nSenderAddrLen)) == SOCKET_ERROR)
					{
						// Error
						NetworkPrintError("recvfrom");
					} else {
						// Got a UDP packet
						//printf("Got a UDP %d byte packet from %s:%d!\n", nbytes,
						//	inet_ntoa(oSenderAddr.sin_addr), ntohs(oSenderAddr.sin_port));
						CClient * pClient;

						if (nbytes <= 0 || nbytes > MAX_UDP_PACKET_SIZE)
							printf("Got a UDP packet of improper size (%d bytes), discarding.\n", nbytes);
						else if ((pClient = ClientGetFromAddress(oSenderAddr)) == NULL)
						{
							// Check for the special case of UDP Handshake packet
							bool bUdpHandshakePacket = NetworkProcessUdpHandshakePacket(cUdpBuffer, nbytes, oSenderAddr);

							if (!bUdpHandshakePacket)
								printf("Got a UDP packet from unknown sender (%s:%d), discarding.\n",
								  inet_ntoa(oSenderAddr.sin_addr), ntohs(oSenderAddr.sin_port));
						} else {
							// Process the received UDP packet
							CPacket oPacket(cUdpBuffer);
							if (!NetworkProcessUdpPacket(oPacket, nbytes, pClient)) {
								printf("Couldn't process a UDP packet (type %d):\n", *((u_char *)cUdpBuffer));
								oPacket.Print(nbytes);
							}
						}
					}
				}
				// Handle TCP data from a client
				else {
					// Recv returned 0 or less than 0
					if ((nbytes = recv(i, buf + snCurrentPacketSize, sizeof(buf) - snCurrentPacketSize, 0)) <= 0)
					{
						CClient * pClient = ClientGetFromSocket(i);

						if (pClient == NULL) {
							printf("Got an error/disconnect on a non-existing client socket (%d), ignoring.\n", i);
						} else {
							// got error or connection closed by client
							if (nbytes == 0) {
								if (pClient->GetJoinStatus() >= ACCEPTED) {
									// connection closed
									printf("Player #%d (name '%s') has left the game (gracefully).\n", pClient->GetPlayerID(),
										pClient->GetPlayer()->GetName().c_str());
								} else {
									printf("eX0ds: socket %d hung up nicely (was not a player)\n", (int)i);
								}
							} else {
								NetworkPrintError("recv");

								if (pClient->GetJoinStatus() >= ACCEPTED) {
									// connection closed
									printf("Player #%d (name '%s') has left the game (connection terminated).\n", pClient->GetPlayerID(),
										pClient->GetPlayer()->GetName().c_str());
								} else {
									printf("eX0ds: socket %d terminated hard (error), was not a player\n", (int)i);
								}
							}

							if (pClient->GetJoinStatus() >= IN_GAME) {
								// Send a Player Left Game to all the other clients
								CPacket oPlayerLeftGamePacket;
								oPlayerLeftGamePacket.pack("hh", 0, (u_short)26);
								oPlayerLeftGamePacket.pack("c", (u_char)pClient->GetPlayerID());
								oPlayerLeftGamePacket.CompleteTpcPacketSize();
								oPlayerLeftGamePacket.BroadcastTcpExcept(pClient, UDP_CONNECTED);
							}

							// Remove the player, if he's already connected
							//if (PlayerGetFromSocket(i) != NULL)
							//	PlayerGetFromSocket(i)->bConnected = false;
							delete pClient;

							//NetworkCloseSocket(i); // bye!
							FD_CLR(i, &master); // remove from master set
							it1 = oActiveSockets.erase(it1);
							bGoToNextIt = false;
						}
					}
					// Recv returned greater than 0
					else
					{
						// we got some TCP data from a client, process it
						//printf("Got %d bytes from client #%d\n", nbytes, (int)i);
						CClient * pClient = ClientGetFromSocket(i);

						if (pClient == NULL)
							printf("Got a TCP packet from non-existing client socket (%d), discarding.\n", i);
						else {
							snCurrentPacketSize += nbytes;
							// Check if received enough to check the packet size
							u_short snRealPacketSize = MAX_TCP_PACKET_SIZE;
							if (snCurrentPacketSize >= 2)
								snRealPacketSize = ntohs(*((short *)buf));
							if (snRealPacketSize > MAX_TCP_PACKET_SIZE) {		// Make sure the packet is not larger than allowed
								printf("Got a TCP packet that's larger than allowed.\n");
								snRealPacketSize = MAX_TCP_PACKET_SIZE;
							}
							// Received an entire packet
							while (snCurrentPacketSize >= snRealPacketSize)
							{
								// Process it
								CPacket oPacket(buf);
								if (!NetworkProcessTcpPacket(oPacket, pClient)) {
									printf("Couldn't process a TCP packet (type %d):\n", ntohs(*((u_short *)(buf + 2))));
									oPacket.Print(snRealPacketSize);
								}
								//memmove(buf, buf + snRealPacketSize, sizeof(buf) - snRealPacketSize);
								//snCurrentPacketSize -= snRealPacketSize;
								snCurrentPacketSize -= snRealPacketSize;
								memmove(buf, buf + snRealPacketSize,
									__min(snCurrentPacketSize, sizeof(buf) - snRealPacketSize));

								if (snCurrentPacketSize >= 2)
									snRealPacketSize = ntohs(*((short *)buf));
								else snRealPacketSize = MAX_TCP_PACKET_SIZE;
								if (snRealPacketSize > MAX_TCP_PACKET_SIZE) {		// Make sure the packet is not larger than allowed
									printf("Got a TCP packet that's larger than allowed.\n");
									snRealPacketSize = MAX_TCP_PACKET_SIZE;
								}
							}
						}
					}
				} // it's SO UGLY!
			}

			if (!bServerThreadRun) {
				break;
			}

			if (bGoToNextIt) ++it1;
		}

		// Sleep
		glfwSleep(0.0);
	}

	printf("Server thread has ended.\n");
	oServerThread = -1;
}

void ServerShutdownThread()
{
	if (oServerThread >= 0)
	{
		bServerThreadRun = false;

		// DEBUG: A hack to send ourselves an empty UDP packet in order to get out of select()
		struct sockaddr_in myaddr;
		myaddr.sin_family = AF_INET;
		myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		myaddr.sin_port = htons(DEFAULT_PORT);
		memset(myaddr.sin_zero, 0, sizeof(myaddr.sin_zero));
		sendto(nUdpSocket, NULL, 0, 0, (struct sockaddr *)&myaddr, sizeof(myaddr));

		shutdown(listener, SD_BOTH);
		shutdown(nUdpSocket, SD_BOTH);
	}
}

void ServerDestroyThread()
{
	if (oServerThread >= 0)
	{
		printf("Waiting for the server thread to complete.\n");
		glfwWaitThread(oServerThread, GLFW_WAIT);
		//glfwDestroyThread(oServerThread);
		oServerThread = -1;

		printf("Server thread has been destroyed.\n");
	}
}

// Shutdown the server
void ServerDeinit()
{
	ServerShutdownThread();

	// Close the connection to all clients
	ClientDeinit();

	ServerDestroyThread();

	// Close the server sockets
	NetworkCloseSocket(listener);
	NetworkCloseSocket(nUdpSocket);
}
