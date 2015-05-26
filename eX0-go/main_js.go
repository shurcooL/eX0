// +build js

package main

import (
	"sync"
	"time"
)

func main() {
	view(true)
}

// TODO: Move things below into right places (dedup them).

var startedProcess = time.Now()

var components struct {
	logic  *logic
	server *server
	client *client
}

type server struct{}

var state = struct {
	sync.Mutex

	TotalPlayerCount uint8

	session struct {
		GlobalStateSequenceNumberTEST uint8
		NextTickTime                  float64
	}
}{
	TotalPlayerCount: 16,
}

var playersStateMu sync.Mutex // Also protects serverLastAckedCmdSequenceNumber.
var playersState = map[int]playerState{}

// TODO: Get rid of this.
var player0Spawn = playerState{
	X: 25,
	Y: -220,
	Z: 6.0,
}

type playerState struct {
	X, Y, Z    float32
	VelX, VelY float32
}
