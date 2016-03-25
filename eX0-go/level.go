package main

import (
	"encoding/binary"
	"errors"
	"fmt"

	"github.com/goxjs/gl"
	"github.com/goxjs/gl/glutil"
	"github.com/goxjs/glfw"
	"github.com/shurcooL/eX0/eX0-go/gpc"
	"golang.org/x/mobile/exp/f32"
)

func newLevel(name string) (*level, error) {
	f, err := glfw.Open(name)
	if err != nil {
		return nil, err
	}
	polygon, err := gpc.Parse(f)
	f.Close()
	if err != nil {
		return nil, err
	}
	return &level{polygon: polygon}, nil
}

type level struct {
	polygon gpc.Polygon

	program                 gl.Program
	pMatrixUniform          gl.Uniform
	mvMatrixUniform         gl.Uniform
	vertexPositionBuffer    gl.Buffer
	vertexPositionAttribute gl.Attrib
}

// initGraphics prepares level for rendering.
func (l *level) initGraphics() error {
	if err := l.initShaders(); err != nil {
		return err
	}
	if err := l.createVbo(); err != nil {
		return err
	}
	return nil
}

func (l *level) initShaders() error {
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

void main() {
	gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
`
	)

	var err error
	l.program, err = glutil.CreateProgram(vertexSource, fragmentSource)
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

func (l *level) createVbo() error {
	l.vertexPositionBuffer = gl.CreateBuffer()
	gl.BindBuffer(gl.ARRAY_BUFFER, l.vertexPositionBuffer)
	var vertices []float32
	for _, contour := range l.polygon.Contours {
		for _, vertex := range contour.Vertices {
			vertices = append(vertices, float32(vertex[0]), float32(vertex[1]))
		}
	}
	gl.BufferData(gl.ARRAY_BUFFER, f32.Bytes(binary.LittleEndian, vertices...), gl.STATIC_DRAW)

	l.vertexPositionAttribute = gl.GetAttribLocation(l.program, "aVertexPosition")
	gl.EnableVertexAttribArray(l.vertexPositionAttribute)

	if glError := gl.GetError(); glError != 0 {
		return fmt.Errorf("gl.GetError: %v", glError)
	}

	return nil
}

func (l *level) setup() {
	gl.UseProgram(l.program)
	gl.BindBuffer(gl.ARRAY_BUFFER, l.vertexPositionBuffer)

	gl.VertexAttribPointer(l.vertexPositionAttribute, 2, gl.FLOAT, false, 0, 0)
}

func (l *level) render() {
	var first int
	for _, contour := range l.polygon.Contours {
		count := len(contour.Vertices)
		gl.DrawArrays(gl.LINE_LOOP, first, count)
		first += count
	}
}
