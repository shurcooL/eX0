// +build chan,!tcp

package main

import (
	"fmt"
	"time"
)

// TCP and UDP via local channels. Requires `go test -tags=chan`.
func testFullConnection() {
	clientToServerTcp := make(chan []byte)
	serverToClientTcp := make(chan []byte)
	clientToServerUdp := make(chan []byte)
	serverToClientUdp := make(chan []byte)

	var clientToServerConn = &Connection{
		sendTcp: clientToServerTcp,
		recvTcp: serverToClientTcp,
		sendUdp: clientToServerUdp,
		recvUdp: serverToClientUdp,
	}

	var serverToClientConn = &Connection{
		sendTcp: serverToClientTcp,
		recvTcp: clientToServerTcp,
		sendUdp: serverToClientUdp,
		recvUdp: clientToServerUdp,
	}

	state.mu.Lock()
	state.connections = append(state.connections, serverToClientConn)
	state.mu.Unlock()

	go handleTcpConnection(serverToClientConn)
	go handleUdp(serverToClientConn)
	go sendServerUpdates()
	go broadcastPingPacket()
	fmt.Println("Started.")

	connectToServer(clientToServerConn, false)
	time.Sleep(10 * time.Second) // Wait 10 seconds before exiting.
}
