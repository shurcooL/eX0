// +build tcp,!chan

package main

import (
	"fmt"
	"net"
	"time"
)

// Virtual TCP and UDP via physical TCP. Requires `go test -tags=tcp`.
func testFullConnection() {
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

	state.Lock()
	state.connections = append(state.connections, serverToClientConn)
	state.Unlock()

	go handleTcpConnection(serverToClientConn)
	go handleUdp(serverToClientConn)
	go sendServerUpdates()
	go broadcastPingPacket()
	fmt.Println("Started.")

	connectToServer(clientToServerConn, false)
	time.Sleep(10 * time.Second) // Wait 10 seconds before exiting.
}
