package main

import "sync"

type server struct{}

var state = struct {
	sync.Mutex

	TotalPlayerCount uint8

	session struct {
		GlobalStateSequenceNumberTEST uint8
		NextTickTime                  float64
	}

	connections []*Connection
}{
	TotalPlayerCount: 16,
}

// TODO: I think this should be moved into logic component (not server), yeah?
//       There's also some overlap with state.connections, shouldn't that be resolved?
var playersStateMu sync.Mutex
var playersState = map[uint8]playerState{} // Player Id -> Player State.

// TODO: Get rid of this.
var player0Spawn = playerState{
	X: 25,
	Y: -220,
	Z: 6.0,
}

// TODO: Split into positions (there will be many over time) and current name, team, connection, etc.
type playerState struct {
	X, Y, Z    float32
	VelX, VelY float32

	Name string
	Team uint8

	// TODO: Move this to a better place.
	conn *Connection

	serverLastAckedCmdSequenceNumber uint8
	lastUpdateSequenceNumber         uint8
}
