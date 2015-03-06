package main

import (
	"log"
	"math"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/shurcooL/eX0/eX0-go/packet"
	glfw "github.com/shurcooL/goglfw"
)

func (this *character) inputCommand(window *glfw.Window) packet.Move {
	var move = packet.Move{
		MoveDirection: 255,
		Z:             this.Z,
	}

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

func (this *character) input(window *glfw.Window) {
	move := this.inputCommand(window)

	// Physics update.
	{
		const TOP_SPEED = 3.5

		var TargetVel mgl32.Vec2

		if move.MoveDirection == 255 {
		} else if move.MoveDirection >= 0 && move.MoveDirection < 8 {
			direction := float64(move.Z) + Tau*float64(move.MoveDirection)/8
			speed := TOP_SPEED
			if move.Stealth != 0 {
				speed -= 2.25
			}

			TargetVel[0] = float32(math.Sin(direction) * speed)
			TargetVel[1] = float32(math.Cos(direction) * speed)
		} else {
			log.Printf("WARNING: Invalid nMoveDirection = %v!\n", move.MoveDirection)
		}

		var CurrentVel = this.Vel
		var Delta = TargetVel.Sub(CurrentVel)
		if DeltaLength := float64(Delta.Len()); DeltaLength >= 0.001 {
			Delta = Delta.Normalize()

			var Move1 = DeltaLength * DeltaLength * 0.03
			var Move2 = math.Min(0.2, DeltaLength)

			CurrentVel = CurrentVel.Add(Delta.Mul(float32(math.Max(Move1, Move2))))
		}

		this.Vel = CurrentVel

		this.pos = this.pos.Add(this.Vel)
		this.Z = move.Z
	}
}
