package main

import (
	"fmt"
	"sync"

	"github.com/go-gl/mathgl/mgl32"
)

type particles struct {
	mu        sync.Mutex
	particles []particle
}

func (p *particles) Add(pos, vel mgl32.Vec2) {
	p.mu.Lock()
	defer p.mu.Unlock()

	fmt.Println("added particle at:", pos)
	p.particles = append(p.particles, particle{
		pos: pos,
	})
}

type particle struct {
	pos mgl32.Vec2
}
