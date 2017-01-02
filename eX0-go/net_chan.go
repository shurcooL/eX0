// +build !js

package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"net"

	"github.com/shurcooL/eX0/eX0-go/packet"
)

// TCP and UDP via local channels.
type chanNetwork struct{}

func (nw chanNetwork) newConnection() *Connection {
	return &Connection{
		nw: nw,

		sendTCP: make(chan []byte, 128),
		recvTCP: make(chan []byte, 128),
		sendUDP: make(chan []byte, 128),
		recvUDP: make(chan []byte, 128),
	}
}

func (chanNetwork) dialServer(clientToServerConn *Connection) {
	components.server.chanListener <- clientToServerConn
	<-components.server.chanListenerReply
}

func (chanNetwork) dialedClient(_ *Connection) {}

func (chanNetwork) sendTCPPacketBytes(c *Connection, b []byte) error {
	c.sendTCP <- b
	return nil
}

func (chanNetwork) receiveTCPPacket(c *Connection) ([]byte, packet.TCPHeader, error) {
	b := <-c.recvTCP
	if len(b) < packet.TCPHeaderSize {
		return nil, packet.TCPHeader{}, fmt.Errorf("tcp packet size %v less than tcp header size %v", len(b), packet.TCPHeaderSize)
	}
	var tcpHeader packet.TCPHeader
	err := binary.Read(bytes.NewReader(b[:packet.TCPHeaderSize]), binary.BigEndian, &tcpHeader)
	if err != nil {
		return nil, packet.TCPHeader{}, err
	}
	if packet.TCPHeaderSize+tcpHeader.Length > packet.MAX_TCP_SIZE {
		return nil, packet.TCPHeader{}, fmt.Errorf("tcp packet size %v greater than max %v", packet.TCPHeaderSize+tcpHeader.Length, packet.MAX_TCP_SIZE)
	}
	return b[packet.TCPHeaderSize:], tcpHeader, nil
}

func (chanNetwork) sendUDPPacketBytes(c *Connection, b []byte) error {
	c.sendUDP <- b
	return nil
}

func (chanNetwork) receiveUDPPacket(c *Connection) ([]byte, packet.UDPHeader, error) {
	b := <-c.recvUDP
	var udpHeader packet.UDPHeader
	err := binary.Read(bytes.NewReader(b), binary.BigEndian, &udpHeader)
	if err != nil {
		return nil, packet.UDPHeader{}, err
	}
	return b[packet.UDPHeaderSize:], udpHeader, nil
}

func (chanNetwork) receiveUDPPacketFrom(_ *server, mux *Connection) ([]byte, packet.UDPHeader, *Connection, *net.UDPAddr, error) {
	b := <-mux.recvUDP
	var udpHeader packet.UDPHeader
	err := binary.Read(bytes.NewReader(b), binary.BigEndian, &udpHeader)
	if err != nil {
		return nil, packet.UDPHeader{}, nil, nil, err
	}
	return b[packet.UDPHeaderSize:], udpHeader, mux, nil, nil // HACK.
}
