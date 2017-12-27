package main

import (
	"math"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/shurcooL/eX0/eX0-go/packet"
)

type CameraI interface {
	CalculateForFrame(gameMoment)
	ModelView() mgl32.Mat4
}

// FreeCamera is a camera that is not attached to any player. It can be scolled around the level.
type FreeCamera struct {
	pos  [2]float32
	zoom float32
}

func (*FreeCamera) CalculateForFrame(gameMoment) {}

func (c *FreeCamera) ModelView() mgl32.Mat4 {
	s := c.Scale()
	return mgl32.Translate3D(float32(components.view.windowSize[0])/2, float32(components.view.windowSize[1])/2, 0).
		Mul4(mgl32.Scale3D(s, s, s)).
		Mul4(mgl32.Translate3D(c.pos[0], c.pos[1], 0))
}

func (c *FreeCamera) Scale() float32 {
	return float32(math.Pow(1.0+0.1*float64(c.zoom), 3))
}

// PlayerCamera is a camera attached to a player.
type PlayerCamera struct {
	logic    *logic
	playerID uint8

	pos playerPosVel
}

func (c *PlayerCamera) CalculateForFrame(gameMoment gameMoment) {
	c.logic.playersStateMu.Lock()
	ps := c.logic.playersState[c.playerID]
	if (ps.conn != nil && ps.conn.JoinStatus < IN_GAME) || ps.Team == packet.Spectator {
		c.logic.playersStateMu.Unlock()
		return
	}
	// TODO: Consider using same position as calculated for all players
	//       during "Calculate player positions for this frame" step,
	//       instead of our own copy (which might not match).
	//       This is better now that we're using same gameMoment.
	c.pos = ps.InterpolatedOrDead(gameMoment, c.playerID)
	c.logic.playersStateMu.Unlock()
}

func (c *PlayerCamera) ModelView() mgl32.Mat4 {
	return mgl32.Translate3D(float32(components.view.windowSize[0])/2, float32(components.view.windowSize[1])/20, 0).
		Mul4(mgl32.HomogRotate3DZ(c.pos.Z)).
		Mul4(mgl32.Translate3D(-c.pos.X, -c.pos.Y, 0))
}
