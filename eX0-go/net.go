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
const shouldHandleUdpDirectly = false

type Connection struct {
	tcp net.Conn
	udp *net.UDPConn

	// Connection to client.
	JoinStatus JoinStatus
	UdpAddr    *net.UDPAddr

	// Common.
	Signature uint64
	PlayerId  uint8 // TODO: Unsure if this should be here, experimental.

	// Unused.
	sendTcp chan []byte
	recvTcp chan []byte
	sendUdp chan []byte
	recvUdp chan []byte
}

func sendTcpPacket2(c *Connection, b []byte) error {
	_, err := c.tcp.Write(b)
	return err
}

func receiveTcpPacket(c *Connection) (io.Reader, packet.TcpHeader, error) {
	var b = make([]byte, packet.MAX_TCP_SIZE)
	_, err := io.ReadFull(c.tcp, b[:packet.TcpHeaderSize])
	if err != nil {
		return nil, packet.TcpHeader{}, err
	}
	var tcpHeader packet.TcpHeader
	err = binary.Read(bytes.NewReader(b[:packet.TcpHeaderSize]), binary.BigEndian, &tcpHeader)
	if err != nil {
		return nil, packet.TcpHeader{}, err
	}
	if packet.TcpHeaderSize+tcpHeader.Length > packet.MAX_TCP_SIZE {
		return nil, packet.TcpHeader{}, fmt.Errorf("tcp packet size %v greater than max %v", packet.TcpHeaderSize+tcpHeader.Length, packet.MAX_TCP_SIZE)
	}
	_, err = io.ReadFull(c.tcp, b[packet.TcpHeaderSize:packet.TcpHeaderSize+tcpHeader.Length])
	if err != nil {
		return nil, packet.TcpHeader{}, err
	}
	return bytes.NewReader(b[:packet.TcpHeaderSize+tcpHeader.Length]), tcpHeader, nil
}

func sendUdpPacket(c *Connection, b []byte) error {
	if c.UdpAddr != nil {
		_, err := c.udp.WriteToUDP(b, c.UdpAddr)
		return err
	} else {
		_, err := c.udp.Write(b)
		return err
	}
}

func receiveUdpPacket(c *Connection) (io.Reader, error) {
	var b [packet.MAX_UDP_SIZE]byte
	n, err := c.udp.Read(b[:])
	if err != nil {
		return nil, err
	}
	return bytes.NewReader(b[:n]), nil
}

func receiveUdpPacketFrom(s *server, mux *Connection) (io.Reader, *Connection, *net.UDPAddr, error) {
	var b [packet.MAX_UDP_SIZE]byte
	n, udpAddr, err := mux.udp.ReadFromUDP(b[:])
	if err != nil {
		return nil, nil, nil, err
	}

	var from *Connection
	s.connectionsMu.Lock()
	for _, connection := range s.connections {
		if connection.UdpAddr != nil && udpAddrEqual(connection.UdpAddr, udpAddr) {
			from = connection
			break
		}
	}
	s.connectionsMu.Unlock()

	return bytes.NewReader(b[:n]), from, udpAddr, nil
}

// udpAddrEqual returns true if non-nil a and b are the same UDP address.
func udpAddrEqual(a, b *net.UDPAddr) bool {
	return a.IP.Equal(b.IP) &&
		a.Port == b.Port &&
		a.Zone == b.Zone
}
