// Package packet is for TCP and UDP packets used in eX0 networking protocol.
package packet

import "encoding/binary"

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

type JoinServerRequest struct {
	TCPHeader

	Version    uint16
	Passphrase [16]byte
	Signature  uint64
}

type JoinServerAccept struct {
	TCPHeader

	YourPlayerID     uint8
	TotalPlayerCount uint8
}

type JoinServerRefuse struct {
	TCPHeader

	RefuseReason uint8
}

type UDPConnectionEstablished struct {
	TCPHeader
}

type LoadLevel struct {
	TCPHeader

	LevelFilename []byte
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

type PlayerJoinedServer struct {
	TCPHeader

	PlayerID   uint8
	NameLength uint8
	Name       []byte
}

type PlayerLeftServer struct {
	TCPHeader

	PlayerID uint8
}

type JoinTeamRequest struct {
	TCPHeader

	PlayerNumber *uint8 // If > 1 players per connection, player index within the connection.
	Team         Team
}

type PlayerJoinedTeam struct {
	TCPHeader

	PlayerID uint8
	Team     Team
	State    *State // If Team != 2.
}

type EnterGamePermission struct {
	TCPHeader
}

type EnteredGameNotification struct {
	TCPHeader
}

type LocalPlayerInfo struct {
	TCPHeader

	NameLength  uint8
	Name        []byte
	CommandRate uint8
	UpdateRate  uint8
}

type PlayerWasHit struct {
	TCPHeader

	PlayerID    uint8
	HealthGiven float32
}

type UDPHeader struct {
	Type Type
}

type ClientCommand struct {
	UDPHeader

	CommandSequenceNumber uint8 // Latest command sequence number (i.e., last one in the slice).
	CommandSeriesNumber   uint8
	MovesCount            uint8  // len(Moves) - 1.
	Moves                 []Move // Oldest first, newest last.
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

type PlayerUpdate struct {
	ActivePlayer uint8
	State        *State // If ActivePlayer == true.
}

type Ping struct {
	UDPHeader

	PingData      uint32
	LastLatencies []uint16
}

type Pong struct {
	UDPHeader

	PingData uint32
}

type Pung struct {
	UDPHeader

	PingData uint32
	Time     float64
}

type Handshake struct {
	UDPHeader

	Signature uint64
}

type TimeRequest struct {
	UDPHeader

	SequenceNumber uint8
}

type TimeResponse struct {
	UDPHeader

	SequenceNumber uint8
	Time           float64
}
