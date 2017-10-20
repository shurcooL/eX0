package main

import (
	"errors"
	"fmt"

	"github.com/goxjs/gl"
	"github.com/goxjs/gl/glutil"
	"github.com/goxjs/glfw"
	"github.com/shurcooL/eX0/eX0-go/gpc"
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
	vertexPositionAttribute gl.Attrib
	vertexPositionBuffer    gl.Buffer
}

// initGraphics prepares level for rendering.
func (l *level) initGraphics() error {
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
	l.program, err = createValidateUseProgram(vertexSource, fragmentSource)
	if err != nil {
		return err
	}

	l.pMatrixUniform = gl.GetUniformLocation(l.program, "uPMatrix")
	l.mvMatrixUniform = gl.GetUniformLocation(l.program, "uMVMatrix")

	l.vertexPositionAttribute = gl.GetAttribLocation(l.program, "aVertexPosition")
	gl.EnableVertexAttribArray(l.vertexPositionAttribute)

	var positionData []float32
	for _, contour := range l.polygon.Contours {
		for _, vertex := range contour.Vertices {
			positionData = append(positionData, float32(vertex[0]), float32(vertex[1]))
		}
	}
	l.vertexPositionBuffer = createVbo3Float(positionData)

	if err := gl.GetError(); err != 0 {
		return fmt.Errorf("gl.GetError: %v", err)
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

// createValidateUseProgram creates, compiles, links, validates, and uses a gl.Program.
func createValidateUseProgram(vertexSource, fragmentSource string) (gl.Program, error) {
	p, err := glutil.CreateProgram(vertexSource, fragmentSource)
	if err != nil {
		return p, err
	}
	gl.ValidateProgram(p)
	if gl.GetProgrami(p, gl.VALIDATE_STATUS) != gl.TRUE {
		return p, errors.New("VALIDATE_STATUS: " + gl.GetProgramInfoLog(p))
	}
	gl.UseProgram(p)
	if err := gl.GetError(); err != 0 {
		return p, fmt.Errorf("gl.GetError: %v", err)
	}
	return p, nil
}
