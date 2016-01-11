// +build !chan,!tcp

package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"net"

	"github.com/shurcooL/eX0/eX0-go/packet"
)

// Normal TCP + UDP.

func newConnection() *Connection {
	return &Connection{}
}

func (clientToServerConn *Connection) dialServer() {
	tcp, err := net.Dial("tcp", *hostFlag+":25045")
	if err != nil {
		panic(err)
	}
	clientToServerConn.tcp = tcp

	udp, err := net.Dial("udp", *hostFlag+":25045")
	if err != nil {
		panic(err)
	}
	clientToServerConn.udp = udp.(*net.UDPConn)
}

func (_ *Connection) dialedClient() {
}

// Normal TCP + UDP. No need to handle UDP directly, since it will come in via the UDP mux.
const shouldHandleUDPDirectly = false

type Connection struct {
	tcp net.Conn
	udp *net.UDPConn

	// Connection to client.
	JoinStatus JoinStatus
	UDPAddr    *net.UDPAddr

	// Common.
	Signature uint64
	PlayerID  uint8 // TODO: Unsure if this should be here, experimental.

	// Unused.
	sendTCP chan []byte
	recvTCP chan []byte
	sendUDP chan []byte
	recvUDP chan []byte
}

func sendTCPPacket2(c *Connection, b []byte) error {
	_, err := c.tcp.Write(b)
	return err
}

func receiveTCPPacket(c *Connection) ([]byte, packet.TCPHeader, error) {
	var b = make([]byte, packet.MAX_TCP_SIZE)
	_, err := io.ReadFull(c.tcp, b[:packet.TCPHeaderSize])
	if err != nil {
		return nil, packet.TCPHeader{}, err
	}
	var tcpHeader packet.TCPHeader
	err = binary.Read(bytes.NewReader(b[:packet.TCPHeaderSize]), binary.BigEndian, &tcpHeader)
	if err != nil {
		return nil, packet.TCPHeader{}, err
	}
	if packet.TCPHeaderSize+tcpHeader.Length > packet.MAX_TCP_SIZE {
		return nil, packet.TCPHeader{}, fmt.Errorf("tcp packet size %v greater than max %v", packet.TCPHeaderSize+tcpHeader.Length, packet.MAX_TCP_SIZE)
	}
	_, err = io.ReadFull(c.tcp, b[packet.TCPHeaderSize:packet.TCPHeaderSize+tcpHeader.Length])
	if err != nil {
		return nil, packet.TCPHeader{}, err
	}
	return b[:packet.TCPHeaderSize+tcpHeader.Length], tcpHeader, nil
}

func sendUDPPacket(c *Connection, b []byte) error {
	if c.UDPAddr != nil {
		_, err := c.udp.WriteToUDP(b, c.UDPAddr)
		return err
	} else {
		_, err := c.udp.Write(b)
		return err
	}
}

func receiveUDPPacket(c *Connection) ([]byte, error) {
	var b [packet.MAX_UDP_SIZE]byte
	n, err := c.udp.Read(b[:])
	if err != nil {
		return nil, err
	}
	return b[:n], nil
}

func receiveUDPPacketFrom(s *server, mux *Connection) ([]byte, *Connection, *net.UDPAddr, error) {
	var b [packet.MAX_UDP_SIZE]byte
	n, udpAddr, err := mux.udp.ReadFromUDP(b[:])
	if err != nil {
		return nil, nil, nil, err
	}

	var from *Connection
	s.connectionsMu.Lock()
	for _, connection := range s.connections {
		if connection.UDPAddr != nil && udpAddrEqual(connection.UDPAddr, udpAddr) {
			from = connection
			break
		}
	}
	s.connectionsMu.Unlock()

	return b[:n], from, udpAddr, nil
}

// udpAddrEqual returns true if non-nil a and b are the same UDP address.
func udpAddrEqual(a, b *net.UDPAddr) bool {
	return a.IP.Equal(b.IP) &&
		a.Port == b.Port &&
		a.Zone == b.Zone
}
