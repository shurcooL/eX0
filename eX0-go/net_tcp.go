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

// Virtual TCP and UDP via physical TCP or WebSocket.
type tcpNetwork struct {
	// useWebSocket controls whether to use raw TCP or WebSocket
	// when client is dialing the server.
	useWebSocket bool
}

func (nw tcpNetwork) newConnection() *Connection {
	c := &Connection{
		nw: nw,

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

func (tn tcpNetwork) dialServer(clientToServerConn *Connection) {
	var (
		tcp net.Conn
		err error
	)
	switch tn.useWebSocket {
	case false:
		// Raw TCP connection.
		tcp, err = net.Dial("tcp", *hostFlag+":25046")
	case true:
		// WebSocket connection (TCP-like).
		var scheme string
		switch *secureFlag {
		case false:
			scheme = "ws"
		case true:
			scheme = "wss"
		}
		tcp, err = websocket.Dial(scheme+"://"+*hostFlag+":25047", "http://localhost/")
	}
	if err != nil {
		panic(err)
	}
	clientToServerConn.tcp = tcp
	close(clientToServerConn.start) // tcp-specific.
}

func (tcpNetwork) dialedClient(c *Connection) {
	close(c.start)
}

func (tcpNetwork) sendTCPPacketBytes(c *Connection, b []byte) error {
	c.sendTCP <- b
	return nil
}

func (tcpNetwork) receiveTCPPacket(c *Connection) ([]byte, packet.TCPHeader, error) {
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
	return b[packet.TCPHeaderSize:], tcpHeader, nil
}

func (tcpNetwork) sendUDPPacketBytes(c *Connection, b []byte) error {
	c.sendUDP <- b
	return nil
}

func (tcpNetwork) receiveUDPPacket(c *Connection) ([]byte, packet.UDPHeader, error) {
	b, ok := <-c.recvUDP
	if !ok {
		return nil, packet.UDPHeader{}, errors.New("conn prob")
	}
	var udpHeader packet.UDPHeader
	err := binary.Read(bytes.NewReader(b), binary.BigEndian, &udpHeader)
	if err != nil {
		return nil, packet.UDPHeader{}, err
	}
	return b[packet.UDPHeaderSize:], udpHeader, nil
}

func (tcpNetwork) receiveUDPPacketFrom(_ *server, mux *Connection) ([]byte, packet.UDPHeader, *Connection, *net.UDPAddr, error) {
	b, ok := <-mux.recvUDP
	if !ok {
		return nil, packet.UDPHeader{}, nil, nil, errors.New("conn prob")
	}
	var udpHeader packet.UDPHeader
	err := binary.Read(bytes.NewReader(b), binary.BigEndian, &udpHeader)
	if err != nil {
		return nil, packet.UDPHeader{}, nil, nil, err
	}
	return b[packet.UDPHeaderSize:], udpHeader, mux, nil, nil // HACK.
}
