package main

import (
	"fmt"

	"github.com/shurcooL/eX0/eX0-go/packet"
)

func sendTCPPacket(c *Connection, b []byte) error {
	// Validate packet size (for debugging).
	// TODO: Instead of setting manually and validating in debug mode, always set automatically here. It's not expensive if done after creating the entire packet, just edit the appropriate byte(s) in the header in the final []byte buffer just before sending over network.
	if debugValidation {
		if len(b) < packet.TCPHeaderSize {
			panic(fmt.Errorf("sendTCPPacket: smaller than packet.TCPHeaderSize: %v", b))
		}
		var tcpHeader packet.TCPHeader
		err := tcpHeader.UnmarshalBinary(b[:packet.TCPHeaderSize])
		if err != nil {
			panic(err)
		}
		if len(b)-packet.TCPHeaderSize != int(tcpHeader.Length) {
			panic(fmt.Errorf("sendTCPPacket: invalid payload size: %v %+v", len(b)-packet.TCPHeaderSize, tcpHeader))
		}
	}

	return sendTCPPacketValidated(c, b)
}

func receiveTCPPacket2(c *Connection, totalPlayerCount uint8) (interface{}, error) {
	b, tcpHeader, err := receiveTCPPacket(c)
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
	b, udpHeader, err := receiveUDPPacket(c)
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
