package main

import (
	"github.com/goxjs/glfw"
	"github.com/shurcooL/eX0/eX0-go/packet"
)

func inputCommand(logic *logic, window *glfw.Window) packet.Move {
	//playersStateMu.Lock()
	var move = packet.Move{
		MoveDirection: -1,
		Z:             logic.playersState[components.client.playerID].LatestPredicted().Z + components.client.ZOffset,
	}
	components.client.ZOffset = 0
	//playersStateMu.Unlock()

	var direction [2]int8
	if (window.GetKey(glfw.KeyA) != glfw.Release) && !(window.GetKey(glfw.KeyD) != glfw.Release) {
		direction[0] = -1
	} else if (window.GetKey(glfw.KeyD) != glfw.Release) && !(window.GetKey(glfw.KeyA) != glfw.Release) {
		direction[0] = +1
	}
	if (window.GetKey(glfw.KeyW) != glfw.Release || window.GetKey(glfw.KeyUp) != glfw.Release) && !(window.GetKey(glfw.KeyS) != glfw.Release || window.GetKey(glfw.KeyDown) != glfw.Release) {
		direction[1] = -1
	} else if (window.GetKey(glfw.KeyS) != glfw.Release || window.GetKey(glfw.KeyDown) != glfw.Release) && !(window.GetKey(glfw.KeyW) != glfw.Release || window.GetKey(glfw.KeyUp) != glfw.Release) {
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

	if window.GetKey(glfw.KeyLeftShift) != glfw.Release || window.GetKey(glfw.KeyRightShift) != glfw.Release {
		move.Stealth = 1
	}

	return move
}
