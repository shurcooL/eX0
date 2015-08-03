package main

import (
	"fmt"
	"time"
)

func ExampleFullConnection() {
	defer func() {
		err := recover()
		if err != nil {
			fmt.Println("panic:", err)
		}
	}()

	components.server = startServer() // Wait for server to start listening.
	components.client = startClient()
	time.Sleep(10 * time.Second) // Wait 10 seconds before exiting.

	// Output:
	// Started server.
	// (packet.JoinServerRequest)(packet.JoinServerRequest{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(26),
	// 		Type:   (packet.Type)(1),
	// 	}),
	// 	Version: (uint16)(1),
	// 	Passphrase: ([16]uint8)([16]uint8{
	// 		(uint8)(115),
	// 		(uint8)(111),
	// 		(uint8)(109),
	// 		(uint8)(101),
	// 		(uint8)(114),
	// 		(uint8)(97),
	// 		(uint8)(110),
	// 		(uint8)(100),
	// 		(uint8)(111),
	// 		(uint8)(109),
	// 		(uint8)(112),
	// 		(uint8)(97),
	// 		(uint8)(115),
	// 		(uint8)(115),
	// 		(uint8)(48),
	// 		(uint8)(49),
	// 	}),
	// 	Signature: (uint64)(123),
	// })
	// (packet.JoinServerAccept)(packet.JoinServerAccept{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(2),
	// 		Type:   (packet.Type)(2),
	// 	}),
	// 	YourPlayerId:     (uint8)(0),
	// 	TotalPlayerCount: (uint8)(15),
	// })
	// (packet.Handshake)(packet.Handshake{
	// 	UdpHeader: (packet.UdpHeader)(packet.UdpHeader{
	// 		Type: (packet.Type)(0),
	// 	}),
	// 	Signature: (uint64)(123),
	// })
	// (packet.UdpConnectionEstablished)(packet.UdpConnectionEstablished{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(0),
	// 		Type:   (packet.Type)(5),
	// 	}),
	// })
	// (packet.LocalPlayerInfo)(packet.LocalPlayerInfo{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(17),
	// 		Type:   (packet.Type)(30),
	// 	}),
	// 	NameLength: (uint8)(14),
	// 	Name: ([]uint8)([]uint8{
	// 		(uint8)(85),
	// 		(uint8)(110),
	// 		(uint8)(110),
	// 		(uint8)(97),
	// 		(uint8)(109),
	// 		(uint8)(101),
	// 		(uint8)(100),
	// 		(uint8)(32),
	// 		(uint8)(80),
	// 		(uint8)(108),
	// 		(uint8)(97),
	// 		(uint8)(121),
	// 		(uint8)(101),
	// 		(uint8)(114),
	// 	}),
	// 	CommandRate: (uint8)(20),
	// 	UpdateRate:  (uint8)(20),
	// })
	// (packet.LoadLevel)(packet.LoadLevel{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(5),
	// 		Type:   (packet.Type)(20),
	// 	}),
	// 	LevelFilename: ([]uint8)([]uint8{
	// 		(uint8)(116),
	// 		(uint8)(101),
	// 		(uint8)(115),
	// 		(uint8)(116),
	// 		(uint8)(51),
	// 	}),
	// })
	// (string)("test3")
	// (packet.CurrentPlayersInfo)(packet.CurrentPlayersInfo{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(31),
	// 		Type:   (packet.Type)(21),
	// 	}),
	// 	Players: ([]packet.PlayerInfo)([]packet.PlayerInfo{
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(14),
	// 			Name: ([]uint8)([]uint8{
	// 				(uint8)(85),
	// 				(uint8)(110),
	// 				(uint8)(110),
	// 				(uint8)(97),
	// 				(uint8)(109),
	// 				(uint8)(101),
	// 				(uint8)(100),
	// 				(uint8)(32),
	// 				(uint8)(80),
	// 				(uint8)(108),
	// 				(uint8)(97),
	// 				(uint8)(121),
	// 				(uint8)(101),
	// 				(uint8)(114),
	// 			}),
	// 			Team:  (packet.Team)(2),
	// 			State: (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 	}),
	// })
	// (packet.EnterGamePermission)(packet.EnterGamePermission{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(0),
	// 		Type:   (packet.Type)(6),
	// 	}),
	// })
	// (packet.EnteredGameNotification)(packet.EnteredGameNotification{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(0),
	// 		Type:   (packet.Type)(7),
	// 	}),
	// })
	// Client connected and joining team.
	// (packet.JoinTeamRequest)(packet.JoinTeamRequest{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(1),
	// 		Type:   (packet.Type)(27),
	// 	}),
	// 	PlayerNumber: (*uint8)(nil),
	// 	Team:         (packet.Team)(0),
	// })
	// (packet.PlayerJoinedTeam)(packet.PlayerJoinedTeam{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(15),
	// 		Type:   (packet.Type)(28),
	// 	}),
	// 	PlayerId: (uint8)(0),
	// 	Team:     (packet.Team)(0),
	// 	State: (*packet.State)(&packet.State{
	// 		CommandSequenceNumber: (uint8)(123),
	// 		X: (float32)(1),
	// 		Y: (float32)(2),
	// 		Z: (float32)(3),
	// 	}),
	// })
	// Unnamed Player joined team Red.
}

// Requires an empty server to be running.
func disabledExampleConnectToEmptyRealServer() {
	defer func() {
		err := recover()
		if err != nil {
			fmt.Println("panic:", err)
		}
	}()

	components.client = startClient()
	time.Sleep(10 * time.Second) // Wait 10 seconds before exiting.

	// Output:
	// (packet.JoinServerAccept)(packet.JoinServerAccept{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(2),
	// 		Type:   (packet.Type)(2),
	// 	}),
	// 	YourPlayerId:     (uint8)(0),
	// 	TotalPlayerCount: (uint8)(15),
	// })
	// (packet.UdpConnectionEstablished)(packet.UdpConnectionEstablished{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(0),
	// 		Type:   (packet.Type)(5),
	// 	}),
	// })
	// (packet.LoadLevel)(packet.LoadLevel{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(5),
	// 		Type:   (packet.Type)(20),
	// 	}),
	// 	LevelFilename: ([]uint8)([]uint8{
	// 		(uint8)(116),
	// 		(uint8)(101),
	// 		(uint8)(115),
	// 		(uint8)(116),
	// 		(uint8)(51),
	// 	}),
	// })
	// (string)("test3")
	// (packet.CurrentPlayersInfo)(packet.CurrentPlayersInfo{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(31),
	// 		Type:   (packet.Type)(21),
	// 	}),
	// 	Players: ([]packet.PlayerInfo)([]packet.PlayerInfo{
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(14),
	// 			Name: ([]uint8)([]uint8{
	// 				(uint8)(85),
	// 				(uint8)(110),
	// 				(uint8)(110),
	// 				(uint8)(97),
	// 				(uint8)(109),
	// 				(uint8)(101),
	// 				(uint8)(100),
	// 				(uint8)(32),
	// 				(uint8)(80),
	// 				(uint8)(108),
	// 				(uint8)(97),
	// 				(uint8)(121),
	// 				(uint8)(101),
	// 				(uint8)(114),
	// 			}),
	// 			Team:  (packet.Team)(2),
	// 			State: (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)(nil),
	// 			Team:       (packet.Team)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 	}),
	// })
	// (packet.EnterGamePermission)(packet.EnterGamePermission{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(0),
	// 		Type:   (packet.Type)(6),
	// 	}),
	// })
	// (packet.PlayerJoinedTeam)(packet.PlayerJoinedTeam{
	// 	TcpHeader: (packet.TcpHeader)(packet.TcpHeader{
	// 		Length: (uint16)(15),
	// 		Type:   (packet.Type)(28),
	// 	}),
	// 	PlayerId: (uint8)(0),
	// 	Team:     (packet.Team)(0),
	// 	State: (*packet.State)(&packet.State{
	// 		CommandSequenceNumber: (uint8)(123),
	// 		X: (float32)(1),
	// 		Y: (float32)(2),
	// 		Z: (float32)(3),
	// 	}),
	// })
	// Client connected and joined team.
}
