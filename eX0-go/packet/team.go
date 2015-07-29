package packet

import "fmt"

type Team uint8

const (
	Red       Team = 0
	Blue      Team = 1
	Spectator Team = 2
)

func (t Team) String() string {
	switch t {
	case Red:
		return "team Red"
	case Blue:
		return "team Blue"
	case Spectator:
		return "spectators"
	default:
		panic(fmt.Errorf("unsupported team %v", t))
	}
}
