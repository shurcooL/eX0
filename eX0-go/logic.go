package main

import (
	"fmt"
	"log"
	"math"
	"os"
	"sync"
	"time"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/shurcooL/eX0/eX0-go/packet"
)

// Tau is the constant τ, which equals to 6.283185... or 2π.
// Reference: https://oeis.org/A019692
const Tau = 2 * math.Pi

type logic struct {
	// Input channel is only read during ticks when components.client != nil.
	Input chan func() packet.Move

	started                   time.Time
	GlobalStateSequenceNumber uint8 // TODO: Use int.
	NextTickTime              float64

	TotalPlayerCount uint8

	// TODO: There's also some overlap with server.connections, shouldn't that be resolved?
	playersStateMu sync.Mutex
	playersState   map[uint8]playerState // Player ID -> Player State.

	particleSystem particleSystem

	level *level
}

func newLogic() *logic {
	return &logic{
		Input:                     make(chan func() packet.Move, 1),
		started:                   time.Now(),
		GlobalStateSequenceNumber: 0,
		NextTickTime:              0,
		playersState:              make(map[uint8]playerState),
	}
}

func (l *logic) start() {
	go l.gameLogic()
}

func (l *logic) gameLogic() {
	var debugFirstJoin = true
	var doInput func() packet.Move

	for {
		tick := false

		state.Lock()
		l.playersStateMu.Lock() // For GlobalStateSequenceNumber.
		now := time.Since(l.started).Seconds()
		for now >= l.NextTickTime {
			l.NextTickTime += 1.0 / commandRate
			l.GlobalStateSequenceNumber++
			tick = true
		}
		l.playersStateMu.Unlock()
		state.Unlock()

		if debugFirstJoin && components.client != nil {
			state.Lock()
			playerID := components.client.playerID
			state.Unlock()
			l.playersStateMu.Lock()
			ps, ok := l.playersState[playerID]
			if ok && ps.Team != packet.Spectator {
				debugFirstJoin = false
				logicTime := float64(l.GlobalStateSequenceNumber) + (now-l.NextTickTime)*commandRate
				fmt.Fprintf(os.Stderr, "%.3f: Pl#%v (%q) joined team %v at logic time %.2f/%v [logic].\n", now, playerID, l.playersState[playerID].Name, ps.Team, logicTime, l.GlobalStateSequenceNumber)
			}
			l.playersStateMu.Unlock()
		}

		if tick && components.client != nil {
			state.Lock()
			playerID := components.client.playerID
			state.Unlock()

			select {
			case doInput = <-l.Input:
			default:
			}
			if doInput != nil {
				l.playersStateMu.Lock()
				ps, ok := l.playersState[playerID]
				if ok && ps.Team != packet.Spectator && ps.Health > 0 {
					// Fill all missing commands (from last authed until one we're supposed to be by now (based on GlobalStateSequenceNumber time).
					for lastState := ps.LatestPredicted(); int8(lastState.SequenceNumber-l.GlobalStateSequenceNumber) < 0; lastState = ps.LatestPredicted() {
						move := doInput()

						newState := l.nextState(lastState, move)

						ps.unconfirmed = append(ps.unconfirmed, predictedMove{
							move:      move,
							predicted: newState,
						})
					}
					l.playersState[playerID] = ps
				}
				l.playersStateMu.Unlock()
			}

			if components.client.serverConn != nil && components.client.serverConn.JoinStatus >= IN_GAME {
				// Send a ClientCommand packet to server.
				// TODO: This should be done via Local/Network State Auther. This currently hardcodes network state auther.
				state.Lock()
				l.playersStateMu.Lock() // For GlobalStateSequenceNumber.
				ps, ok := l.playersState[playerID]
				if ok && ps.Team != packet.Spectator && len(ps.unconfirmed) > 0 {
					var p packet.ClientCommand
					p.CommandSequenceNumber = l.GlobalStateSequenceNumber - 1
					p.CommandSeriesNumber = 1 // TODO: Don't hardcode this.
					for _, unconfirmed := range ps.unconfirmed {
						p.Moves = append(p.Moves, unconfirmed.move)
					}
					//fmt.Printf("%.3f: sending ClientCommand with %v moves, clientLastAckedCSN=%v, G-1=%v\n", now, len(p.Moves), clientLastAckedCmdSequenceNumber, l.GlobalStateSequenceNumber-1)
					/*for i, unconfirmed := range ps.unconfirmed {
						fmt.Println(i, "unconfirmed.predicted.SequenceNumber:", unconfirmed.predicted.SequenceNumber, "dir:", unconfirmed.move.MoveDirection)
					}*/
					p.MovesCount = uint8(len(p.Moves)) - 1

					err := sendUDPPacket(components.client.serverConn, &p)
					if err != nil {
						panic(err)
					}
				}
				l.playersStateMu.Unlock()
				state.Unlock()
			}

			l.particleSystem.Tick(now)
		}

		state.Lock() // For started and NextTickTime.
		now = time.Since(l.started).Seconds()
		sleep := time.Duration((l.NextTickTime - now) * float64(time.Second))
		state.Unlock()
		time.Sleep(sleep)
	}
}

type gameMoment float64

// SNAndTick returns game moment m as sequence number and tick.
// Tick is in [0, 1) range.
func (m gameMoment) SNAndTick() (sequenceNumber uint8, tick float64) {
	sn, tick := math.Modf(float64(m) * commandRate)
	sn2 := int(sn) // Workaround for https://github.com/gopherjs/gopherjs/issues/733.
	return uint8(sn2), tick
}

type playerPosVel struct {
	X, Y, Z    float32
	VelX, VelY float32
}

type sequencedPlayerPosVel struct {
	playerPosVel
	SequenceNumber uint8 // TODO: Use int.
}

type predictedMove struct {
	move      packet.Move
	predicted sequencedPlayerPosVel
}

// TODO: Split into positions (there will be many over time) and current name, team, connection, etc.
type playerState struct {
	authed      []sequencedPlayerPosVel
	unconfirmed []predictedMove

	Name string
	Team packet.Team

	// TODO: Might want to move Health to []sequencedPlayerPosVel, so its history is preserved, and DeadState won't be needed.
	Health    float32      // Health is in [0, 100] range.
	DeadState playerPosVel // DeadState is player state at the moment they died.

	// TODO: Move this to a better place.
	conn *Connection

	lastServerUpdateSequenceNumber uint8 // Sequence Number of last packet.ServerUpdate sent to this connection. // TODO: This should go into a serverToClient connection struct.
}

func (ps playerState) LatestAuthed() sequencedPlayerPosVel {
	return ps.authed[len(ps.authed)-1]
}
func (ps playerState) LatestPredicted() sequencedPlayerPosVel {
	if len(ps.unconfirmed) > 0 {
		return ps.unconfirmed[len(ps.unconfirmed)-1].predicted
	}
	return ps.authed[len(ps.authed)-1]
}

func (ps *playerState) PushAuthed(logic *logic, newState sequencedPlayerPosVel) {
	if len(ps.authed) > 0 && newState.SequenceNumber == ps.authed[len(ps.authed)-1].SequenceNumber {
		// Skip updates that are not newer.
		return
	}

	// Drop unconfirmed predicted moves once they've been authed.
	for len(ps.unconfirmed) > 0 && newState.SequenceNumber != ps.unconfirmed[0].predicted.SequenceNumber {
		//fmt.Fprintf(os.Stderr, "PushAuthed: dropping unmatched ps.unconfirmed\n")
		ps.unconfirmed = ps.unconfirmed[1:]
	}
	if len(ps.unconfirmed) > 0 && newState.SequenceNumber == ps.unconfirmed[0].predicted.SequenceNumber {
		if same := mgl32.FloatEqualThreshold(newState.X, ps.unconfirmed[0].predicted.X, 0.001) &&
			mgl32.FloatEqualThreshold(newState.Y, ps.unconfirmed[0].predicted.Y, 0.001); same {
			//fmt.Fprintf(os.Stderr, "PushAuthed: dropping matched ps.unconfirmed, same!\n")
		} else {
			fmt.Fprintf(os.Stderr, "PushAuthed: dropping matched ps.unconfirmed, diff by %v, %v\n", newState.X-ps.unconfirmed[0].predicted.X, newState.Y-ps.unconfirmed[0].predicted.Y)
		}

		// Keep the locally-predicted velocity.
		newState.VelX = ps.unconfirmed[0].predicted.VelX
		newState.VelY = ps.unconfirmed[0].predicted.VelY

		ps.unconfirmed = ps.unconfirmed[1:]
	}

	// TODO: GC.
	//fmt.Fprintln(os.Stderr, "PushAuthed:", newState.SequenceNumber)
	ps.authed = append(ps.authed, newState)

	// Replay remaining ones.
	prevState := newState
	for i := range ps.unconfirmed {
		ps.unconfirmed[i].predicted = logic.nextState(prevState, ps.unconfirmed[i].move)
		prevState = ps.unconfirmed[i].predicted
	}
}

func (ps *playerState) NewSeries() {
	// TODO: Consider preserving.
	ps.authed = nil
	ps.unconfirmed = nil
}

func (ps playerState) InterpolatedOrDead(gameMoment gameMoment, playerID uint8) playerPosVel {
	if ps.Health > 0 {
		// When we don't have perfect information about present, return position 100 ms in the past.
		if components.client == nil || components.client.playerID != playerID {
			gameMoment -= 0.1
		}

		pos := ps.interpolated(gameMoment)

		// HACK, TODO: Clean this up. Currently assumes asking for latest time for client player.
		if components.client != nil && components.client.playerID == playerID {
			components.client.TargetZMu.Lock()
			pos.Z = components.client.TargetZ
			components.client.TargetZMu.Unlock()
		}

		return pos
	} else {
		return ps.DeadState
	}
}

func (ps *playerState) SetDead(gameMoment gameMoment, playerID uint8) {
	// When we don't have perfect information about present, return position 100 ms in the past.
	if components.client == nil || components.client.playerID != playerID {
		gameMoment -= 0.1
	}

	pos := ps.interpolated(gameMoment)

	// HACK, TODO: Clean this up. Currently assumes asking for latest time for client player.
	if components.client != nil && components.client.playerID == playerID {
		components.client.TargetZMu.Lock()
		pos.Z = components.client.TargetZ
		components.client.TargetZMu.Unlock()
	}

	ps.DeadState = pos
}

func (ps playerState) interpolated(gameMoment gameMoment) playerPosVel {
	desiredAStateSN, tick := gameMoment.SNAndTick()

	// Gather all authed and predicted states to iterate over.
	states := append([]sequencedPlayerPosVel(nil), ps.authed...)
	for _, unconfirmed := range ps.unconfirmed {
		states = append(states, unconfirmed.predicted)
	}

	if len(states) == 0 {
		log.Println("playerState.interpolated called when there are no states")
		return playerPosVel{}
	}
	if len(states) == 1 {
		return states[0].playerPosVel
	}

	ai := len(states) - 1
	a := states[ai]

	// Check if we're looking for a sequence number newer than history contains.
	if int8(desiredAStateSN-a.SequenceNumber) > 0 {
		// Point A is not yet in history, so we'd need to extrapolate into future a lot.
		// TODO: Extrapolate into future?
		//fmt.Println("warning: using LatestAuthed because:", int8(desiredAStateSN-a.SequenceNumber))
		return states[len(states)-1].playerPosVel
	}
	// Scroll index of a back until it's the desired sn (or earlier).
	for int8(a.SequenceNumber-desiredAStateSN) > 0 {
		ai--
		if ai < 0 {
			// Point A is not in history, so we'd need to extrapolate into past... Just return earliest state for now.
			// TODO: Extrapolate in past?
			return states[0].playerPosVel
		}
		a = states[ai]
	}

	bi := ai + 1
	if bi >= len(states) {
		// Point B is not yet in history, so we'd need to extrapolate into future a little.
		// TODO: Extrapolate into future?
		return states[len(states)-1].playerPosVel
	}
	b := states[bi]

	interp := float32(desiredAStateSN-a.SequenceNumber) + float32(tick)
	interpDistance := float32(b.SequenceNumber - a.SequenceNumber)
	interp /= interpDistance // Normalize.

	return playerPosVel{
		X: (1-interp)*a.playerPosVel.X + interp*b.playerPosVel.X,
		Y: (1-interp)*a.playerPosVel.Y + interp*b.playerPosVel.Y,
		Z: (1-interp)*a.playerPosVel.Z + interp*b.playerPosVel.Z,
	}
}
