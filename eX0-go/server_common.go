package main

import (
	"fmt"
	"sync"
	"time"

	"github.com/shurcooL/eX0/eX0-go/packet"
)

type server struct{}

var state = struct {
	sync.Mutex

	TotalPlayerCount uint8

	session struct {
		GlobalStateSequenceNumberTEST uint8
		NextTickTime                  float64
	}

	connections []*Connection
}{
	TotalPlayerCount: 16,
}

const commandRate = 20

// TODO: I think this should be moved into logic component (not server), yeah?
//       There's also some overlap with state.connections, shouldn't that be resolved?
var playersStateMu sync.Mutex
var playersState = map[uint8]playerState{} // Player Id -> Player State.

type playerPosVel struct {
	X, Y, Z    float32
	VelX, VelY float32
}

type sequencedPlayerPosVel struct {
	playerPosVel
	SequenceNumber uint8
}

// TODO: Split into positions (there will be many over time) and current name, team, connection, etc.
type playerState struct {
	authed []sequencedPlayerPosVel

	Name string
	Team packet.Team

	// TODO: Move this to a better place.
	conn *Connection

	lastServerUpdateSequenceNumber uint8 // Sequence Number of last packet.ServerUpdate sent to this connection. // TODO: This should go into a serverToClient connection struct.
}

func (ps playerState) LatestAuthed() sequencedPlayerPosVel {
	return ps.authed[len(ps.authed)-1]
}

func (ps *playerState) PushAuthed(newState sequencedPlayerPosVel) {
	if len(ps.authed) > 0 && newState.SequenceNumber == ps.authed[len(ps.authed)-1].SequenceNumber {
		// Skip updates that are not newer.
		return
	}
	// TODO: GC.
	//fmt.Fprintln(os.Stderr, "PushAuthed:", newState.SequenceNumber)
	ps.authed = append(ps.authed, newState)
}

func (ps *playerState) NewSeries() {
	// TODO: Consider preserving.
	ps.authed = nil
}

func (ps playerState) Interpolated() playerPosVel {
	if len(ps.authed) == 1 {
		return ps.authed[0].playerPosVel
	}

	bi := len(ps.authed) - 1
	b := ps.authed[bi]

	// Check if we're looking for a sequence number newer than history contains.
	if int8(state.session.GlobalStateSequenceNumberTEST-b.SequenceNumber) > 0 {
		// TODO: Extrapolate from history?
		fmt.Println("warning: using LatestAuthed because:", int8(state.session.GlobalStateSequenceNumberTEST-b.SequenceNumber))
		return ps.LatestAuthed().playerPosVel
	}

	for b.SequenceNumber != state.session.GlobalStateSequenceNumberTEST {
		bi--
		b = ps.authed[bi]
		if bi == 0 {
			break
		}
	}

	if bi == 0 {
		return ps.authed[0].playerPosVel
	}

	a := ps.authed[bi-1]

	interp := float32((time.Since(startedProcess).Seconds() - state.session.NextTickTime + 1.0/commandRate) * commandRate)

	return playerPosVel{
		X: (1-interp)*a.playerPosVel.X + interp*b.playerPosVel.X,
		Y: (1-interp)*a.playerPosVel.Y + interp*b.playerPosVel.Y,
		Z: (1-interp)*a.playerPosVel.Z + interp*b.playerPosVel.Z,
	}
}
