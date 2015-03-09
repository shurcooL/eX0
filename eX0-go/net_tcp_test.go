// +build tcp,!chan

package main

import (
	"fmt"
	"net"
)

func testFullConnection() {
	// Virtual TCP and UDP via physical TCP. Requires `go test -tags=tcp`.

	c := make(chan *Connection)
	go func(c chan *Connection) {
		var serverToClientConn = newConnection()
		ln, err := net.Listen("tcp", "localhost:25045")
		if err != nil {
			panic(err)
		}
		tcp, err := ln.Accept()
		if err != nil {
			panic(err)
		}
		serverToClientConn.tcp = tcp
		close(serverToClientConn.start)
		c <- serverToClientConn
	}(c)

	s := make(chan *Connection)
	go func(s chan *Connection) {
		var clientToServerConn = newConnection()
		tcp, err := net.Dial("tcp", "localhost:25045")
		if err != nil {
			panic(err)
		}
		clientToServerConn.tcp = tcp
		close(clientToServerConn.start)
		s <- clientToServerConn
	}(s)

	var clientToServerConn = <-s
	var serverToClientConn = <-c

	state.mu.Lock()
	state.connections = append(state.connections, serverToClientConn)
	state.mu.Unlock()

	go handleTcpConnection(serverToClientConn)
	go handleUdp(serverToClientConn)
	go sendServerUpdates()
	go broadcastPingPacket()
	fmt.Println("Started.")

	connectToServer(clientToServerConn, nil)
}
