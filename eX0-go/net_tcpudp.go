// +build !js

package main

import (
	"fmt"
	"io"
	"net"

	"github.com/shurcooL/eX0/eX0-go/packet"
)

// Normal TCP + UDP.
type tcpUDPNetwork struct{}

func (nw tcpUDPNetwork) newConnection() *Connection {
	return &Connection{nw: nw}
}

func (tcpUDPNetwork) dialServer(clientToServerConn *Connection) {
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

func (tcpUDPNetwork) dialedClient(_ *Connection) {}

func (tcpUDPNetwork) sendTCPPacketBytes(c *Connection, b []byte) error {
	_, err := c.tcp.Write(b)
	return err
}

func (tcpUDPNetwork) receiveTCPPacket(c *Connection) ([]byte, packet.TCPHeader, error) {
	var b = make([]byte, packet.MAX_TCP_SIZE)
	_, err := io.ReadFull(c.tcp, b[:packet.TCPHeaderSize])
	if err != nil {
		return nil, packet.TCPHeader{}, err
	}
	var tcpHeader packet.TCPHeader
	err = tcpHeader.UnmarshalBinary(b[:packet.TCPHeaderSize])
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
	return b[packet.TCPHeaderSize : packet.TCPHeaderSize+tcpHeader.Length], tcpHeader, nil
}

func (tcpUDPNetwork) sendUDPPacketBytes(c *Connection, b []byte) error {
	if c.UDPAddr != nil {
		_, err := c.udp.WriteToUDP(b, c.UDPAddr)
		return err
	} else {
		_, err := c.udp.Write(b)
		return err
	}
}

func (tcpUDPNetwork) receiveUDPPacket(c *Connection) ([]byte, packet.UDPHeader, error) {
	var b [packet.MAX_UDP_SIZE]byte
	n, err := c.udp.Read(b[:])
	if err != nil {
		return nil, packet.UDPHeader{}, err
	}
	var udpHeader packet.UDPHeader
	err = udpHeader.UnmarshalBinary(b[:packet.UDPHeaderSize])
	if err != nil {
		return nil, packet.UDPHeader{}, err
	}
	return b[packet.UDPHeaderSize:n], udpHeader, nil
}

func (tcpUDPNetwork) receiveUDPPacketFrom(s *server, mux *Connection) ([]byte, packet.UDPHeader, *Connection, *net.UDPAddr, error) {
	var b [packet.MAX_UDP_SIZE]byte
	n, udpAddr, err := mux.udp.ReadFromUDP(b[:])
	if err != nil {
		return nil, packet.UDPHeader{}, nil, nil, err
	}
	var udpHeader packet.UDPHeader
	err = udpHeader.UnmarshalBinary(b[:packet.UDPHeaderSize])
	if err != nil {
		return nil, packet.UDPHeader{}, nil, nil, err
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

	return b[packet.UDPHeaderSize:n], udpHeader, from, udpAddr, nil
}

// udpAddrEqual returns true if non-nil a and b are the same UDP address.
func udpAddrEqual(a, b *net.UDPAddr) bool {
	return a.IP.Equal(b.IP) &&
		a.Port == b.Port &&
		a.Zone == b.Zone
}
