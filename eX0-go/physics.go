package main

import (
	"log"
	"math"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/shurcooL/eX0/eX0-go/packet"
)

// It takes State #n and Command #n to produce State #n+1.
func nextState(state sequencedPlayerPosVel, move packet.Move) (newState sequencedPlayerPosVel) {
	const TOP_SPEED = 3.5

	var TargetVel mgl32.Vec2

	if move.MoveDirection == -1 {
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

	var CurrentVel = mgl32.Vec2{state.VelX, state.VelY}
	var Delta = TargetVel.Sub(CurrentVel)
	if DeltaLength := float64(Delta.Len()); DeltaLength >= 0.001 {
		Delta = Delta.Normalize()

		var Move1 = DeltaLength * DeltaLength * 0.03
		var Move2 = math.Min(0.2, DeltaLength)

		CurrentVel = CurrentVel.Add(Delta.Mul(float32(math.Max(Move1, Move2))))
	}

	if commandRate == 1 {
		CurrentVel = TargetVel.Mul(20)
	}

	newState.X = state.X + CurrentVel[0]
	newState.Y = state.Y + CurrentVel[1]
	newState.Z = move.Z
	newState.VelX = CurrentVel[0]
	newState.VelY = CurrentVel[1]
	newState.SequenceNumber = state.SequenceNumber + 1

	return newState
}
