package main

import (
	"math"
	"time"
)

const Tau = 2 * math.Pi

type logic struct {
	quit chan struct{} // Receiving a value on this channel results in sending a response, and quitting.

	Input   chan func()
	doInput func()
}

func startLogic() *logic {
	state.Lock()
	startedProcess = time.Now()
	state.session.GlobalStateSequenceNumberTEST = 0
	state.session.NextTickTime = 0
	state.Unlock()
	l := &logic{
		quit:  make(chan struct{}),
		Input: make(chan func()),
	}
	go l.gameLogic()
	return l
}

func (l *logic) gameLogic() {
	var doInput func()

	for {
		select {
		case <-l.quit:
			l.quit <- struct{}{}
			return
		case doInput = <-l.Input:
			// TODO: Fix bug in GopherJS, remove this noop case body.
			//       https://github.com/gopherjs/gopherjs/issues/267
			_ = 0
		default:
		}

		tick := false
		sleep := time.Millisecond

		state.Lock()
		if now := time.Since(startedProcess).Seconds(); now >= state.session.NextTickTime {
			state.session.NextTickTime += 1.0 / 20 * 20
			state.session.GlobalStateSequenceNumberTEST++
			//fmt.Fprintln(os.Stderr, "GlobalStateSequenceNumberTEST++:", state.session.GlobalStateSequenceNumberTEST)
			tick = true
			//sleep = time.Duration((state.session.NextTickTime - now) * float64(time.Second))
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
