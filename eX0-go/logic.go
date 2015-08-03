package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"math"
	"os"
	"sync"
	"time"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/shurcooL/eX0/eX0-go/packet"
)

// Tau is the constant τ, which equals to 6.283185... or 2π.
const Tau = 2 * math.Pi

type logic struct {
	// TODO: Remove quit since it's not used anymore, if it's really not needed.
	quit chan struct{} // Receiving a value on this channel results in sending a response, and quitting.

	Input  chan func(logic *logic) packet.Move
	client chan *client

	started                   time.Time
	GlobalStateSequenceNumber uint8
	NextTickTime              float64

	TotalPlayerCount uint8

	// TODO: There's also some overlap with server.connections, shouldn't that be resolved?
	playersStateMu sync.Mutex
	playersState   map[uint8]playerState // Player Id -> Player State.
}

func startLogic() *logic {
	l := &logic{
		quit:                      make(chan struct{}),
		Input:                     make(chan func(logic *logic) packet.Move),
		client:                    make(chan *client),
		started:                   time.Now(),
		GlobalStateSequenceNumber: 0,
		NextTickTime:              0,

		playersState: make(map[uint8]playerState),
	}
	go l.gameLogic()
	return l
}

func (l *logic) gameLogic() {
	var debugFirstJoin = true
	var doInput func(logic *logic) packet.Move
	var client *client // TODO: This is used instead of reading components.client pointer directly. Find a better way to resolve a data race with components struct.

	for {
		select {
		case <-l.quit:
			l.quit <- struct{}{}
			return
		case doInput = <-l.Input:
		case client = <-l.client:
		default:
		}

		tick := false
		sleep := time.Millisecond

		state.Lock()
		for now := time.Since(l.started).Seconds(); now >= l.NextTickTime; {
			l.NextTickTime += 1.0 / commandRate
			l.GlobalStateSequenceNumber++
			tick = true
		}
		//sleep = time.Duration((l.NextTickTime - now) * float64(time.Second))
		state.Unlock()

		if debugFirstJoin && client != nil {
			l.playersStateMu.Lock()
			ps, ok := l.playersState[client.playerId]
			if ok && ps.Team != packet.Spectator {
				debugFirstJoin = false
				logicTime := float64(l.GlobalStateSequenceNumber) + (time.Since(l.started).Seconds()-l.NextTickTime)*commandRate
				fmt.Fprintf(os.Stderr, "%.3f: Pl#%v (%q) joined team %v at logic time %.2f/%v [logic].\n", time.Since(l.started).Seconds(), client.playerId, l.playersState[client.playerId].Name, ps.Team, logicTime, l.GlobalStateSequenceNumber)
			}
			l.playersStateMu.Unlock()
		}

		if tick && client != nil {
			if doInput != nil {
				l.playersStateMu.Lock()
				ps, ok := l.playersState[client.playerId]
				if ok && ps.Team != packet.Spectator {
					// Fill all missing commands (from last authed until one we're supposed to be by now (based on GlobalStateSequenceNumberTEST time).
					for lastState := ps.LatestPredicted(); int8(lastState.SequenceNumber-l.GlobalStateSequenceNumber) < 0; lastState = ps.LatestPredicted() {
						move := doInput(l)

						newState := nextState(lastState, move)

						ps.unconfirmed = append(ps.unconfirmed, predictedMove{
							move:      move,
							predicted: newState,
						})
					}
					l.playersState[client.playerId] = ps
				}
				l.playersStateMu.Unlock()
			}

			l.playersStateMu.Lock()
			ps, ok := l.playersState[client.playerId]
			l.playersStateMu.Unlock()

			if ok && ps.Team != packet.Spectator && len(ps.unconfirmed) > 0 {

				var moves []packet.Move
				for _, unconfirmed := range ps.unconfirmed {
					moves = append(moves, unconfirmed.move)
				}

				// TODO: This should be done via Local/Network State Auther. This currently hardcodes network state auther.
				// Send a ClientCommand packet to server.
				if client != nil && client.serverConn != nil && client.serverConn.JoinStatus >= IN_GAME {
					var p packet.ClientCommand
					p.Type = packet.ClientCommandType
					state.Lock()
					p.CommandSequenceNumber = l.GlobalStateSequenceNumber - 1
					state.Unlock()
					p.CommandSeriesNumber = 1 // TODO: Don't hardcode this.
					p.Moves = moves
					//fmt.Printf("%.3f: sending ClientCommand with %v moves, clientLastAckedCSN=%v, G-1=%v\n", time.Since(l.started).Seconds(), len(p.Moves), clientLastAckedCmdSequenceNumber, l.GlobalStateSequenceNumber-1)
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

					err = sendUdpPacket(client.serverConn, buf.Bytes())
					if err != nil {
						panic(err)
					}
				}
			}
		}

		time.Sleep(sleep)
	}
}

type playerPosVel struct {
	X, Y, Z    float32
	VelX, VelY float32
}

type sequencedPlayerPosVel struct {
	playerPosVel
	SequenceNumber uint8
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

func (ps *playerState) PushAuthed(newState sequencedPlayerPosVel) {
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
		same := mgl32.FloatEqualThreshold(newState.X, ps.unconfirmed[0].predicted.X, 0.001) &&
			mgl32.FloatEqualThreshold(newState.Y, ps.unconfirmed[0].predicted.Y, 0.001)
		if same {
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
		ps.unconfirmed[i].predicted = nextState(prevState, ps.unconfirmed[i].move)
		prevState = ps.unconfirmed[i].predicted
	}
}

func (ps *playerState) NewSeries() {
	// TODO: Consider preserving.
	ps.authed = nil
	ps.unconfirmed = nil
}

func (ps playerState) Interpolated(logic *logic, playerId uint8) playerPosVel {
	desiredAStateSN := logic.GlobalStateSequenceNumber - 1

	// When we don't have perfect information about present, return position 100 ms in the past.
	if components.client == nil || components.client.playerId != playerId {
		desiredAStateSN -= 2 // HACK: Assumes command rate of 20, this puts us 100 ms in the past (2 * 1s/20 = 100 ms).
	}

	// TODO: See if copying ps.authed slic is needed, maybe not.
	states := append([]sequencedPlayerPosVel(nil), ps.authed...)
	for _, unconfirmed := range ps.unconfirmed {
		states = append(states, unconfirmed.predicted)
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

	interp := float32(desiredAStateSN-a.SequenceNumber) + float32((time.Since(logic.started).Seconds()-logic.NextTickTime+1.0/commandRate)*commandRate)
	interpDistance := float32(b.SequenceNumber - a.SequenceNumber)
	interp = interp / interpDistance

	var z float32
	if components.client != nil && components.client.playerId == playerId && b.SequenceNumber == logic.GlobalStateSequenceNumber {
		z = b.playerPosVel.Z + components.client.ZOffset
	} else {
		z = (1-interp)*a.playerPosVel.Z + interp*b.playerPosVel.Z
	}
	return playerPosVel{
		X: (1-interp)*a.playerPosVel.X + interp*b.playerPosVel.X,
		Y: (1-interp)*a.playerPosVel.Y + interp*b.playerPosVel.Y,
		Z: z,
	}
}
