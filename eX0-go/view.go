package main

import (
	"time"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/goxjs/gl"
	"github.com/goxjs/glfw"
)

var windowSize = [2]int{640, 480}

var cameraX, cameraY float64 = 362, 340

func view(runClientAndGameLogic bool) {
	err := glfw.Init(gl.ContextWatcher)
	if err != nil {
		panic(err)
	}
	defer glfw.Terminate()

	glfw.WindowHint(glfw.Samples, 8) // Anti-aliasing.

	window, err := glfw.CreateWindow(windowSize[0], windowSize[1], "eX0-go", nil, nil)
	if err != nil {
		panic(err)
	}
	window.MakeContextCurrent()

	gl.ClearColor(227/255.0, 189/255.0, 162/255.0, 1)
	gl.Clear(gl.COLOR_BUFFER_BIT)

	window.SetScrollCallback(func(_ *glfw.Window, xoff, yoff float64) {
		cameraX += xoff * 5
		cameraY -= yoff * 5
	})

	framebufferSizeCallback := func(w *glfw.Window, framebufferSize0, framebufferSize1 int) {
		gl.Viewport(0, 0, framebufferSize0, framebufferSize1)

		windowSize[0], windowSize[1] = w.GetSize()
	}
	{
		var framebufferSize [2]int
		framebufferSize[0], framebufferSize[1] = window.GetFramebufferSize()
		framebufferSizeCallback(window, framebufferSize[0], framebufferSize[1])
	}
	window.SetFramebufferSizeCallback(framebufferSizeCallback)

	l, err := newLevel("test3.wwl")
	if err != nil {
		panic(err)
	}

	c, err := newCharacter()
	if err != nil {
		panic(err)
	}

	if runClientAndGameLogic {
		components.client = startClient()

		// TODO: Use components.logic = startLogic().
		{
			state.session.GlobalStateSequenceNumberTEST = 0
			state.session.NextTickTime = time.Since(startedProcess).Seconds()
			go gameLogic(func() { input(window) })
		}
	}

	for !window.ShouldClose() {
		glfw.PollEvents()

		gl.Clear(gl.COLOR_BUFFER_BIT)

		pMatrix := mgl32.Ortho2D(0, float32(windowSize[0]), 0, float32(windowSize[1]))
		mvMatrix := mgl32.Translate3D(float32(cameraX), float32(cameraY), 0)

		l.setup()
		gl.UniformMatrix4fv(l.pMatrixUniform, pMatrix[:])
		gl.UniformMatrix4fv(l.mvMatrixUniform, mvMatrix[:])
		l.render()

		playersStateMu.Lock()
		for _, ps := range playersState {
			mvMatrix = mgl32.Translate3D(float32(cameraX), float32(cameraY), 0)
			mvMatrix = mvMatrix.Mul4(mgl32.Translate3D(ps.X, ps.Y, 0))
			mvMatrix = mvMatrix.Mul4(mgl32.HomogRotate3DZ(-ps.Z))

			c.setup()
			gl.UniformMatrix4fv(c.pMatrixUniform, pMatrix[:])
			gl.UniformMatrix4fv(c.mvMatrixUniform, mvMatrix[:])
			c.render()
		}
		playersStateMu.Unlock()

		window.SwapBuffers()
	}
}
