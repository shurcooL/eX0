// +build chan,!tcp

package main

import (
	"bytes"
	"io"
	"net"
)

func newConnection() *Connection {
	return &Connection{
		sendTcp: make(chan []byte, 128),
		recvTcp: make(chan []byte, 128),
		sendUdp: make(chan []byte, 128),
		recvUdp: make(chan []byte, 128),
	}
}

func (clientToServerConn *Connection) dialServer() {
	chanListener <- clientToServerConn
	<-chanListenerReply
}

func (_ *Connection) dialedClient() {
}

func (c *Connection) maybeHandleUdpDirectly() {
	// chan-specific. Need to handle UDP directly on same connection, since there won't be a separate one.
	go handleUdp(c)
}

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
