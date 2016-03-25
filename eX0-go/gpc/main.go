// Package gpc parses GPC format files.
package gpc

import (
	"fmt"
	"io"
	"os"

	"github.com/go-gl/mathgl/mgl64"
)

// Polygon is a polygon.
type Polygon struct {
	Contours []Contour
}

// Contour is a contour.
type Contour struct {
	Vertices []mgl64.Vec2
}

// Parse parses a GPC format reader.
func Parse(r io.Reader) (Polygon, error) {
	var p Polygon

	var nc uint64
	_, err := fmt.Fscan(r, &nc)
	if err != nil {
		return p, err
	}
	p.Contours = make([]Contour, nc)

	for ci := range p.Contours {
		var nv uint64
		_, err := fmt.Fscan(r, &nv)
		if err != nil {
			return p, err
		}
		p.Contours[ci].Vertices = make([]mgl64.Vec2, nv)

		for vi := range p.Contours[ci].Vertices {
			_, err := fmt.Fscan(r, &p.Contours[ci].Vertices[vi][0], &p.Contours[ci].Vertices[vi][1])
			if err != nil {
				return p, err
			}
		}
	}

	return p, nil
}

// ParseFile parses a GPC format file.
func ParseFile(path string) (Polygon, error) {
	f, err := os.Open(path)
	if err != nil {
		return Polygon{}, err
	}
	defer f.Close()

	return Parse(f)
}
