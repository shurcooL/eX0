// +build tcp,!chan

package main

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"net"

	"github.com/goxjs/websocket"

	"github.com/shurcooL/eX0/eX0-go/packet"
)

// Virtual TCP and UDP via physical TCP. Requires `-tags=tcp`.
// dialServer can be controlled to use raw TCP or WebSocket.

func newConnection() *Connection {
	c := &Connection{
		sendTCP: make(chan []byte),
		recvTCP: make(chan []byte, 128),
		sendUDP: make(chan []byte),
		recvUDP: make(chan []byte, 128),

		start: make(chan struct{}),
	}

	// Send.
	go func(c *Connection) {
		<-c.start
		for {
			select {
			case b := <-c.sendTCP:
				_, err := c.tcp.Write(append([]byte{0, 0}, b...))
				_ = err
			case b := <-c.sendUDP:
				var buf = bytes.NewBuffer(make([]byte, 0, 2))
				err := binary.Write(buf, binary.LittleEndian, uint16(len(b)))
				if err != nil {
					panic(err)
				}

				_, err = c.tcp.Write(append(buf.Bytes(), b...))
				_ = err
			}
		}
	}(c)
	// Read.
	go func(c *Connection) {
		<-c.start
		for {
			var udpSize uint16
			err := binary.Read(c.tcp, binary.LittleEndian, &udpSize)
			if err != nil {
				close(c.recvTCP)
				close(c.recvUDP)
				return
			}

			if udpSize == 0 {
				var b = make([]byte, packet.MAX_TCP_SIZE)
				_, err := io.ReadFull(c.tcp, b[:packet.TCPHeaderSize])
				if err != nil {
					close(c.recvTCP)
					close(c.recvUDP)
					return
				}
				var tcpHeader packet.TCPHeader
				err = binary.Read(bytes.NewReader(b[:packet.TCPHeaderSize]), binary.BigEndian, &tcpHeader)
				if err != nil {
					close(c.recvTCP)
					close(c.recvUDP)
					return
				}
				if packet.TCPHeaderSize+tcpHeader.Length > packet.MAX_TCP_SIZE {
					close(c.recvTCP)
					close(c.recvUDP)
					return
				}
				_, err = io.ReadFull(c.tcp, b[packet.TCPHeaderSize:packet.TCPHeaderSize+tcpHeader.Length])
				if err != nil {
					close(c.recvTCP)
					close(c.recvUDP)
					return
				}
				c.recvTCP <- b[:packet.TCPHeaderSize+tcpHeader.Length]
			} else {
				var b = make([]byte, udpSize, udpSize)
				_, err := io.ReadFull(c.tcp, b)
				if err != nil {
					close(c.recvTCP)
					close(c.recvUDP)
					return
				}
				c.recvUDP <- b
			}
		}
	}(c)

	return c
}

func (clientToServerConn *Connection) dialServer() {
	var (
		tcp net.Conn
		err error
	)
	switch 1 {
	case 0:
		// TCP connection.
		tcp, err = net.Dial("tcp", *hostFlag+":25045")
	case 1:
		// WebSocket connection.
		tcp, err = websocket.Dial("ws://"+*hostFlag+":25046", "http://localhost/")
	default:
		panic("invalid choice")
	}
	if err != nil {
		panic(err)
	}
	clientToServerConn.tcp = tcp
	close(clientToServerConn.start) // tcp-specific.
}

func (c *Connection) dialedClient() {
	close(c.start)
}

// tcp-specific. Need to handle UDP directly on same connection, since there won't be a separate one.
const shouldHandleUDPDirectly = true

type Connection struct {
	tcp net.Conn

	// Connection to client.
	JoinStatus JoinStatus

	// Common.
	Signature uint64
	PlayerID  uint8 // TODO: Unsure if this should be here, experimental.

	sendTCP chan []byte
	recvTCP chan []byte
	sendUDP chan []byte
	recvUDP chan []byte

	start chan struct{}

	// Unused.
	udp     *net.UDPConn
	UDPAddr *net.UDPAddr
}

func sendTCPPacket2(c *Connection, b []byte) error {
	c.sendTCP <- b
	return nil
}

func receiveTCPPacket(c *Connection) (io.Reader, packet.TCPHeader, error) {
	b, ok := <-c.recvTCP
	if !ok {
		return nil, packet.TCPHeader{}, errors.New("conn prob")
	}
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
	return bytes.NewReader(b), tcpHeader, nil
}

func sendUDPPacket(c *Connection, b []byte) error {
	c.sendUDP <- b
	return nil
}

func receiveUDPPacket(c *Connection) (io.Reader, error) {
	b, ok := <-c.recvUDP
	if !ok {
		return nil, errors.New("conn prob")
	}
	return bytes.NewReader(b), nil
}

func receiveUDPPacketFrom(_ *server, mux *Connection) (io.Reader, *Connection, *net.UDPAddr, error) {
	b, ok := <-mux.recvUDP
	if !ok {
		return nil, nil, nil, errors.New("conn prob")
	}
	return bytes.NewReader(b), mux, nil, nil // HACK.
}
