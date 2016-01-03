package main

import (
	"github.com/go-gl/mathgl/mgl32"
	"github.com/shurcooL/eX0/eX0-go/packet"
)

type CameraI interface {
	CalculateForFrame()
	ModelView() mgl32.Mat4
}

// FreeCamera is a camera that is not attached to any player. It can be scolled around the level.
type FreeCamera struct {
	pos [2]float32
}

func (*FreeCamera) CalculateForFrame() {}

func (c *FreeCamera) ModelView() mgl32.Mat4 {
	return mgl32.Translate3D(c.pos[0], c.pos[1], 0)
}

// PlayerCamera is a camera attached to a player.
type PlayerCamera struct {
	logic    *logic
	playerID uint8

	pos playerPosVel
}

func (c *PlayerCamera) CalculateForFrame() {
	c.logic.playersStateMu.Lock()
	ps := c.logic.playersState[c.playerID]
	if (ps.conn != nil && ps.conn.JoinStatus < IN_GAME) || ps.Team == packet.Spectator {
		c.logic.playersStateMu.Unlock()
		return
	}
	c.pos = ps.Interpolated(c.logic, c.playerID)
	c.logic.playersStateMu.Unlock()
}

func (c *PlayerCamera) ModelView() mgl32.Mat4 {
	mat := mgl32.Ident4()
	mat = mat.Mul4(mgl32.Translate3D(float32(components.view.windowSize[0])/2, float32(components.view.windowSize[1])/20, 0))
	mat = mat.Mul4(mgl32.HomogRotate3DZ(c.pos.Z))
	mat = mat.Mul4(mgl32.Translate3D(-c.pos.X, -c.pos.Y, 0))
	return mat
}
