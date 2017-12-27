package main

import (
	"fmt"

	"github.com/go-gl/mathgl/mgl32"
)

type particles struct{}

func (p *particles) add(pos, vel mgl32.Vec2) {
	fmt.Println("added particle at:", pos)
}
