// +build js

package main

import (
	"sync"
	"time"
)

var startedProcess = time.Now()

func main() {
	view(true)
}

// TODO: Move things below into right places (dedup them).

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

var player0State = struct {
	X, Y, Z    float32
	VelX, VelY float32
}{
	X: 25,
	Y: -220,
	Z: 6.0,
}
