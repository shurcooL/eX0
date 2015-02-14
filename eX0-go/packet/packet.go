// Package packet is for TCP and UDP packets used in eX0 networking protocol.
package packet

const MAX_UDP_SIZE = 1448

type Type uint8

const (
	JoinServerRequestType        Type = 1
	JoinServerAcceptType         Type = 2
	JoinServerRefuseType         Type = 3
	UdpConnectionEstablishedType Type = 5
	EnterGamePermissionType      Type = 6
	EnteredGameNotificationType  Type = 7
	LoadLevelType                Type = 20
	CurrentPlayersInfoType       Type = 21
	JoinTeamRequestType          Type = 27
	PlayerJoinedTeamType         Type = 28
	LocalPlayerInfoType          Type = 30

	ServerUpdateType Type = 2
	PingType         Type = 10
	PongType         Type = 11
	PungType         Type = 12
	HandshakeType    Type = 100
	TimeRequestType  Type = 105
	TimeResponseType Type = 106
)

//go:generate stringer -type=Type

type TcpHeader struct {
	Length uint16
	Type   Type
}

type JoinServerRequest struct {
	TcpHeader

	Version    uint16
	Passphrase [16]byte
	Signature  uint64
}

type JoinServerAccept struct {
	TcpHeader

	YourPlayerId     uint8
	TotalPlayerCount uint8
}

type JoinServerRefuse struct {
	TcpHeader

	RefuseReason uint8
}

type UdpConnectionEstablished struct {
	TcpHeader
}

type LoadLevel struct {
	TcpHeader

	LevelFilename []byte
}

type CurrentPlayersInfo struct {
	TcpHeader

	Players []PlayerInfo
}

type PlayerInfo struct {
	NameLength uint8
	Name       []byte
	Team       uint8
	State      *State // If Team != 2.
}

type State struct {
	CommandSequenceNumber uint8
	X                     float32
	Y                     float32
	Z                     float32
}

type JoinTeamRequest struct {
	TcpHeader

	PlayerNumber *uint8 // If > 1 players per connection, player index within the connection.
	Team         uint8
}

type PlayerJoinedTeam struct {
	TcpHeader

	PlayerId uint8
	Team     uint8
	State    *State // If Team != 2.
}

type EnterGamePermission struct {
	TcpHeader
}

type EnteredGameNotification struct {
	TcpHeader
}

type LocalPlayerInfo struct {
	TcpHeader

	NameLength  uint8
	Name        []byte
	CommandRate uint8
	UpdateRate  uint8
}

type UdpHeader struct {
	Type Type
}

type ServerUpdatePacket struct {
	UdpHeader

	CurrentUpdateSequenceNumber uint8
	Players                     []PlayerUpdate
}

type PlayerUpdate struct {
	ActivePlayer uint8
	State        *State // If ActivePlayer == true.
}

type Ping struct {
	UdpHeader

	PingData      uint32
	LastLatencies []uint16
}

type Pong struct {
	UdpHeader

	PingData uint32
}

type Pung struct {
	UdpHeader

	PingData uint32
	Time     float64
}

type Handshake struct {
	UdpHeader

	Signature uint64
}

type TimeRequest struct {
	UdpHeader

	SequenceNumber uint8
}

type TimeResponse struct {
	UdpHeader

	SequenceNumber uint8
	Time           float64
}
