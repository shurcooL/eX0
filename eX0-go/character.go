package main

import (
	"encoding/binary"
	"errors"
	"fmt"
	"math"

	"github.com/go-gl/mathgl/mgl32"
	"golang.org/x/mobile/f32"
	"golang.org/x/mobile/gl"
	"golang.org/x/mobile/gl/glutil"
)

func newCharacter() (*character, error) {
	l := new(character)

	err := l.initShaders()
	if err != nil {
		return nil, err
	}
	err = l.createVbo()
	if err != nil {
		return nil, err
	}

	return l, nil
}

type character struct {
	pos mgl32.Vec2
	Vel mgl32.Vec2
	Z   float32

	vertexCount int

	program                 gl.Program
	pMatrixUniform          gl.Uniform
	mvMatrixUniform         gl.Uniform
	vertexPositionBuffer    gl.Buffer
	vertexPositionAttribute gl.Attrib
}

const (
	characterVertexSource = `//#version 120 // OpenGL 2.1.
//#version 100 // WebGL.

attribute vec3 aVertexPosition;

uniform mat4 uMVMatrix;
uniform mat4 uPMatrix;

void main() {
	gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);
}
`
	characterFragmentSource = `//#version 120 // OpenGL 2.1.
//#version 100 // WebGL.

void main() {
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
`
)

func (l *character) initShaders() error {
	var err error
	l.program, err = glutil.CreateProgram(characterVertexSource, characterFragmentSource)
	if err != nil {
		return err
	}

	gl.ValidateProgram(l.program)
	if gl.GetProgrami(l.program, gl.VALIDATE_STATUS) != gl.TRUE {
		return errors.New("VALIDATE_STATUS: " + gl.GetProgramInfoLog(l.program))
	}

	gl.UseProgram(l.program)

	l.pMatrixUniform = gl.GetUniformLocation(l.program, "uPMatrix")
	l.mvMatrixUniform = gl.GetUniformLocation(l.program, "uMVMatrix")

	if glError := gl.GetError(); glError != 0 {
		return fmt.Errorf("gl.GetError: %v", glError)
	}

	return nil
}

func (l *character) createVbo() error {
	l.vertexPositionBuffer = gl.CreateBuffer()
	gl.BindBuffer(gl.ARRAY_BUFFER, l.vertexPositionBuffer)

	var vertices []float32 = drawCircleBorderCustom(mgl32.Vec2{}, mgl32.Vec2{16, 16}, 2, 12, 1, 11)
	l.vertexCount = len(vertices) / 2

	vertices = append(vertices, -1, (3 + 10))
	vertices = append(vertices, -1, (3 - 1))
	vertices = append(vertices, 1, (3 - 1))
	vertices = append(vertices, 1, (3 + 10))

	gl.BufferData(gl.ARRAY_BUFFER, f32.Bytes(binary.LittleEndian, vertices...), gl.STATIC_DRAW)

	l.vertexPositionAttribute = gl.GetAttribLocation(l.program, "aVertexPosition")
	gl.EnableVertexAttribArray(l.vertexPositionAttribute)

	if glError := gl.GetError(); glError != 0 {
		return fmt.Errorf("gl.GetError: %v", glError)
	}

	return nil
}

func (l *character) setup() {
	gl.UseProgram(l.program)
	gl.BindBuffer(gl.ARRAY_BUFFER, l.vertexPositionBuffer)

	gl.VertexAttribPointer(l.vertexPositionAttribute, 2, gl.FLOAT, false, 0, 0)
}

func (l *character) render() {
	var first int

	count := l.vertexCount
	gl.DrawArrays(gl.TRIANGLE_STRIP, first, count)
	first += count

	count = 4
	gl.DrawArrays(gl.TRIANGLE_FAN, first, count)
	first += count
}

// ---

func drawCircleBorderCustom(pos mgl32.Vec2, size mgl32.Vec2, borderWidth float32, totalSlices, startSlice, endSlice int32) (vertices []float32) {
	var x = float64(totalSlices)
	for i := startSlice; i <= endSlice; i++ {
		vertices = append(vertices, float32(pos[0]+float32(math.Sin(Tau*float64(i)/x))*size[0]/2), float32(pos[1]+float32(math.Cos(Tau*float64(i)/x))*size[1]/2))
		vertices = append(vertices, float32(pos[0]+float32(math.Sin(Tau*float64(i)/x))*(size[0]/2-borderWidth)), float32(pos[1]+float32(math.Cos(Tau*float64(i)/x))*(size[1]/2-borderWidth)))
	}
	return vertices
}
