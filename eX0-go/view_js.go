// +build js

package main

import (
	"math"
	"time"
)

func main() {
	view()
}

var startedProcess = time.Now()

var state = struct {
	TotalPlayerCount uint8

	session struct {
		GlobalStateSequenceNumberTEST uint8
		NextTickTime                  float64
	}
}{
	TotalPlayerCount: 16,
}

const Tau = 2 * math.Pi

func gameLogic(doInput func()) {
	for {
		for time.Since(startedProcess).Seconds() >= state.session.NextTickTime {
			state.session.NextTickTime += 1.0 / 20
			state.session.GlobalStateSequenceNumberTEST++

			if doInput != nil {
				doInput()
			}
		}

		time.Sleep(time.Millisecond)
	}
}

var lastAckedCmdSequenceNumber uint8
