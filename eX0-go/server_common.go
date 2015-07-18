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

var playersStateMu sync.Mutex            // Also protects serverLastAckedCmdSequenceNumber.
var playersState = map[int]playerState{} // Player Id -> Player State.

// TODO: Get rid of this.
var player0Spawn = playerState{
	X: 25,
	Y: -220,
	Z: 6.0,
}

type playerState struct {
	X, Y, Z    float32
	VelX, VelY float32

	// TODO: Move this to a better place.
	conn *Connection
}
