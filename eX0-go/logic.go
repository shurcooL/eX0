package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"math"
	"os"
	"time"

	"github.com/shurcooL/eX0/eX0-go/packet"
)

const Tau = 2 * math.Pi

type logic struct {
	quit chan struct{} // Receiving a value on this channel results in sending a response, and quitting.

	Input chan func() packet.Move

	started time.Time
}

func startLogic() *logic {
	l := &logic{
		quit:    make(chan struct{}),
		Input:   make(chan func() packet.Move),
		started: time.Now(),
	}
	state.Lock()
	state.session.GlobalStateSequenceNumberTEST = 0
	state.session.NextTickTime = 0
	state.Unlock()
	go l.gameLogic()
	return l
}

func (l *logic) gameLogic() {
	var debugFirstJoin = true
	var doInput func() packet.Move

	for {
		select {
		case <-l.quit:
			l.quit <- struct{}{}
			return
		case doInput = <-l.Input:
		default:
		}

		tick := false
		sleep := time.Millisecond

		state.Lock()
		if now := time.Since(l.started).Seconds(); now >= state.session.NextTickTime {
			state.session.NextTickTime += 1.0 / commandRate
			state.session.GlobalStateSequenceNumberTEST++
			//fmt.Fprintln(os.Stderr, "GlobalStateSequenceNumberTEST++:", state.session.GlobalStateSequenceNumberTEST)
			tick = true
			//sleep = time.Duration((state.session.NextTickTime - now) * float64(time.Second))
		}
		state.Unlock()

		if debugFirstJoin && components.client != nil {
			playersStateMu.Lock()
			ps, ok := playersState[components.client.playerId]
			if ok && ps.Team != packet.Spectator {
				debugFirstJoin = false
				logicTime := float64(state.session.GlobalStateSequenceNumberTEST) + (time.Since(l.started).Seconds()-state.session.NextTickTime)*commandRate
				fmt.Fprintf(os.Stderr, "%.3f: Pl#%v (%q) joined team %v at logic time %.2f/%v [logic].\n", time.Since(l.started).Seconds(), components.client.playerId, playersState[components.client.playerId].Name, ps.Team, logicTime, state.session.GlobalStateSequenceNumberTEST)
			}
			playersStateMu.Unlock()
		}

		if tick && components.client != nil {
			if doInput != nil {
				playersStateMu.Lock()
				ps, ok := playersState[components.client.playerId]
				if ok && ps.Team != packet.Spectator {
					// Fill all missing commands (from last authed until one we're supposed to be by now (based on GlobalStateSequenceNumberTEST time).
					for lastState := ps.LatestPredicted(); int8(lastState.SequenceNumber-state.session.GlobalStateSequenceNumberTEST) < 0; lastState = ps.LatestPredicted() {
						move := doInput()

						newState := nextState(lastState, move)

						ps.unconfirmed = append(ps.unconfirmed, predictedMove{
							move:      move,
							predicted: newState,
						})
					}
					playersState[components.client.playerId] = ps
				}
				playersStateMu.Unlock()
			}

			playersStateMu.Lock()
			ps, ok := playersState[components.client.playerId]
			playersStateMu.Unlock()

			if ok && ps.Team != packet.Spectator && len(ps.unconfirmed) > 0 {

				var moves []packet.Move
				for _, unconfirmed := range ps.unconfirmed {
					moves = append(moves, unconfirmed.move)
				}

				// TODO: This should be done via Local/Network State Auther. This currently hardcodes network state auther.
				// Send a ClientCommand packet to server.
				if components.client != nil && components.client.serverConn != nil && components.client.serverConn.JoinStatus >= IN_GAME {
					var p packet.ClientCommand
					p.Type = packet.ClientCommandType
					state.Lock()
					p.CommandSequenceNumber = state.session.GlobalStateSequenceNumberTEST - 1
					state.Unlock()
					p.CommandSeriesNumber = 1 // TODO: Don't hardcode this.
					p.Moves = moves
					//fmt.Printf("%.3f: sending ClientCommand with %v moves, clientLastAckedCSN=%v, G-1=%v\n", time.Since(l.started).Seconds(), len(p.Moves), clientLastAckedCmdSequenceNumber, state.session.GlobalStateSequenceNumberTEST-1)
					/*for i, unconfirmed := range ps.unconfirmed {
						fmt.Println(i, "unconfirmed.predicted.SequenceNumber:", unconfirmed.predicted.SequenceNumber, "dir:", unconfirmed.move.MoveDirection)
					}*/
					p.MovesCount = uint8(len(p.Moves)) - 1

					var buf bytes.Buffer
					err := binary.Write(&buf, binary.BigEndian, &p.UdpHeader)
					if err != nil {
						panic(err)
					}
					err = binary.Write(&buf, binary.BigEndian, &p.CommandSequenceNumber)
					if err != nil {
						panic(err)
					}
					err = binary.Write(&buf, binary.BigEndian, &p.CommandSeriesNumber)
					if err != nil {
						panic(err)
					}
					err = binary.Write(&buf, binary.BigEndian, &p.MovesCount)
					if err != nil {
						panic(err)
					}
					for _, move := range p.Moves {
						err = binary.Write(&buf, binary.BigEndian, &move)
						if err != nil {
							panic(err)
						}
					}

					err = sendUdpPacket(components.client.serverConn, buf.Bytes())
					if err != nil {
						panic(err)
					}
				}
			}
		}

		time.Sleep(sleep)
	}
}
