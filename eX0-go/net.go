package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"net"

	"github.com/shurcooL/eX0/eX0-go/packet"
)

type Connection struct {
	// Server.
	tcp net.Conn
	udp net.Conn

	// Client.
	JoinStatus JoinStatus
	UdpAddr    *net.UDPAddr

	// Common.
	Signature uint64
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
	_, err := c.udp.Write(b)
	return err
}

func receiveUdpPacket(c *Connection) (io.Reader, error) {
	var b [packet.MAX_UDP_SIZE]byte
	n, err := c.udp.Read(b[:])
	if err != nil {
		return nil, err
	}
	return bytes.NewReader(b[:n]), nil
}
