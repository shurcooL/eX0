package main

import "fmt"

func ExampleConnectToEmptyRealServer() {
	defer func() {
		err := recover()
		if err != nil {
			fmt.Println("panic:", err)
		}
	}()

	main()

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
	// 		Length: (uint16)(25),
	// 		Type:   (packet.Type)(21),
	// 	}),
	// 	Players: ([]packet.PlayerInfo)([]packet.PlayerInfo{
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(8),
	// 			Name: ([]uint8)([]uint8{
	// 				(uint8)(115),
	// 				(uint8)(104),
	// 				(uint8)(117),
	// 				(uint8)(114),
	// 				(uint8)(99),
	// 				(uint8)(111),
	// 				(uint8)(111),
	// 				(uint8)(76),
	// 			}),
	// 			Team:  (uint8)(2),
	// 			State: (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
	// 			State:      (*packet.State)(nil),
	// 		}),
	// 		(packet.PlayerInfo)(packet.PlayerInfo{
	// 			NameLength: (uint8)(0),
	// 			Name:       ([]uint8)([]uint8{}),
	// 			Team:       (uint8)(0),
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
	// 	Team:     (uint8)(0),
	// 	State: (*packet.State)(&packet.State{
	// 		CommandSequenceNumber: (uint8)(123),
	// 		X: (float32)(1),
	// 		Y: (float32)(2),
	// 		Z: (float32)(3),
	// 	}),
	// })
	// done
}
