package main

import (
	"math"
	"time"
)

const Tau = 2 * math.Pi

type logic struct{}

func startLogic() *logic {
	state.session.GlobalStateSequenceNumberTEST = 0
	state.session.NextTickTime = time.Since(startedProcess).Seconds()
	go gameLogic(nil)

	return &logic{}
}

func gameLogic(doInput func()) {
	for {
		tick := false
		sleep := time.Millisecond

		state.Lock()
		if now := time.Since(startedProcess).Seconds(); now >= state.session.NextTickTime {
			state.session.NextTickTime += 1.0 / 20
			state.session.GlobalStateSequenceNumberTEST++
			tick = true
			sleep = time.Duration((state.session.NextTickTime - now) * float64(time.Second))
		}
		state.Unlock()

		if tick {
			if doInput != nil {
				doInput()
			}
		}

		time.Sleep(sleep)
	}
}
