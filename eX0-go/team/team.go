package team

import "fmt"

// Stringer implements String method for team.
type Stringer uint8

func (t Stringer) String() string {
	switch t {
	case 0:
		return "team Red"
	case 1:
		return "team Blue"
	case 2:
		return "spectators"
	default:
		panic(fmt.Errorf("unsupported team %v", t))
	}
}
