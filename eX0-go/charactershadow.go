package main

import (
	"encoding/binary"
	"fmt"
	"math"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/goxjs/gl"
	"golang.org/x/mobile/exp/f32"
)

type characterShadow struct {
	vertexCount int

	program                 gl.Program
	pMatrixUniform          gl.Uniform
	mvMatrixUniform         gl.Uniform
	vertexPositionAttribute gl.Attrib
	vertexAlphaAttribute    gl.Attrib
	vertexPositionBuffer    gl.Buffer
	vertexAlphaBuffer       gl.Buffer
}

func newCharacterShadow() (*characterShadow, error) {
	l := new(characterShadow)

	const (
		vertexSource = `//#version 120 // OpenGL 2.1.
//#version 100 // WebGL.

uniform mat4 uMVMatrix;
uniform mat4 uPMatrix;

attribute vec3 aVertexPosition;
attribute float aVertexAlpha;

varying float vPixelAlpha;

void main() {
	vPixelAlpha = aVertexAlpha;
	gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);
}
`
		fragmentSource = `//#version 120 // OpenGL 2.1.
//#version 100 // WebGL.

#ifdef GL_ES
	precision lowp float;
#endif

varying float vPixelAlpha;

void main() {
	gl_FragColor = vec4(0, 0, 0, vPixelAlpha);
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

	l.vertexPositionAttribute = gl.GetAttribLocation(l.program, "aVertexPosition")
	gl.EnableVertexAttribArray(l.vertexPositionAttribute)
	l.vertexAlphaAttribute = gl.GetAttribLocation(l.program, "aVertexAlpha")
	gl.EnableVertexAttribArray(l.vertexAlphaAttribute)

	positionData := append([]float32{0, 0}, circle(mgl32.Vec2{}, mgl32.Vec2{16, 16}.Mul(1.75), 16)...)
	l.vertexCount = len(positionData) / 2
	l.vertexPositionBuffer = createVbo3Float(positionData)

	alphaData := make([]uint8, l.vertexCount)
	alphaData[0] = 76 // First vertex is the circle center, it has non-zero alpha. The rest are zero.
	l.vertexAlphaBuffer = createVbo3Ubyte(alphaData)

	if err := gl.GetError(); err != 0 {
		return nil, fmt.Errorf("gl.GetError: %v", err)
	}
	return l, nil
}

func (l *characterShadow) setup() {
	gl.UseProgram(l.program)

	gl.BindBuffer(gl.ARRAY_BUFFER, l.vertexPositionBuffer)
	gl.VertexAttribPointer(l.vertexPositionAttribute, 2, gl.FLOAT, false, 0, 0)

	gl.BindBuffer(gl.ARRAY_BUFFER, l.vertexAlphaBuffer)
	gl.VertexAttribPointer(l.vertexAlphaAttribute, 1, gl.UNSIGNED_BYTE, true, 0, 0)
}

func (l *characterShadow) render() {
	gl.Enable(gl.BLEND)
	gl.DrawArrays(gl.TRIANGLE_FAN, 0, l.vertexCount)
	gl.Disable(gl.BLEND)
}

func circle(pos mgl32.Vec2, size mgl32.Vec2, slices int) (vertices []float32) {
	x := float64(slices)
	for i := slices; i >= 0; i-- {
		vertices = append(vertices,
			pos[0]+float32(math.Sin(Tau*float64(i)/x))*size[0]/2,
			pos[1]+float32(math.Cos(Tau*float64(i)/x))*size[1]/2)
	}
	return vertices
}

func createVbo3Float(data []float32) gl.Buffer {
	vbo := gl.CreateBuffer()
	gl.BindBuffer(gl.ARRAY_BUFFER, vbo)
	gl.BufferData(gl.ARRAY_BUFFER, f32.Bytes(binary.LittleEndian, data...), gl.STATIC_DRAW)
	return vbo
}

func createVbo3Ubyte(data []uint8) gl.Buffer {
	vbo := gl.CreateBuffer()
	gl.BindBuffer(gl.ARRAY_BUFFER, vbo)
	gl.BufferData(gl.ARRAY_BUFFER, data, gl.STATIC_DRAW)
	return vbo
}
