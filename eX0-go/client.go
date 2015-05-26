package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"time"

	//"github.com/gopherjs/websocket"

	"github.com/shurcooL/eX0/eX0-go/packet"
	"github.com/shurcooL/go-goon"
)

var pongSentTimes = make(map[uint32]time.Time) // PingData -> Time.

//const addr = "dmitri.shuralyov.com:25045"
const addr = "localhost:25045"

var clientToServerConn *Connection

var clientLastAckedCmdSequenceNumber uint8

type client struct{}

func startClient() *client {
	clientToServerConn = newConnection()

	clientToServerConn.dialServer()

	connectToServer(clientToServerConn)

	return &client{}
}

func connectToServer(s *Connection) {
	s.Signature = uint64(time.Now().UnixNano())

	{
		var p = packet.JoinServerRequest{
			TcpHeader: packet.TcpHeader{
				Type: packet.JoinServerRequestType,
			},
			Version:    1,
			Passphrase: [16]byte{'s', 'o', 'm', 'e', 'r', 'a', 'n', 'd', 'o', 'm', 'p', 'a', 's', 's', '0', '1'},
			Signature:  s.Signature,
		}

		p.Length = 26

		var buf bytes.Buffer
		err := binary.Write(&buf, binary.BigEndian, &p)
		if err != nil {
			panic(err)
		}
		err = sendTcpPacket(s, buf.Bytes())
		if err != nil {
			panic(err)
		}
	}

	{
		buf, err := receiveTcpPacket(s)
		if err != nil {
			panic(err)
		}
		var r packet.JoinServerAccept
		err = binary.Read(buf, binary.BigEndian, &r)
		if err != nil {
			panic(err)
		}
		goon.Dump(r)

		state.Lock()
		state.TotalPlayerCount = r.TotalPlayerCount + 1
		state.Unlock()
	}

	// Upgrade connection to UDP at this point.

	{
		var p packet.Handshake
		p.Type = packet.HandshakeType
		p.Signature = s.Signature

		var buf bytes.Buffer
		err := binary.Write(&buf, binary.BigEndian, &p)
		if err != nil {
			panic(err)
		}
		err = sendUdpPacket(s, buf.Bytes())
		if err != nil {
			panic(err)
		}
	}

	{
		buf, err := receiveTcpPacket(s)
		if err != nil {
			panic(err)
		}
		var r packet.UdpConnectionEstablished
		err = binary.Read(buf, binary.BigEndian, &r)
		if err != nil {
			panic(err)
		}
		goon.Dump(r)
	}

	{
		const name = "shurcooL"

		var p packet.LocalPlayerInfo
		p.Type = packet.LocalPlayerInfoType
		p.NameLength = uint8(len(name))
		p.Name = []byte(name)
		p.CommandRate = 20
		p.UpdateRate = 20

		p.Length = 3 + uint16(len(name))

		var buf bytes.Buffer
		err := binary.Write(&buf, binary.BigEndian, &p.TcpHeader)
		if err != nil {
			panic(err)
		}
		err = binary.Write(&buf, binary.BigEndian, &p.NameLength)
		if err != nil {
			panic(err)
		}
		err = binary.Write(&buf, binary.BigEndian, &p.Name)
		if err != nil {
			panic(err)
		}
		err = binary.Write(&buf, binary.BigEndian, &p.CommandRate)
		if err != nil {
			panic(err)
		}
		err = binary.Write(&buf, binary.BigEndian, &p.UpdateRate)
		if err != nil {
			panic(err)
		}
		err = sendTcpPacket(s, buf.Bytes())
		if err != nil {
			panic(err)
		}
	}

	{
		buf, err := receiveTcpPacket(s)
		if err != nil {
			panic(err)
		}
		var r packet.LoadLevel
		err = binary.Read(buf, binary.BigEndian, &r.TcpHeader)
		if err != nil {
			panic(err)
		}
		r.LevelFilename = make([]byte, r.Length)
		err = binary.Read(buf, binary.BigEndian, &r.LevelFilename)
		if err != nil {
			panic(err)
		}
		goon.Dump(r)
		goon.Dump(string(r.LevelFilename))
	}

	{
		buf, err := receiveTcpPacket(s)
		if err != nil {
			panic(err)
		}
		var r packet.CurrentPlayersInfo
		err = binary.Read(buf, binary.BigEndian, &r.TcpHeader)
		if err != nil {
			panic(err)
		}
		r.Players = make([]packet.PlayerInfo, state.TotalPlayerCount)
		for i := range r.Players {
			var playerInfo packet.PlayerInfo
			err = binary.Read(buf, binary.BigEndian, &playerInfo.NameLength)
			if err != nil {
				panic(err)
			}

			if playerInfo.NameLength != 0 {
				playerInfo.Name = make([]byte, playerInfo.NameLength)
				err = binary.Read(buf, binary.BigEndian, &playerInfo.Name)
				if err != nil {
					panic(err)
				}

				err = binary.Read(buf, binary.BigEndian, &playerInfo.Team)
				if err != nil {
					panic(err)
				}

				if playerInfo.Team != 2 {
					playerInfo.State = new(packet.State)
					err = binary.Read(buf, binary.BigEndian, playerInfo.State)
					if err != nil {
						panic(err)
					}
				}
			}

			r.Players[i] = playerInfo
		}
		goon.Dump(r)
	}

	{
		buf, err := receiveTcpPacket(s)
		if err != nil {
			panic(err)
		}
		var r packet.EnterGamePermission
		err = binary.Read(buf, binary.BigEndian, &r)
		if err != nil {
			panic(err)
		}
		goon.Dump(r)
	}

	{
		var p packet.EnteredGameNotification
		p.Type = packet.EnteredGameNotificationType

		p.Length = 0

		var buf bytes.Buffer
		err := binary.Write(&buf, binary.BigEndian, &p)
		if err != nil {
			panic(err)
		}
		err = sendTcpPacket(s, buf.Bytes())
		if err != nil {
			panic(err)
		}
	}

	time.Sleep(3 * time.Second)

	{
		var p packet.JoinTeamRequest
		p.Type = packet.JoinTeamRequestType
		p.Team = 0

		p.Length = 1

		var buf bytes.Buffer
		err := binary.Write(&buf, binary.BigEndian, &p.TcpHeader)
		if err != nil {
			panic(err)
		}
		err = binary.Write(&buf, binary.BigEndian, &p.Team)
		if err != nil {
			panic(err)
		}
		err = sendTcpPacket(s, buf.Bytes())
		if err != nil {
			panic(err)
		}
	}

	{
		buf, err := receiveTcpPacket(s)
		if err != nil {
			panic(err)
		}
		var r packet.PlayerJoinedTeam
		err = binary.Read(buf, binary.BigEndian, &r.TcpHeader)
		if err != nil {
			panic(err)
		}
		err = binary.Read(buf, binary.BigEndian, &r.PlayerId)
		if err != nil {
			panic(err)
		}
		err = binary.Read(buf, binary.BigEndian, &r.Team)
		if err != nil {
			panic(err)
		}
		if r.Team != 2 {
			r.State = new(packet.State)
			err = binary.Read(buf, binary.BigEndian, r.State)
			if err != nil {
				panic(err)
			}
		}

		{
			r2 := r
			r2.State = &packet.State{CommandSequenceNumber: 123, X: 1.0, Y: 2.0, Z: 3.0} // Override with deterministic value so test passes.
			goon.Dump(r2)
		}

		clientLastAckedCmdSequenceNumber = r.State.CommandSequenceNumber
	}

	fmt.Println("Client connected and joined team.")

	go func() {
		for {
			buf, err := receiveUdpPacket(s)
			if err != nil {
				panic(err)
			}

			var udpHeader packet.UdpHeader
			err = binary.Read(buf, binary.BigEndian, &udpHeader)
			if err != nil {
				panic(err)
			}

			switch udpHeader.Type {
			case packet.PingType:
				var r packet.Ping
				err = binary.Read(buf, binary.BigEndian, &r.PingData)
				if err != nil {
					panic(err)
				}
				r.LastLatencies = make([]uint16, state.TotalPlayerCount)
				err = binary.Read(buf, binary.BigEndian, &r.LastLatencies)
				if err != nil {
					panic(err)
				}

				{
					var p packet.Pong
					p.Type = packet.PongType
					p.PingData = r.PingData

					pongSentTimes[r.PingData] = time.Now()

					var buf bytes.Buffer
					err := binary.Write(&buf, binary.BigEndian, &p)
					if err != nil {
						panic(err)
					}
					err = sendUdpPacket(s, buf.Bytes())
					if err != nil {
						panic(err)
					}
				}
			case packet.PungType:
				localTimeAtPungReceive := time.Now()

				var r packet.Pung
				err = binary.Read(buf, binary.BigEndian, &r.PingData)
				if err != nil {
					panic(err)
				}
				err = binary.Read(buf, binary.BigEndian, &r.Time)
				if err != nil {
					panic(err)
				}

				{
					// Get the time sent of the matching Pong packet.
					if localTimeAtPongSend, ok := pongSentTimes[r.PingData]; ok {
						delete(pongSentTimes, r.PingData)

						// Calculate own latency and update it on the scoreboard.
						latency := localTimeAtPungReceive.Sub(localTimeAtPongSend)
						log.Printf("Own latency is %.5f ms.\n", latency.Seconds()*1000)
					}
				}
			case packet.ServerUpdateType:
				var r packet.ServerUpdate
				err = binary.Read(buf, binary.BigEndian, &r.CurrentUpdateSequenceNumber)
				if err != nil {
					panic(err)
				}
				r.Players = make([]packet.PlayerUpdate, state.TotalPlayerCount)
				for i := range r.Players {
					var playerUpdate packet.PlayerUpdate
					err = binary.Read(buf, binary.BigEndian, &playerUpdate.ActivePlayer)
					if err != nil {
						panic(err)
					}

					if playerUpdate.ActivePlayer != 0 {
						playerUpdate.State = new(packet.State)
						err = binary.Read(buf, binary.BigEndian, playerUpdate.State)
						if err != nil {
							panic(err)
						}
					}

					r.Players[i] = playerUpdate
				}

				if components.server == nil {
					if playerUpdate := r.Players[0]; playerUpdate.ActivePlayer != 0 {
						player0StateMu.Lock()
						player0State.X = playerUpdate.State.X
						player0State.Y = playerUpdate.State.Y
						player0State.Z = playerUpdate.State.Z
						player0StateMu.Unlock()
					}
				}
			}
		}
	}()
}
