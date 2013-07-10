#include "globals.h"

#pragma comment(linker, "/NODEFAULTLIB:\"LIBCMT\"")

// initialization
bool Init(int argc, char *argv[])
{
	// init glfw
	//if (!glfwInit())
	//	return false;

	// Initialize components
	nPlayerCount = 2; PlayerInit();		// Initialize the players
	if (!NetworkInit())					// Initialize the networking
		return false;

	return true;
}

// deinitialization
void Deinit()
{
	NetworkDeinit();					// Shutdown the networking component
	nPlayerCount = 0; PlayerInit();		// delete all players

	// terminate glfw
	//glfwTerminate();
}

void printx(char x[22])
{
	printf("packet: ");
	for (int i = 0; i < 22; ++i)
	{
		if (i < 6)
			printf("%d ", (int)x[i]);
		else
			printf("%c ", x[i]);
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	fd_set master;   // master file descriptor list
	fd_set read_fds; // temp file descriptor list for select()
	list<SOCKET>	oActiveSockets;
	struct sockaddr_in myaddr;	 // server address
	struct sockaddr_in remoteaddr; // client address
	int fdmax;		// maximum file descriptor number, for Linux select()
	SOCKET listener;	 // listening socket descriptor
	SOCKET newfd;		// newly accept()ed socket descriptor
	char buf[2 * MAX_TCP_PACKET_SIZE];	// Buffer for incoming packets
	int nYes = 1;		// for setsockopt() SO_REUSEADDR, below
	int nbytes;
	socklen_t addrlen;
	SOCKET i;//, j;
	short		snCurrentPacketSize = 0;

	printf("Max size of tcp packet: %d\nReal size: %d\nJoin accept packet size: %d\n",
		MAX_TCP_PACKET_SIZE, sizeof(struct TcpPacket_t), sizeof(struct TcpPacketJoinGameAccept_t));

	// Initialize the networking component
	if (!Init(argc, argv))
		exit(1);

	FD_ZERO(&master);	// clear the master and temp sets
	FD_ZERO(&read_fds);
	oActiveSockets.clear();

	// Create the listener socket
	if ((listener = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		NetworkPrintError("socket");
		exit(1);
	}
	printf("Created listener socket #%d.\n", (int)listener);

	// lose the pesky "address already in use" error message
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *)&nYes, sizeof(nYes)) == SOCKET_ERROR) {
		NetworkPrintError("setsockopt");
		exit(1);
	}
	// Disable the Nagle algorithm for send coalescing
	if (setsockopt(listener, IPPROTO_TCP, TCP_NODELAY, (char *)&nYes, sizeof(nYes)) == SOCKET_ERROR) {
		NetworkPrintError("setsockopt(nodelay)");
		exit(1);
	}

	// bind
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(DEFAULT_PORT);
	memset(myaddr.sin_zero, 0, sizeof(myaddr.sin_zero));
	if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == SOCKET_ERROR) {
		NetworkPrintError("bind");
		exit(1);
	}

	// listen
	if (listen(listener, 10) == SOCKET_ERROR) {
		NetworkPrintError("listen");
		exit(1);
	}

	// add the listener to the master set
	FD_SET(listener, &master);
	oActiveSockets.push_back(listener);

	// keep track of the biggest file descriptor
	fdmax = (int)listener; // so far, it's this one

	// Main loop
	while (true)
	{
		read_fds = master; // copy it
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == SOCKET_ERROR) {
			NetworkPrintError("select");
			exit(1);
		}

		// run through the existing connections looking for data to read
		//for (int nLoop1 = 0; nLoop1 < (int)read_fds.fd_count; ++nLoop1) {
		//for (int nLoop1 = -1; nLoop1 < nPlayerCount; ++nLoop1) {
		for (list<SOCKET>::iterator it1 = oActiveSockets.begin(); it1 != oActiveSockets.end(); )
		{
			bool bGoToNextIt = true;

			/*if (nLoop1 == -1) i = (int)listener;
			else i = (int)PlayerGet(nLoop1)->nTcpSocket;*/
			i = *it1;

			if (FD_ISSET(i, &read_fds)) { // we got one!!
				if (i == listener) {
					// handle new connections
					addrlen = sizeof(remoteaddr);
					if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr,
														&addrlen)) == INVALID_SOCKET) {
						NetworkPrintError("accept");
					} else {
						FD_SET(newfd, &master); // add to master set
						oActiveSockets.push_back((SOCKET)newfd);
						if ((int)newfd > fdmax) {	// keep track of the maximum
							fdmax = (int)newfd;
						}
						printf("selectserver: new connection from %s on socket %d\n",
							inet_ntoa(remoteaddr.sin_addr), (int)newfd);

						// Disable the Nagle algorithm for send coalescing
						if (setsockopt(newfd, IPPROTO_TCP, TCP_NODELAY, (char *)&nYes, sizeof(nYes)) == SOCKET_ERROR) {
							NetworkPrintError("setsockopt(nodelay newsock)");
							exit(1);
						}
					}
				} else {
					// handle data from a client
					if ((nbytes = recv(i, buf + snCurrentPacketSize, sizeof(buf) - snCurrentPacketSize, 0)) <= 0) {
						// got error or connection closed by client
						if (nbytes == 0) {
							// connection closed
							printf("selectserver: socket %d hung up\n", (int)i);

							// Remove the player, if he's already connected
							if (PlayerGetFromSocket(i) != NULL)
								PlayerGetFromSocket(i)->bConnected = false;
						} else {
							NetworkPrintError("recv");

							// connection closed
							printf("selectserver: socket %d terminated hard (error), removing player\n", (int)i);

							// Remove the player, if he's already connected
							if (PlayerGetFromSocket(i) != NULL)
								PlayerGetFromSocket(i)->bConnected = false;
						}
						NetworkCloseSocket(i); // bye!
						FD_CLR(i, &master); // remove from master set
						it1 = oActiveSockets.erase(it1);
						bGoToNextIt = false;
					} else {
						// we got some data from a client, process it
						//printf("Got %d bytes from client #%d\n", nbytes, (int)i);

						snCurrentPacketSize += nbytes;
						short snRealPacketSize = MAX_TCP_PACKET_SIZE;
						if (snCurrentPacketSize >= 2)
							snRealPacketSize = ntohs(((struct TcpPacket_t *)buf)->snPacketSize);
						// Check if received a full maximum-length packet
						// or enough to check the packet size
						while (snCurrentPacketSize >= snRealPacketSize
							&& snRealPacketSize <= MAX_TCP_PACKET_SIZE)		// Make sure the packet is not larger than allowed
						{
							// Received the entire packet, process it
							//printx(buf);
							if (!NetworkProcessPacket((struct TcpPacket_t *)buf, i))
								printf("Couldn't process a packet.\n");
							memmove(buf, buf + snRealPacketSize, sizeof(buf) - snRealPacketSize);
							snCurrentPacketSize -= snRealPacketSize;

							if (snCurrentPacketSize >= 2)
								snRealPacketSize = ntohs(((struct TcpPacket_t *)buf)->snPacketSize);
							else snRealPacketSize = MAX_TCP_PACKET_SIZE;
						}

						// send to everyone!
						//for (int nLoop2 = 0; nLoop2 < (int)master.fd_count; ++nLoop2) {
						//for (int nLoop2 = 0; nLoop2 < nPlayerCount; ++nLoop2) {
						/*for (list<SOCKET>::iterator it2 = oActiveSockets.begin(); it2 != oActiveSockets.end(); ++it2)
						{
							//j = (int)PlayerGet(nLoop2)->nTcpSocket;
							j = *it2;

							// except the listener and ourselves
							if (j != listener && j != i) {
								if (send(j, buf, nbytes, 0) == SOCKET_ERROR) {
									NetworkPrintError("send");
								}
							}
						}*/
					}
				} // it's SO UGLY!
			}

			if (bGoToNextIt) ++it1;
		}

		//Sleep(0);
	}

	// Deinit
	Deinit();

	return 0;
}
