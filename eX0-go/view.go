package main

import (
	"fmt"
	"log"
	"time"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/goxjs/gl"
	"github.com/goxjs/glfw"
	"github.com/shurcooL/eX0/eX0-go/packet"
	"github.com/shurcooL/eX0/eX0-go/render"
)

type view struct {
	logic *logic

	windowSize [2]int

	activeCamera int
	cameras      []CameraI
}

func newView(logic *logic) *view {
	return &view{
		logic:        logic,
		windowSize:   [2]int{640, 480},
		activeCamera: 0,
		cameras:      []CameraI{&FreeCamera{pos: [2]float32{25, 140}}},
	}
}

func (v *view) initAndMainLoop() {
	err := glfw.Init(gl.ContextWatcher)
	if err != nil {
		panic(err)
	}
	defer glfw.Terminate()

	glfw.WindowHint(glfw.Samples, 8) // Anti-aliasing.

	window, err := glfw.CreateWindow(v.windowSize[0], v.windowSize[1], "eX0-go", nil, nil)
	if err != nil {
		panic(err)
	}
	window.MakeContextCurrent()

	glfw.SwapInterval(1) // Vsync.

	gl.ClearColor(227.0/255, 189.0/255, 162.0/255, 1)
	gl.Clear(gl.COLOR_BUFFER_BIT)

	framebufferSizeCallback := func(w *glfw.Window, framebufferSize0, framebufferSize1 int) {
		gl.Viewport(0, 0, framebufferSize0, framebufferSize1)

		v.windowSize[0], v.windowSize[1] = w.GetSize()
	}
	{
		var framebufferSize [2]int
		framebufferSize[0], framebufferSize[1] = window.GetFramebufferSize()
		framebufferSizeCallback(window, framebufferSize[0], framebufferSize[1])
	}
	window.SetFramebufferSizeCallback(framebufferSizeCallback)

	window.SetKeyCallback(func(w *glfw.Window, key glfw.Key, scancode int, action glfw.Action, mods glfw.ModifierKey) {
		if action == glfw.Press {
			switch key {
			case glfw.KeyEscape:
				window.SetShouldClose(true)
			case glfw.KeyV:
				v.activeCamera = (v.activeCamera + 1) % len(v.cameras)
			case glfw.Key0:
				v.cameras[0].(*FreeCamera).zoom = 0
			}
		}
		if action != glfw.Release {
			switch key {
			case glfw.KeyEqual:
				v.cameras[0].(*FreeCamera).zoom++
			case glfw.KeyMinus:
				v.cameras[0].(*FreeCamera).zoom--
			}
		}
	})
	window.SetScrollCallback(func(_ *glfw.Window, xoff, yoff float64) {
		// TODO: A better mechanism to feed input events stream to active camera.
		switch v.activeCamera {
		case 0:
			scale := v.cameras[0].(*FreeCamera).Scale()
			v.cameras[0].(*FreeCamera).pos[0] += float32(xoff) * 5 / scale
			v.cameras[0].(*FreeCamera).pos[1] -= float32(yoff) * 5 / scale
		}
	})

	err = v.logic.level.initGraphics()
	if err != nil {
		panic(err)
	}

	c, err := newCharacter()
	if err != nil {
		panic(err)
	}

	shadow, err := newCharacterShadow()
	if err != nil {
		panic(err)
	}

	particle, err := render.NewParticle()
	if err != nil {
		panic(err)
	}

	gl.Enable(gl.CULL_FACE)
	gl.BlendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)

	if components.client != nil {
		v.logic.Input <- func() packet.Move { return inputCommand(window) }

		// Create and set player-centric camera.
		v.cameras = append(v.cameras, &PlayerCamera{logic: v.logic, playerID: components.client.playerID})
		v.activeCamera = len(v.cameras) - 1
	}

	frameStarted := time.Now()

	for !window.ShouldClose() {
		timePassed := time.Since(frameStarted)
		frameStarted = time.Now()

		state.Lock() // For logic.started.
		gameMoment := gameMoment(frameStarted.Sub(v.logic.started).Seconds())
		state.Unlock()

		glfw.PollEvents()

		if components.client != nil {
			components.client.TargetZMu.Lock()
			if (window.GetKey(glfw.KeyLeft) != glfw.Release) && !(window.GetKey(glfw.KeyRight) != glfw.Release) {
				components.client.TargetZ -= 2.0 * float32(timePassed.Seconds())
			} else if (window.GetKey(glfw.KeyRight) != glfw.Release) && !(window.GetKey(glfw.KeyLeft) != glfw.Release) {
				components.client.TargetZ += 2.0 * float32(timePassed.Seconds())
			}
			components.client.TargetZMu.Unlock()
		}

		gl.Clear(gl.COLOR_BUFFER_BIT)

		// Calculate camera and player positions for this frame.
		var players []visiblePlayer
		state.Lock()
		v.logic.playersStateMu.Lock()
		v.cameras[v.activeCamera].CalculateForFrame(gameMoment)
		for id, ps := range v.logic.playersState {
			if ps.conn != nil && ps.conn.JoinStatus < IN_GAME { // TODO: Fix JoinStatus race with line server.go:719.
				continue
			}
			if ps.Team == packet.Spectator {
				continue
			}

			pos := ps.InterpolatedOrDead(gameMoment, id)

			mvMatrix := v.cameras[v.activeCamera].ModelView()
			mvMatrix = mvMatrix.Mul4(mgl32.Translate3D(pos.X, pos.Y, 0))
			mvMatrix = mvMatrix.Mul4(mgl32.HomogRotate3DZ(-pos.Z))

			players = append(players, visiblePlayer{MV: mvMatrix, Team: ps.Team, Dead: ps.Health == 0})
		}
		v.logic.playersStateMu.Unlock()
		state.Unlock()

		pMatrix := mgl32.Ortho2D(0, float32(v.windowSize[0]), 0, float32(v.windowSize[1]))
		mvMatrix := v.cameras[v.activeCamera].ModelView()

		// Render level.
		v.logic.level.setup()
		gl.UniformMatrix4fv(v.logic.level.pMatrixUniform, pMatrix[:])
		gl.UniformMatrix4fv(v.logic.level.mvMatrixUniform, mvMatrix[:])
		v.logic.level.render()
		v.logic.level.cleanup()

		// Render player shadows.
		shadow.setup()
		gl.UniformMatrix4fv(shadow.pMatrixUniform, pMatrix[:])
		for _, p := range players {
			gl.UniformMatrix4fv(shadow.mvMatrixUniform, p.MV[:])
			shadow.render()
		}
		shadow.cleanup()

		// Render players.
		c.setup()
		gl.UniformMatrix4fv(c.pMatrixUniform, pMatrix[:])
		for _, p := range players {
			gl.UniformMatrix4fv(c.mvMatrixUniform, p.MV[:])
			c.render(p.Team, p.Dead)
		}
		c.cleanup()

		// Render particles.
		particle.Bind()
		particle.SetProjection(pMatrix[:])
		v.logic.particleSystem.Lock()
		for _, p := range v.logic.particleSystem.particles {
			mv := mvMatrix
			mv = mv.Mul4(mgl32.Translate3D(p.Position.X(), p.Position.Y(), 0))
			particle.SetModelView(mv[:])
			particle.Render()
		}
		v.logic.particleSystem.Unlock()
		particle.Unbind()

		window.SwapBuffers()

		if err := gl.GetError(); err != 0 {
			log.Println(fmt.Errorf("gl.GetError: %v", err))
		}
	}

	if components.client != nil {
		// Take away logic's access to window before terminating GLFW.
		v.logic.Input <- nil
	}
}

type visiblePlayer struct {
	MV   mgl32.Mat4 // Model-view matrix for player position.
	Team packet.Team
	Dead bool
}
