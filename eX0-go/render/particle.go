// Package render provides functionality to render eX0 objects.
package render

import (
	"encoding/binary"
	"errors"
	"fmt"

	"github.com/goxjs/gl"
	"github.com/goxjs/gl/glutil"
	"golang.org/x/mobile/exp/f32"
)

type Particle struct {
	program                 gl.Program
	pMatrixUniform          gl.Uniform
	mvMatrixUniform         gl.Uniform
	vertexPositionAttribute gl.Attrib
	vertexPositionBuffer    gl.Buffer
}

func NewParticle() (*Particle, error) {
	const (
		vertexSource = `//#version 120 // OpenGL 2.1.
//#version 100 // WebGL.

uniform mat4 uMVMatrix;
uniform mat4 uPMatrix;

attribute vec3 aVertexPosition;

void main() {
	gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);
}
`
		fragmentSource = `//#version 120 // OpenGL 2.1.
//#version 100 // WebGL.

#ifdef GL_ES
	precision lowp float;
#endif

void main() {
	gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}
`
	)

	p := new(Particle)
	var err error
	p.program, err = createValidateUseProgram(vertexSource, fragmentSource)
	if err != nil {
		return nil, err
	}
	p.pMatrixUniform = gl.GetUniformLocation(p.program, "uPMatrix")
	p.mvMatrixUniform = gl.GetUniformLocation(p.program, "uMVMatrix")
	p.vertexPositionAttribute = gl.GetAttribLocation(p.program, "aVertexPosition")

	p.vertexPositionBuffer = gl.CreateBuffer()
	gl.BindBuffer(gl.ARRAY_BUFFER, p.vertexPositionBuffer)
	gl.BufferData(gl.ARRAY_BUFFER, f32.Bytes(binary.LittleEndian, 0, 0), gl.STATIC_DRAW)

	if code := gl.GetError(); code != 0 {
		return nil, fmt.Errorf("GL error %d", code)
	}
	return p, nil
}

func (p *Particle) Bind() {
	gl.UseProgram(p.program)

	gl.BindBuffer(gl.ARRAY_BUFFER, p.vertexPositionBuffer)
	gl.EnableVertexAttribArray(p.vertexPositionAttribute)
	gl.VertexAttribPointer(p.vertexPositionAttribute, 2, gl.FLOAT, false, 0, 0)
}
func (p *Particle) Unbind() {
	gl.DisableVertexAttribArray(p.vertexPositionAttribute)
}

func (p *Particle) SetProjection(projection []float32) {
	gl.UniformMatrix4fv(p.pMatrixUniform, projection)
}
func (p *Particle) SetModelView(mv []float32) {
	gl.UniformMatrix4fv(p.mvMatrixUniform, mv)
}

func (p *Particle) Render() {
	gl.DrawArrays(gl.POINTS, 0, 1)
}

// createValidateUseProgram creates, compiles, links, validates, and uses a gl.Program.
func createValidateUseProgram(vertexSource, fragmentSource string) (gl.Program, error) {
	p, err := glutil.CreateProgram(vertexSource, fragmentSource)
	if err != nil {
		return gl.Program{}, err
	}
	gl.ValidateProgram(p)
	if gl.GetProgrami(p, gl.VALIDATE_STATUS) != gl.TRUE {
		return gl.Program{}, errors.New("VALIDATE_STATUS: " + gl.GetProgramInfoLog(p))
	}
	gl.UseProgram(p)
	if code := gl.GetError(); code != 0 {
		return gl.Program{}, fmt.Errorf("GL error %d", code)
	}
	return p, nil
}
