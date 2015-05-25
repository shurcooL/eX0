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

func newConnection() *Connection {
	return &Connection{}
}

func (clientToServerConn *Connection) dialServer() {
	tcp, err := net.Dial("tcp", addr)
	if err != nil {
		panic(err)
	}
	clientToServerConn.tcp = tcp

	udp, err := net.Dial("udp", addr)
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

	// Unused.
	sendTcp chan []byte
	recvTcp chan []byte
	sendUdp chan []byte
	recvUdp chan []byte
}

func sendTcpPacket(c *Connection, b []byte) error {
	_, err := c.tcp.Write(b)
	return err
}

func receiveTcpPacket(c *Connection) (io.Reader, error) {
	const tcpHeaderSize = 3 // TODO: This shouldn't be hardcoded here.

	var b = make([]byte, packet.MAX_TCP_SIZE)
	_, err := io.ReadFull(c.tcp, b[:tcpHeaderSize])
	if err != nil {
		return nil, err
	}
	var tcpHeader packet.TcpHeader
	err = binary.Read(bytes.NewReader(b[:tcpHeaderSize]), binary.BigEndian, &tcpHeader)
	if err != nil {
		return nil, err
	}
	if tcpHeaderSize+tcpHeader.Length > packet.MAX_TCP_SIZE {
		return nil, fmt.Errorf("tcp packet size %v greater than max %v", tcpHeaderSize+tcpHeader.Length, packet.MAX_TCP_SIZE)
	}
	_, err = io.ReadFull(c.tcp, b[tcpHeaderSize:tcpHeaderSize+tcpHeader.Length])
	if err != nil {
		return nil, err
	}
	return bytes.NewReader(b[:tcpHeaderSize+tcpHeader.Length]), nil
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

func receiveUdpPacketFrom(mux *Connection) (io.Reader, *Connection, *net.UDPAddr, error) {
	var b [packet.MAX_UDP_SIZE]byte
	n, udpAddr, err := mux.udp.ReadFromUDP(b[:])
	if err != nil {
		return nil, nil, nil, err
	}

	var from *Connection
	state.Lock()
	for _, connection := range state.connections {
		if connection.UdpAddr != nil &&
			connection.UdpAddr.IP.Equal(udpAddr.IP) &&
			connection.UdpAddr.Port == udpAddr.Port &&
			connection.UdpAddr.Zone == udpAddr.Zone {

			from = connection
			break
		}
	}
	state.Unlock()

	return bytes.NewReader(b[:n]), from, udpAddr, nil
}
