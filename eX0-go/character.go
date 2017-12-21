package main

import (
	"fmt"
	"math"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/goxjs/gl"
	"github.com/shurcooL/eX0/eX0-go/packet"
)

type character struct {
	vertexCount int

	program                 gl.Program
	pMatrixUniform          gl.Uniform
	mvMatrixUniform         gl.Uniform
	colorUniform            gl.Uniform
	vertexPositionAttribute gl.Attrib
	vertexPositionBuffer    gl.Buffer
}

func newCharacter() (*character, error) {
	l := new(character)

	const (
		vertexSource = `//#version 120 // OpenGL 2.1.
//#version 100 // WebGL.

attribute vec3 aVertexPosition;

uniform mat4 uMVMatrix;
uniform mat4 uPMatrix;

void main() {
	gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);
}
`
		fragmentSource = `//#version 120 // OpenGL 2.1.
//#version 100 // WebGL.

#ifdef GL_ES
	precision lowp float;
#endif

uniform vec3 uColor;

void main() {
	gl_FragColor = vec4(uColor, 1.0);
}
`
	)

	var err error
	l.program, err = createValidateUseProgram(vertexSource, fragmentSource)
	if err != nil {
		return nil, err
	}

	l.pMatrixUniform = gl.GetUniformLocation(l.program, "uPMatrix")
	l.mvMatrixUniform = gl.GetUniformLocation(l.program, "uMVMatrix")
	l.colorUniform = gl.GetUniformLocation(l.program, "uColor")

	l.vertexPositionAttribute = gl.GetAttribLocation(l.program, "aVertexPosition")

	// Circle border, then 4 vertices for the gun quad.
	positionData := circleBorder(mgl32.Vec2{}, mgl32.Vec2{16, 16}, 2, 12, 1, 11)
	positionData = append(positionData, -1, (3 + 10))
	positionData = append(positionData, -1, (3 - 1))
	positionData = append(positionData, 1, (3 - 1))
	positionData = append(positionData, 1, (3 + 10))
	l.vertexCount = len(positionData) / 2
	l.vertexPositionBuffer = createVbo3Float(positionData)

	if err := gl.GetError(); err != 0 {
		return nil, fmt.Errorf("gl.GetError: %v", err)
	}
	return l, nil
}

func (l *character) setup() {
	gl.UseProgram(l.program)

	gl.BindBuffer(gl.ARRAY_BUFFER, l.vertexPositionBuffer)
	gl.EnableVertexAttribArray(l.vertexPositionAttribute)
	gl.VertexAttribPointer(l.vertexPositionAttribute, 2, gl.FLOAT, false, 0, 0)
}
func (l *character) cleanup() {
	gl.DisableVertexAttribArray(l.vertexPositionAttribute)
}

func (l *character) render(team packet.Team, dead bool) {
	switch {
	case !dead && team == packet.Red:
		gl.Uniform3f(l.colorUniform, 1, 0, 0)
	case !dead && team == packet.Blue:
		gl.Uniform3f(l.colorUniform, 0, 0, 1)
	case dead:
		gl.Uniform3f(l.colorUniform, 0, 0, 0)
	}
	gl.DrawArrays(gl.TRIANGLE_STRIP, 0, l.vertexCount-4) // Circle border.
	gl.DrawArrays(gl.TRIANGLE_FAN, l.vertexCount-4, 4)   // Gun quad.
}

func circleBorder(pos mgl32.Vec2, size mgl32.Vec2, borderWidth float32, slices, startSlice, endSlice int) (vertices []float32) {
	x := float64(slices)
	for i := startSlice; i <= endSlice; i++ {
		vertices = append(vertices,
			pos[0]+float32(math.Sin(Tau*float64(i)/x))*size[0]/2,
			pos[1]+float32(math.Cos(Tau*float64(i)/x))*size[1]/2)
		vertices = append(vertices,
			pos[0]+float32(math.Sin(Tau*float64(i)/x))*(size[0]/2-borderWidth),
			pos[1]+float32(math.Cos(Tau*float64(i)/x))*(size[1]/2-borderWidth))
	}
	return vertices
}
