package main

import (
	"time"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/goxjs/gl"
	"github.com/goxjs/glfw"
	"github.com/shurcooL/eX0/eX0-go/packet"
)

type view struct {
	logic *logic

	windowSize [2]int

	cameraPos [2]float32
}

func runView(logic *logic, gameLogicInput bool) *view {
	v := &view{
		logic:      logic,
		windowSize: [2]int{640, 480},
		cameraPos:  [2]float32{362, 340},
	}
	v.initAndMainLoop(gameLogicInput)
	return v
}

func (v *view) initAndMainLoop(gameLogicInput bool) {
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

	gl.ClearColor(227/255.0, 189/255.0, 162/255.0, 1)
	gl.Clear(gl.COLOR_BUFFER_BIT)

	window.SetScrollCallback(func(_ *glfw.Window, xoff, yoff float64) {
		v.cameraPos[0] += float32(xoff) * 5
		v.cameraPos[1] -= float32(yoff) * 5
	})

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

	err = v.logic.level.initGraphics()
	if err != nil {
		panic(err)
	}

	c, err := newCharacter()
	if err != nil {
		panic(err)
	}

	if gameLogicInput {
		v.logic.Input <- func(logic *logic) packet.Move { return inputCommand(logic, window) }
	}

	frameStarted := time.Now()

	for !window.ShouldClose() {
		timePassed := time.Since(frameStarted)
		frameStarted = time.Now()

		glfw.PollEvents()

		if gameLogicInput {
			if (window.GetKey(glfw.KeyLeft) != glfw.Release) && !(window.GetKey(glfw.KeyRight) != glfw.Release) {
				components.client.ZOffset -= 2.0 * float32(timePassed.Seconds())
			} else if (window.GetKey(glfw.KeyRight) != glfw.Release) && !(window.GetKey(glfw.KeyLeft) != glfw.Release) {
				components.client.ZOffset += 2.0 * float32(timePassed.Seconds())
			}
		}

		gl.Clear(gl.COLOR_BUFFER_BIT)

		pMatrix := mgl32.Ortho2D(0, float32(v.windowSize[0]), 0, float32(v.windowSize[1]))
		mvMatrix := mgl32.Translate3D(v.cameraPos[0], v.cameraPos[1], 0)

		v.logic.level.setup()
		gl.UniformMatrix4fv(v.logic.level.pMatrixUniform, pMatrix[:])
		gl.UniformMatrix4fv(v.logic.level.mvMatrixUniform, mvMatrix[:])
		v.logic.level.render()

		state.Lock()
		v.logic.playersStateMu.Lock()
		for id, ps := range v.logic.playersState {
			if ps.conn != nil && ps.conn.JoinStatus < IN_GAME {
				continue
			}
			if ps.Team == packet.Spectator {
				continue
			}

			pos := ps.Interpolated(v.logic, uint8(id))

			mvMatrix = mgl32.Translate3D(v.cameraPos[0], v.cameraPos[1], 0)
			mvMatrix = mvMatrix.Mul4(mgl32.Translate3D(pos.X, pos.Y, 0))
			mvMatrix = mvMatrix.Mul4(mgl32.HomogRotate3DZ(-pos.Z))

			c.setup()
			gl.UniformMatrix4fv(c.pMatrixUniform, pMatrix[:])
			gl.UniformMatrix4fv(c.mvMatrixUniform, mvMatrix[:])
			c.render(ps.Team)
		}
		v.logic.playersStateMu.Unlock()
		state.Unlock()

		window.SwapBuffers()
	}
}
