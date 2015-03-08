package main

import (
	"errors"
	"fmt"
	"math"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/shurcooL/gogl"
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

	program                 *gogl.Program
	pMatrixUniform          *gogl.UniformLocation
	mvMatrixUniform         *gogl.UniformLocation
	vertexPositionBuffer    *gogl.Buffer
	vertexPositionAttribute int
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
	vertexShader := gl.CreateShader(gl.VERTEX_SHADER)
	gl.ShaderSource(vertexShader, characterVertexSource)
	gl.CompileShader(vertexShader)
	defer gl.DeleteShader(vertexShader)

	if !gl.GetShaderParameterb(vertexShader, gl.COMPILE_STATUS) {
		return errors.New("COMPILE_STATUS: " + gl.GetShaderInfoLog(vertexShader))
	}

	fragmentShader := gl.CreateShader(gl.FRAGMENT_SHADER)
	gl.ShaderSource(fragmentShader, characterFragmentSource)
	gl.CompileShader(fragmentShader)
	defer gl.DeleteShader(fragmentShader)

	if !gl.GetShaderParameterb(fragmentShader, gl.COMPILE_STATUS) {
		return errors.New("COMPILE_STATUS: " + gl.GetShaderInfoLog(fragmentShader))
	}

	l.program = gl.CreateProgram()
	gl.AttachShader(l.program, vertexShader)
	gl.AttachShader(l.program, fragmentShader)

	gl.LinkProgram(l.program)
	if !gl.GetProgramParameterb(l.program, gl.LINK_STATUS) {
		return errors.New("LINK_STATUS: " + gl.GetProgramInfoLog(l.program))
	}

	gl.ValidateProgram(l.program)
	if !gl.GetProgramParameterb(l.program, gl.VALIDATE_STATUS) {
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

	gl.BufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW)

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
