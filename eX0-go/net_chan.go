// +build chan

package main

import (
	"bytes"
	"io"
	"net"
)

type Connection struct {
	sendTcp chan []byte
	recvTcp chan []byte
	sendUdp chan []byte
	recvUdp chan []byte

	// Unused.
	tcp        net.Conn
	udp        *net.UDPConn
	JoinStatus JoinStatus
	UdpAddr    *net.UDPAddr

	// Common.
	Signature uint64
}

func sendTcpPacket(c *Connection, b []byte) error {
	c.sendTcp <- b
	return nil
}

func receiveTcpPacket(c *Connection) (io.Reader, error) {
	b := <-c.recvTcp
	return bytes.NewReader(b), nil
}

func sendUdpPacket(c *Connection, b []byte) error {
	c.sendUdp <- b
	return nil
}

func receiveUdpPacket(c *Connection) (io.Reader, error) {
	b := <-c.recvUdp
	return bytes.NewReader(b), nil
}

func receiveUdpPacketFrom(mux *Connection) (io.Reader, *Connection, *net.UDPAddr, error) {
	b := <-mux.recvUdp
	return bytes.NewReader(b), mux, nil, nil // HACK.
}
