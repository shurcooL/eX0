// Package packet is for TCP and UDP packets used in eX0 networking protocol.
package packet

import (
	"bytes"
	"encoding/binary"
	"fmt"
)

const MAX_TCP_SIZE = 1448
const MAX_UDP_SIZE = 1448

type Type uint8

const (
	// TCP packet types.
	JoinServerRequestType        Type = 1
	JoinServerAcceptType         Type = 2
	JoinServerRefuseType         Type = 3
	UDPConnectionEstablishedType Type = 5
	EnterGamePermissionType      Type = 6
	EnteredGameNotificationType  Type = 7
	LoadLevelType                Type = 20
	CurrentPlayersInfoType       Type = 21
	PlayerJoinedServerType       Type = 25
	PlayerLeftServerType         Type = 26
	JoinTeamRequestType          Type = 27
	PlayerJoinedTeamType         Type = 28
	LocalPlayerInfoType          Type = 30
	PlayerWasHitType             Type = 40

	// UDP packet types.
	ClientCommandType Type = 1
	ServerUpdateType  Type = 2
	PingType          Type = 10
	PongType          Type = 11
	PungType          Type = 12
	HandshakeType     Type = 100
	TimeRequestType   Type = 105
	TimeResponseType  Type = 106
)

//go:generate stringer -type=Type

type TCPHeader struct {
	Length uint16
	Type   Type
}

// TCPHeaderSize is the size in bytes of the TCP packet header.
const TCPHeaderSize = 3

func init() {
	if TCPHeaderSize != binary.Size(TCPHeader{}) {
		panic("TCPHeaderSize != binary.Size(TCPHeader{})")
	}
}

func (h *TCPHeader) marshalBinary() []byte {
	b := make([]byte, TCPHeaderSize)
	b[0] = byte(h.Length >> 8)
	b[1] = byte(h.Length)
	b[2] = byte(h.Type)
	return b
}
func (h *TCPHeader) UnmarshalBinary(b []byte) error {
	// TODO: Should this be != or <? Find out how UnmarshalBinary is supposed to handle not enough/too much data.
	if len(b) != TCPHeaderSize {
		return fmt.Errorf("TCPHeader has unexpected size of %v instead of %v", len(b), TCPHeaderSize)
	}
	h.Length = uint16(b[0])<<8 | uint16(b[1])
	h.Type = Type(b[2])
	return nil
}

type JoinServerRequest struct {
	TCPHeader

	Version    uint16
	Passphrase [16]byte
	Signature  uint64
}

func (p *JoinServerRequest) MarshalBinary() ([]byte, error) {
	p.Type = JoinServerRequestType
	p.Length = 26
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.Version)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.Passphrase)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.Signature)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *JoinServerRequest) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.Version)
	if err != nil {
		return err
	}
	err = binary.Read(buf, binary.BigEndian, &p.Passphrase)
	if err != nil {
		return err
	}
	err = binary.Read(buf, binary.BigEndian, &p.Signature)
	if err != nil {
		return err
	}
	return nil
}

type JoinServerAccept struct {
	TCPHeader

	YourPlayerID     uint8
	TotalPlayerCount uint8
}

func (p *JoinServerAccept) MarshalBinary() ([]byte, error) {
	p.Type = JoinServerAcceptType
	p.Length = 2
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.YourPlayerID)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.TotalPlayerCount)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *JoinServerAccept) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.YourPlayerID)
	if err != nil {
		return err
	}
	err = binary.Read(buf, binary.BigEndian, &p.TotalPlayerCount)
	if err != nil {
		return err
	}
	return nil
}

type JoinServerRefuse struct {
	TCPHeader

	RefuseReason uint8
}

func (p *JoinServerRefuse) MarshalBinary() ([]byte, error) {
	p.Type = JoinServerRefuseType
	p.Length = 1
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.RefuseReason)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *JoinServerRefuse) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.RefuseReason)
	if err != nil {
		return err
	}
	return nil
}

type UDPConnectionEstablished struct {
	TCPHeader
}

func (p *UDPConnectionEstablished) MarshalBinary() ([]byte, error) {
	p.Type = UDPConnectionEstablishedType
	p.Length = 0
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *UDPConnectionEstablished) UnmarshalBinary(b []byte) error {
	return nil
}

type LoadLevel struct {
	TCPHeader

	LevelFilename []byte
}

func (p *LoadLevel) MarshalBinary() ([]byte, error) {
	p.Type = LoadLevelType
	p.Length = uint16(len(p.LevelFilename))
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.LevelFilename)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *LoadLevel) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	p.LevelFilename = make([]byte, len(b))
	err = binary.Read(buf, binary.BigEndian, &p.LevelFilename)
	if err != nil {
		return err
	}
	return nil
}

type CurrentPlayersInfo struct {
	TCPHeader

	Players []PlayerInfo
}

type PlayerInfo struct {
	NameLength uint8
	Name       []byte // If NameLength > 0.
	Team       Team   // If NameLength > 0.
	State      *State // If NameLength > 0 && Team != 2.
}

type State struct {
	CommandSequenceNumber uint8
	X                     float32
	Y                     float32
	Z                     float32
}

func (p *CurrentPlayersInfo) MarshalBinary() ([]byte, error) {
	p.Type = CurrentPlayersInfoType
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	for _, playerInfo := range p.Players {
		err = binary.Write(&buf, binary.BigEndian, &playerInfo.NameLength)
		if err != nil {
			return nil, err
		}

		if playerInfo.NameLength != 0 {
			err = binary.Write(&buf, binary.BigEndian, &playerInfo.Name)
			if err != nil {
				return nil, err
			}
			err = binary.Write(&buf, binary.BigEndian, &playerInfo.Team)
			if err != nil {
				return nil, err
			}

			if playerInfo.Team != Spectator {
				err = binary.Write(&buf, binary.BigEndian, playerInfo.State)
				if err != nil {
					return nil, err
				}
			}
		}
	}
	return buf.Bytes(), nil
}
func (p *CurrentPlayersInfo) UnmarshalBinary(b []byte, totalPlayerCount uint8) error {
	buf := bytes.NewReader(b)
	var err error
	p.Players = make([]PlayerInfo, totalPlayerCount) // TODO: Figure out if this is the best way.
	for i := range p.Players {
		err = binary.Read(buf, binary.BigEndian, &p.Players[i].NameLength)
		if err != nil {
			return err
		}

		if p.Players[i].NameLength != 0 {
			p.Players[i].Name = make([]byte, p.Players[i].NameLength)
			err = binary.Read(buf, binary.BigEndian, &p.Players[i].Name)
			if err != nil {
				return err
			}

			err = binary.Read(buf, binary.BigEndian, &p.Players[i].Team)
			if err != nil {
				return err
			}

			if p.Players[i].Team != Spectator {
				p.Players[i].State = new(State)
				err = binary.Read(buf, binary.BigEndian, p.Players[i].State)
				if err != nil {
					return err
				}
			}
		}
	}
	return nil
}

type PlayerJoinedServer struct {
	TCPHeader

	PlayerID   uint8
	NameLength uint8
	Name       []byte
}

func (p *PlayerJoinedServer) MarshalBinary() ([]byte, error) {
	p.Type = PlayerJoinedServerType
	p.Length = 2 + uint16(len(p.Name))
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.PlayerID)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.NameLength)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.Name)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *PlayerJoinedServer) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.PlayerID)
	if err != nil {
		return err
	}
	err = binary.Read(buf, binary.BigEndian, &p.NameLength)
	if err != nil {
		return err
	}
	p.Name = make([]byte, p.NameLength)
	err = binary.Read(buf, binary.BigEndian, &p.Name)
	if err != nil {
		return err
	}
	return nil
}

type PlayerLeftServer struct {
	TCPHeader

	PlayerID uint8
}

func (p *PlayerLeftServer) MarshalBinary() ([]byte, error) {
	p.Type = PlayerLeftServerType
	p.Length = 1
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.PlayerID)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *PlayerLeftServer) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.PlayerID)
	if err != nil {
		return err
	}
	return nil
}

type JoinTeamRequest struct {
	TCPHeader

	PlayerNumber *uint8 // If > 1 players per connection, player index within the connection.
	Team         Team
}

func (p *JoinTeamRequest) MarshalBinary() ([]byte, error) {
	p.Type = JoinTeamRequestType
	p.Length = 1
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.Team)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *JoinTeamRequest) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	// TODO: Handle potential PlayerNumbep.
	err = binary.Read(buf, binary.BigEndian, &p.Team)
	if err != nil {
		return err
	}
	return nil
}

type PlayerJoinedTeam struct {
	TCPHeader

	PlayerID uint8
	Team     Team
	State    *State // If Team != 2.
}

func (p *PlayerJoinedTeam) MarshalBinary() ([]byte, error) {
	p.Type = PlayerJoinedTeamType
	p.Length = 2
	if p.State != nil {
		p.Length += 13
	}
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.PlayerID)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.Team)
	if err != nil {
		return nil, err
	}
	if p.State != nil {
		err = binary.Write(&buf, binary.BigEndian, p.State)
		if err != nil {
			return nil, err
		}
	}
	return buf.Bytes(), nil
}
func (p *PlayerJoinedTeam) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.PlayerID)
	if err != nil {
		return err
	}
	err = binary.Read(buf, binary.BigEndian, &p.Team)
	if err != nil {
		return err
	}
	if p.Team != Spectator {
		p.State = new(State)
		err = binary.Read(buf, binary.BigEndian, p.State)
		if err != nil {
			return err
		}
	}
	return nil
}

type EnterGamePermission struct {
	TCPHeader
}

func (p *EnterGamePermission) MarshalBinary() ([]byte, error) {
	p.Type = EnterGamePermissionType
	p.Length = 0
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *EnterGamePermission) UnmarshalBinary(b []byte) error {
	return nil
}

type EnteredGameNotification struct {
	TCPHeader
}

func (p *EnteredGameNotification) MarshalBinary() ([]byte, error) {
	p.Type = EnteredGameNotificationType
	p.Length = 0
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *EnteredGameNotification) UnmarshalBinary(b []byte) error {
	return nil
}

type LocalPlayerInfo struct {
	TCPHeader

	NameLength  uint8
	Name        []byte
	CommandRate uint8
	UpdateRate  uint8
}

func (p *LocalPlayerInfo) MarshalBinary() ([]byte, error) {
	p.Type = LocalPlayerInfoType
	p.Length = 3 + uint16(len(p.Name))
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.NameLength)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.Name)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.CommandRate)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.UpdateRate)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *LocalPlayerInfo) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.NameLength)
	if err != nil {
		return err
	}
	p.Name = make([]byte, p.NameLength)
	err = binary.Read(buf, binary.BigEndian, &p.Name)
	if err != nil {
		return err
	}
	err = binary.Read(buf, binary.BigEndian, &p.CommandRate)
	if err != nil {
		return err
	}
	err = binary.Read(buf, binary.BigEndian, &p.UpdateRate)
	if err != nil {
		return err
	}
	return nil
}

type PlayerWasHit struct {
	TCPHeader

	PlayerID    uint8
	HealthGiven float32
}

func (p *PlayerWasHit) MarshalBinary() ([]byte, error) {
	p.Type = PlayerWasHitType
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.TCPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.PlayerID)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.HealthGiven)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *PlayerWasHit) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.PlayerID)
	if err != nil {
		return err
	}
	err = binary.Read(buf, binary.BigEndian, &p.HealthGiven)
	if err != nil {
		return err
	}
	return nil
}

type UDPHeader struct {
	Type Type
}

// UDPHeaderSize is the size in bytes of the UDP packet header.
const UDPHeaderSize = 1

func init() {
	if UDPHeaderSize != binary.Size(UDPHeader{}) {
		panic("UDPHeaderSize != binary.Size(UDPHeader{})")
	}
}

func (h *UDPHeader) marshalBinary() []byte {
	b := make([]byte, UDPHeaderSize)
	b[0] = byte(h.Type)
	return b
}
func (h *UDPHeader) UnmarshalBinary(b []byte) error {
	// TODO: Should this be != or <? Find out how UnmarshalBinary is supposed to handle not enough/too much data.
	if len(b) != UDPHeaderSize {
		return fmt.Errorf("UDPHeader has unexpected size of %v instead of %v", len(b), UDPHeaderSize)
	}
	h.Type = Type(b[0])
	return nil
}

type ClientCommand struct {
	UDPHeader

	CommandSequenceNumber uint8 // Latest command sequence number (i.e., last one in the slice).
	CommandSeriesNumber   uint8
	MovesCount            uint8  // len(Moves) - 1.
	Moves                 []Move // Oldest first, newest last.
}

func (p *ClientCommand) MarshalBinary() ([]byte, error) {
	p.Type = ClientCommandType
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.UDPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.CommandSequenceNumber)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.CommandSeriesNumber)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.MovesCount)
	if err != nil {
		return nil, err
	}
	for _, move := range p.Moves {
		err = binary.Write(&buf, binary.BigEndian, &move)
		if err != nil {
			return nil, err
		}
	}
	return buf.Bytes(), nil
}
func (p *ClientCommand) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.CommandSequenceNumber)
	if err != nil {
		return err
	}
	err = binary.Read(buf, binary.BigEndian, &p.CommandSeriesNumber)
	if err != nil {
		return err
	}
	err = binary.Read(buf, binary.BigEndian, &p.MovesCount)
	if err != nil {
		return err
	}
	movesCount := uint16(p.MovesCount) + 1 // De-normalize back to 1 (min value), prevent overflow to 0.
	p.Moves = make([]Move, movesCount)
	err = binary.Read(buf, binary.BigEndian, &p.Moves)
	if err != nil {
		return err
	}
	return nil
}

type Move struct {
	MoveDirection int8
	Stealth       uint8
	Z             float32
}

type ServerUpdate struct {
	UDPHeader

	CurrentUpdateSequenceNumber uint8
	PlayerUpdates               []PlayerUpdate
}

func (p *ServerUpdate) MarshalBinary() ([]byte, error) {
	p.Type = ServerUpdateType
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.UDPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.CurrentUpdateSequenceNumber)
	if err != nil {
		return nil, err
	}
	for _, playerUpdate := range p.PlayerUpdates {
		err = binary.Write(&buf, binary.BigEndian, &playerUpdate.ActivePlayer)
		if err != nil {
			return nil, err
		}

		if playerUpdate.ActivePlayer != 0 {
			err = binary.Write(&buf, binary.BigEndian, playerUpdate.State)
			if err != nil {
				return nil, err
			}
		}
	}
	return buf.Bytes(), nil
}
func (p *ServerUpdate) UnmarshalBinary(b []byte, totalPlayerCount uint8) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.CurrentUpdateSequenceNumber)
	if err != nil {
		return err
	}
	p.PlayerUpdates = make([]PlayerUpdate, totalPlayerCount) // TODO: Figure out if this is the best way.
	for i := range p.PlayerUpdates {
		var playerUpdate PlayerUpdate
		err = binary.Read(buf, binary.BigEndian, &playerUpdate.ActivePlayer)
		if err != nil {
			return err
		}

		if playerUpdate.ActivePlayer != 0 {
			playerUpdate.State = new(State)
			err = binary.Read(buf, binary.BigEndian, playerUpdate.State)
			if err != nil {
				return err
			}
		}

		p.PlayerUpdates[i] = playerUpdate
	}
	return nil
}

type PlayerUpdate struct {
	ActivePlayer uint8
	State        *State // If ActivePlayer == true.
}

type Ping struct {
	UDPHeader

	PingData      uint32
	LastLatencies []uint16
}

func (p *Ping) MarshalBinary() ([]byte, error) {
	p.Type = PingType
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.UDPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.PingData)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.LastLatencies)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *Ping) UnmarshalBinary(b []byte, totalPlayerCount uint8) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.PingData)
	if err != nil {
		return err
	}
	p.LastLatencies = make([]uint16, totalPlayerCount) // TODO: Figure out if this is the best way.
	err = binary.Read(buf, binary.BigEndian, &p.LastLatencies)
	if err != nil {
		return err
	}
	return nil
}

type Pong struct {
	UDPHeader

	PingData uint32
}

func (p *Pong) MarshalBinary() ([]byte, error) {
	p.Type = PongType
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.UDPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.PingData)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *Pong) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.PingData)
	if err != nil {
		return err
	}
	return nil
}

type Pung struct {
	UDPHeader

	PingData uint32
	Time     float64
}

func (p *Pung) MarshalBinary() ([]byte, error) {
	p.Type = PungType
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.UDPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.PingData)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.Time)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *Pung) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.PingData)
	if err != nil {
		return err
	}
	err = binary.Read(buf, binary.BigEndian, &p.Time)
	if err != nil {
		return err
	}
	return nil
}

type Handshake struct {
	UDPHeader

	Signature uint64
}

func (p *Handshake) MarshalBinary() ([]byte, error) {
	p.Type = HandshakeType
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.UDPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.Signature)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *Handshake) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.Signature)
	if err != nil {
		return err
	}
	return nil
}

type TimeRequest struct {
	UDPHeader

	SequenceNumber uint8
}

func (p *TimeRequest) MarshalBinary() ([]byte, error) {
	p.Type = TimeRequestType
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.UDPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.SequenceNumber)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *TimeRequest) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.SequenceNumber)
	if err != nil {
		return err
	}
	return nil
}

type TimeResponse struct {
	UDPHeader

	SequenceNumber uint8
	Time           float64
}

func (p *TimeResponse) MarshalBinary() ([]byte, error) {
	p.Type = TimeResponseType
	var buf bytes.Buffer
	var err error
	_, err = buf.Write(p.UDPHeader.marshalBinary())
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.SequenceNumber)
	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.BigEndian, &p.Time)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
func (p *TimeResponse) UnmarshalBinary(b []byte) error {
	buf := bytes.NewReader(b)
	var err error
	err = binary.Read(buf, binary.BigEndian, &p.SequenceNumber)
	if err != nil {
		return err
	}
	err = binary.Read(buf, binary.BigEndian, &p.Time)
	if err != nil {
		return err
	}
	return nil
}
