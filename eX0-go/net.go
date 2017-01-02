package main

import (
	"encoding"
	"fmt"
	"net"

	"github.com/shurcooL/eX0/eX0-go/packet"
)

const commandRate = 20

// network is the mechanism by which connections are established and
// individual packets are transported.
type network interface {
	newConnection() *Connection

	dialServer(*Connection)
	dialedClient(*Connection)

	sendTCPPacketBytes(c *Connection, b []byte) error
	receiveTCPPacket(c *Connection) ([]byte, packet.TCPHeader, error)
	sendUDPPacketBytes(c *Connection, b []byte) error
	receiveUDPPacket(c *Connection) ([]byte, packet.UDPHeader, error)
	receiveUDPPacketFrom(s *server, mux *Connection) ([]byte, packet.UDPHeader, *Connection, *net.UDPAddr, error)
}

// Connection is a structure that holds data relevant to a connection.
type Connection struct {
	nw network

	// TCP+UDP-only.
	tcp net.Conn
	udp *net.UDPConn

	// TCP+UDP-only.
	// Connection to client.
	JoinStatus JoinStatus
	UDPAddr    *net.UDPAddr

	// udp and UDPAddr are unused by TCP-only.

	// Common.
	Signature uint64
	PlayerID  uint8 // TODO: Unsure if this should be here, experimental.

	// chan-only.
	sendTCP chan []byte
	recvTCP chan []byte
	sendUDP chan []byte
	recvUDP chan []byte

	// TCP-only.
	start chan struct{}
}

func sendTCPPacket(c *Connection, p encoding.BinaryMarshaler) error {
	b, err := p.MarshalBinary()
	if err != nil {
		return err
	}
	return c.nw.sendTCPPacketBytes(c, b)
}

func sendUDPPacket(c *Connection, p encoding.BinaryMarshaler) error {
	b, err := p.MarshalBinary()
	if err != nil {
		return err
	}
	return c.nw.sendUDPPacketBytes(c, b)
}

func broadcastTCPPacket(cs []*Connection, p encoding.BinaryMarshaler) error {
	b, err := p.MarshalBinary()
	if err != nil {
		return err
	}
	for _, c := range cs {
		err = c.nw.sendTCPPacketBytes(c, b)
		if err != nil {
			// TODO: This error handling is wrong. If fail to send to one client, should still send to others, etc.
			return err
		}
	}
	return nil
}

func receiveTCPPacket2(c *Connection, totalPlayerCount uint8) (interface{}, error) {
	b, tcpHeader, err := c.nw.receiveTCPPacket(c)
	if err != nil {
		return nil, err
	}

	switch tcpHeader.Type {
	case packet.JoinServerRequestType:
		var p = packet.JoinServerRequest{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.JoinServerAcceptType:
		var p = packet.JoinServerAccept{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.JoinServerRefuseType:
		var p = packet.JoinServerRefuse{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.UDPConnectionEstablishedType:
		var p = packet.UDPConnectionEstablished{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.LoadLevelType:
		var p = packet.LoadLevel{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.CurrentPlayersInfoType:
		var p = packet.CurrentPlayersInfo{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b, totalPlayerCount)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.LocalPlayerInfoType:
		var p = packet.LocalPlayerInfo{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.EnterGamePermissionType:
		var p = packet.EnterGamePermission{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.EnteredGameNotificationType:
		var p = packet.EnteredGameNotification{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.JoinTeamRequestType:
		var p = packet.JoinTeamRequest{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.PlayerJoinedServerType:
		var p = packet.PlayerJoinedServer{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.PlayerLeftServerType:
		var p = packet.PlayerLeftServer{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.PlayerJoinedTeamType:
		var p = packet.PlayerJoinedTeam{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.PlayerWasHitType:
		var p = packet.PlayerWasHit{TCPHeader: tcpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	default:
		return nil, fmt.Errorf("invalid TCP packet type: %v", tcpHeader.Type)
	}
}

func receiveUDPPacket2(c *Connection, totalPlayerCount uint8) (interface{}, error) {
	b, udpHeader, err := c.nw.receiveUDPPacket(c)
	if err != nil {
		return nil, err
	}

	switch udpHeader.Type {
	case packet.TimeResponseType:
		var p = packet.TimeResponse{UDPHeader: udpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.PingType:
		var p = packet.Ping{UDPHeader: udpHeader}
		err = p.UnmarshalBinary(b, totalPlayerCount)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.PungType:
		var p = packet.Pung{UDPHeader: udpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, err
		}
		return p, nil
	case packet.ServerUpdateType:
		var p = packet.ServerUpdate{UDPHeader: udpHeader}
		err = p.UnmarshalBinary(b, totalPlayerCount)
		if err != nil {
			return nil, err
		}
		return p, nil
	default:
		return nil, fmt.Errorf("invalid UDP packet type: %v", udpHeader.Type)
	}
}

func receiveUDPPacketFrom2(s *server, mux *Connection, totalPlayerCount uint8) (interface{}, *Connection, *net.UDPAddr, error) {
	b, udpHeader, c, udpAddr, err := mux.nw.receiveUDPPacketFrom(s, mux)
	if err != nil {
		return nil, nil, nil, err
	}

	if c == nil && udpHeader.Type != packet.HandshakeType {
		return nil, nil, nil, fmt.Errorf("nil c, unexpected udpHeader.Type: %v", udpHeader.Type)
	}

	// TODO: Dedup with receiveUDPPacket2?
	switch udpHeader.Type {
	case packet.HandshakeType:
		var p = packet.Handshake{UDPHeader: udpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, nil, nil, err
		}
		return p, c, udpAddr, nil
	case packet.TimeRequestType:
		var p = packet.TimeRequest{UDPHeader: udpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, nil, nil, err
		}
		return p, c, udpAddr, nil
	case packet.TimeResponseType:
		var p = packet.TimeResponse{UDPHeader: udpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, nil, nil, err
		}
		return p, c, udpAddr, nil
	case packet.PingType:
		var p = packet.Ping{UDPHeader: udpHeader}
		err = p.UnmarshalBinary(b, totalPlayerCount)
		if err != nil {
			return nil, nil, nil, err
		}
		return p, c, udpAddr, nil
	case packet.PongType:
		var p = packet.Pong{UDPHeader: udpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, nil, nil, err
		}
		return p, c, udpAddr, nil
	case packet.PungType:
		var p = packet.Pung{UDPHeader: udpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, nil, nil, err
		}
		return p, c, udpAddr, nil
	case packet.ClientCommandType:
		var p = packet.ClientCommand{UDPHeader: udpHeader}
		err = p.UnmarshalBinary(b)
		if err != nil {
			return nil, nil, nil, err
		}
		return p, c, udpAddr, nil
	case packet.ServerUpdateType:
		var p = packet.ServerUpdate{UDPHeader: udpHeader}
		err = p.UnmarshalBinary(b, totalPlayerCount)
		if err != nil {
			return nil, nil, nil, err
		}
		return p, c, udpAddr, nil
	default:
		return nil, nil, nil, fmt.Errorf("invalid UDP packet type: %v", udpHeader.Type)
	}
}
