package main

import (
	"math"
	"time"
)

const Tau = 2 * math.Pi

func gameLogic(doInput func()) {
	for {
		state.Lock()
		for time.Since(startedProcess).Seconds() >= state.session.NextTickTime {
			state.session.NextTickTime += 1.0 / 20
			state.session.GlobalStateSequenceNumberTEST++

			if doInput != nil {
				doInput()
			}
		}
		state.Unlock()

		time.Sleep(time.Millisecond)
		//runtime.Gosched()
	}
}
