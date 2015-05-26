package main

import (
	"bytes"
	"encoding/binary"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/goxjs/glfw"
	"github.com/shurcooL/eX0/eX0-go/packet"
)

func inputCommand(window *glfw.Window) packet.Move {
	playersStateMu.Lock()
	var move = packet.Move{
		MoveDirection: 255,
		Z:             playersState[int(components_client_id)].Z,
	}
	playersStateMu.Unlock()

	var direction [2]int8
	if (window.GetKey(glfw.KeyA) != glfw.Release) && !(window.GetKey(glfw.KeyD) != glfw.Release) {
		direction[0] = -1
	} else if (window.GetKey(glfw.KeyD) != glfw.Release) && !(window.GetKey(glfw.KeyA) != glfw.Release) {
		direction[0] = +1
	}
	if (window.GetKey(glfw.KeyW) != glfw.Release) && !(window.GetKey(glfw.KeyS) != glfw.Release) {
		direction[1] = -1
	} else if (window.GetKey(glfw.KeyS) != glfw.Release) && !(window.GetKey(glfw.KeyW) != glfw.Release) {
		direction[1] = +1
	}
	switch {
	case direction[0] == 0 && direction[1] == -1:
		move.MoveDirection = 0
	case direction[0] == 1 && direction[1] == -1:
		move.MoveDirection = 1
	case direction[0] == 1 && direction[1] == 0:
		move.MoveDirection = 2
	case direction[0] == 1 && direction[1] == 1:
		move.MoveDirection = 3
	case direction[0] == 0 && direction[1] == 1:
		move.MoveDirection = 4
	case direction[0] == -1 && direction[1] == 1:
		move.MoveDirection = 5
	case direction[0] == -1 && direction[1] == 0:
		move.MoveDirection = 6
	case direction[0] == -1 && direction[1] == -1:
		move.MoveDirection = 7
	}

	if (window.GetKey(glfw.KeyLeft) != glfw.Release) && !(window.GetKey(glfw.KeyRight) != glfw.Release) {
		move.Z -= mgl32.DegToRad(3)
	} else if (window.GetKey(glfw.KeyRight) != glfw.Release) && !(window.GetKey(glfw.KeyLeft) != glfw.Release) {
		move.Z += mgl32.DegToRad(3)
	}

	if window.GetKey(glfw.KeyLeftShift) != glfw.Release || window.GetKey(glfw.KeyRightShift) != glfw.Release {
		move.Stealth = 1
	}

	return move
}

func input(window *glfw.Window) {
	move := inputCommand(window)

	// TODO: This should be done via Local/Network State Auther. This currently hardcodes network state auther.
	// Send a ClientCommand packet to server.
	{
		var p packet.ClientCommand
		p.Type = packet.ClientCommandType
		p.CommandSequenceNumber = clientLastAckedCmdSequenceNumber
		p.CommandSeriesNumber = 1
		p.Moves = []packet.Move{move}
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

		err = sendUdpPacket(clientToServerConn, buf.Bytes())
		if err != nil {
			panic(err)
		}

		// TODO: This should be happening when receiving packet.ServerUpdate, etc.
		clientLastAckedCmdSequenceNumber++
	}
}
