package main

import (
	"math"
	"time"
)

const Tau = 2 * math.Pi

type logic struct {
	quit chan struct{} // Receiving a value on this channel results in sending a response, and quitting.
}

func startLogic(doInput func()) *logic {
	state.Lock()
	state.session.GlobalStateSequenceNumberTEST = 0
	state.session.NextTickTime = time.Since(startedProcess).Seconds()
	state.Unlock()
	l := &logic{
		quit: make(chan struct{}),
	}
	go l.gameLogic(doInput)
	return l
}

func (l *logic) gameLogic(doInput func()) {
	for {
		select {
		case <-l.quit:
			l.quit <- struct{}{}
			return
		default:
		}

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
