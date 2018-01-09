package main

import (
	"fmt"
	"sync"

	"github.com/go-gl/mathgl/mgl32"
)

type particleSystem struct {
	sync.Mutex
	particles []particle
}

func (ps *particleSystem) Add(position, velocity mgl32.Vec2, bornAt, maxAge float64) {
	ps.Lock()
	defer ps.Unlock()

	ps.particles = append(ps.particles, particle{
		Position: position,
		Velocity: velocity,
		BornAt:   bornAt,
		Time:     bornAt,
		DieAt:    bornAt + maxAge,
	})
}

func (ps *particleSystem) Tick(now float64) {
	ps.Lock()
	defer ps.Unlock()

	// Movement.
	for i := range ps.particles {
		p := &ps.particles[i]
		for p.Time <= now {
			p.Time += 1.0 / commandRate
			if p.Time >= p.DieAt {
				fmt.Printf("BOOM at x=%f, y=%f and LogicTime=%.20f\n", p.Position.X(), p.Position.Y(), p.DieAt)
				break
			}

			p.Position = p.Position.Add(p.Velocity)
			//fmt.Printf("%f: bouncy bullet vel: %f (%t)\n", p.Time, p.Velocity.Len(), p.Age(p.Time) < 1.75)
			if p.Age(p.Time) < 1.75 {
				// Slow down minimally during its first 1.75 seconds of life.
				p.Velocity = p.Velocity.Mul(0.990)
			} else {
				// Slow down significantly during its last 1.25 seconds of life.
				x := p.Velocity.Len() - 0.2
				if x < 0.01 {
					x = 0.01
				}
				p.Velocity = p.Velocity.Normalize().Mul(x)
			}
		}
	}

	// Remove dead particles.
	var alive []particle
	for _, p := range ps.particles {
		if p.Time >= p.DieAt {
			// Dead particle, skip it.
			continue
		}
		alive = append(alive, p)
	}
	ps.particles = alive
}

type particle struct {
	Position mgl32.Vec2
	Velocity mgl32.Vec2
	BornAt   float64
	Time     float64
	DieAt    float64
}

func (p particle) Age(now float64) float64 {
	return now - p.BornAt
}
