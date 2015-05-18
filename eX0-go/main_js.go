// +build js

package main

import "time"

var startedProcess = time.Now()

func main() {
	view()
}

// TODO: Move things below into right places (dedup them).

var state = struct {
	TotalPlayerCount uint8

	session struct {
		GlobalStateSequenceNumberTEST uint8
		NextTickTime                  float64
	}
}{
	TotalPlayerCount: 16,
}

var lastAckedCmdSequenceNumber uint8
