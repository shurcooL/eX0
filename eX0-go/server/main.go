// eX0 server test.
package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"math"
	"net"
	"sync"
	"time"

	"github.com/shurcooL/eX0/eX0-go/packet"
	"github.com/shurcooL/go-goon"
)

const Tau = 2 * math.Pi

func gameLogic() {
	for {
		for time.Since(startedProcess).Seconds() >= state.session.NextTickTime {
			state.session.NextTickTime += 1.0 / 20
			state.session.GlobalStateSequenceNumberTEST++
		}

		time.Sleep(time.Millisecond)
	}
}

func main() {
	state.session.GlobalStateSequenceNumberTEST = 0
	state.session.NextTickTime = time.Since(startedProcess).Seconds()
	go gameLogic()

	ln, err := net.Listen("tcp", ":25045")
	if err != nil {
		panic(err)
	}

	udpAddr, err := net.ResolveUDPAddr("udp", ":25045")
	if err != nil {
		panic(err)
	}
	ln2, err := net.ListenUDP("udp", udpAddr)
	if err != nil {
		panic(err)
	}
	go handleUdp(ln2)
	go sendServerUpdates(ln2)
	go broadcastPingPacket(ln2)

	fmt.Println("Started.")

	for {
		tcp, err := ln.Accept()
		if err != nil {
			panic(err)
		}
		go handleConnection(tcp)
	}
}

var startedProcess = time.Now()

var state = struct {
	TotalPlayerCount uint8

	session struct {
		GlobalStateSequenceNumberTEST uint8
		NextTickTime                  float64
	}

	mu          sync.Mutex
	connections map[net.Conn]Connection // TcpConn -> Connection.
}{
	TotalPlayerCount: 16,
	connections:      make(map[net.Conn]Connection),
}

type Connection struct {
	JoinStatus JoinStatus
	Signature  uint64
	UdpAddr    *net.UDPAddr
}

func handleUdp(udp *net.UDPConn) {
	for {
		var b [packet.MAX_UDP_SIZE]byte
		n, udpAddr, err := udp.ReadFromUDP(b[:])
		if err != nil {
			panic(err)
		}
		_ = udpAddr
		var buf = bytes.NewReader(b[:n])

		var udpHeader packet.UdpHeader
		err = binary.Read(buf, binary.BigEndian, &udpHeader)
		if err != nil {
			panic(err)
		}

		switch udpHeader.Type {
		case packet.HandshakeType:
			var r packet.Handshake
			err = binary.Read(buf, binary.BigEndian, &r.Signature)
			if err != nil {
				panic(err)
			}
			goon.Dump(r)

			var tcp net.Conn
			state.mu.Lock()
			for tcpConn, c := range state.connections {
				if c.Signature == r.Signature {
					c.JoinStatus = UDP_CONNECTED
					c.UdpAddr = udpAddr
					state.connections[tcpConn] = c

					tcp = tcpConn
				}
			}
			state.mu.Unlock()

			if tcp != nil {
				var p packet.UdpConnectionEstablished
				p.Type = packet.UdpConnectionEstablishedType

				p.Length = 0

				err := binary.Write(tcp, binary.BigEndian, &p)
				if err != nil {
					panic(err)
				}
			}
		case packet.TimeRequestType:
			var r packet.TimeRequest
			err = binary.Read(buf, binary.BigEndian, &r.SequenceNumber)
			if err != nil {
				panic(err)
			}

			{
				var p packet.TimeResponse
				p.Type = packet.TimeResponseType
				p.SequenceNumber = r.SequenceNumber
				p.Time = time.Since(startedProcess).Seconds()

				var buf bytes.Buffer
				err := binary.Write(&buf, binary.BigEndian, &p)
				if err != nil {
					panic(err)
				}
				_, err = udp.WriteToUDP(buf.Bytes(), udpAddr)
				if err != nil {
					panic(err)
				}
			}
		case packet.PongType:
			var r packet.Pong
			err = binary.Read(buf, binary.BigEndian, &r.PingData)
			if err != nil {
				panic(err)
			}

			{
				var p packet.Pung
				p.Type = packet.PungType
				p.PingData = r.PingData
				p.Time = time.Since(startedProcess).Seconds()

				var buf bytes.Buffer
				err := binary.Write(&buf, binary.BigEndian, &p)
				if err != nil {
					panic(err)
				}
				_, err = udp.WriteToUDP(buf.Bytes(), udpAddr)
				if err != nil {
					panic(err)
				}
			}
		case packet.ClientCommandType:
			var r packet.ClientCommand
			err = binary.Read(buf, binary.BigEndian, &r.CommandSequenceNumber)
			if err != nil {
				panic(err)
			}
			err = binary.Read(buf, binary.BigEndian, &r.CommandSeriesNumber)
			if err != nil {
				panic(err)
			}
			err = binary.Read(buf, binary.BigEndian, &r.MovesCount)
			if err != nil {
				panic(err)
			}
			r.Moves = make([]packet.Move, r.MovesCount)
			err = binary.Read(buf, binary.BigEndian, &r.Moves)
			if err != nil {
				panic(err)
			}
			//goon.Dump(r)

			// TODO: Properly process and authenticate new result states.
			if len(r.Moves) > 0 {
				lastMove := r.Moves[len(r.Moves)-1]

				// TODO: Figure out player id, not hardcode 0.
				if lastMove.MoveDirection != 255 {
					direction := float64(lastMove.Z) + Tau*float64(lastMove.MoveDirection)/8

					player0State.X += 10 * float32(math.Sin(direction))
					player0State.Y += 10 * float32(math.Cos(direction))
				}
				player0State.Z = lastMove.Z

				lastAckedCmdSequenceNumber = r.CommandSequenceNumber
			}
		}
	}
}

var player0State = packet.State{
	X: 25,
	Y: -220,
	Z: 6.0,
}

var lastAckedCmdSequenceNumber uint8
var lastUpdateSequenceNumber uint8 = 1

func sendServerUpdates(udp *net.UDPConn) {
	for ; true; time.Sleep(time.Second / 20) {
		{
			var p packet.ServerUpdate
			p.Type = packet.ServerUpdateType
			p.CurrentUpdateSequenceNumber = lastUpdateSequenceNumber
			p.Players = make([]packet.PlayerUpdate, state.TotalPlayerCount)
			p.Players[0] = packet.PlayerUpdate{
				ActivePlayer: 1,
				State: &packet.State{
					CommandSequenceNumber: lastAckedCmdSequenceNumber, // HACK.
					X: player0State.X,
					Y: player0State.Y,
					Z: player0State.Z,
				},
			}

			var buf bytes.Buffer
			err := binary.Write(&buf, binary.BigEndian, &p.UdpHeader)
			if err != nil {
				panic(err)
			}
			err = binary.Write(&buf, binary.BigEndian, &p.CurrentUpdateSequenceNumber)
			if err != nil {
				panic(err)
			}
			for _, playerUpdate := range p.Players {
				err = binary.Write(&buf, binary.BigEndian, &playerUpdate.ActivePlayer)
				if err != nil {
					panic(err)
				}

				if playerUpdate.ActivePlayer != 0 {
					err = binary.Write(&buf, binary.BigEndian, playerUpdate.State)
					if err != nil {
						panic(err)
					}
				}
			}

			var udpAddr *net.UDPAddr
			state.mu.Lock()
			for _, c := range state.connections {
				if c.JoinStatus < IN_GAME {
					continue
				}
				udpAddr = c.UdpAddr
			}
			state.mu.Unlock()

			if udpAddr == nil {
				continue
			}

			_, err = udp.WriteToUDP(buf.Bytes(), udpAddr)
			if err != nil {
				panic(err)
			}
		}

		lastUpdateSequenceNumber++
	}
}

var lastPingData uint32
var pingSentTimes = make(map[uint32]time.Time) // TODO: Use.

func broadcastPingPacket(udp *net.UDPConn) {
	const BROADCAST_PING_PERIOD = 2500 * time.Millisecond // How often to broadcast the Ping packet on the server.

	for ; true; time.Sleep(BROADCAST_PING_PERIOD) {
		{
			var p packet.Ping
			p.Type = packet.PingType
			p.PingData = lastPingData
			p.LastLatencies = make([]uint16, state.TotalPlayerCount)

			var buf bytes.Buffer
			err := binary.Write(&buf, binary.BigEndian, &p.UdpHeader)
			if err != nil {
				panic(err)
			}
			err = binary.Write(&buf, binary.BigEndian, &p.PingData)
			if err != nil {
				panic(err)
			}
			err = binary.Write(&buf, binary.BigEndian, &p.LastLatencies)
			if err != nil {
				panic(err)
			}

			var udpAddr *net.UDPAddr
			state.mu.Lock()
			for _, c := range state.connections {
				if c.JoinStatus < IN_GAME {
					continue
				}
				udpAddr = c.UdpAddr
			}
			state.mu.Unlock()

			if udpAddr == nil {
				continue
			}

			_, err = udp.WriteToUDP(buf.Bytes(), udpAddr)
			if err != nil {
				panic(err)
			}
		}

		lastPingData++
	}
}

func handleConnection(tcp net.Conn) {
	err := handleConnection2(tcp)
	fmt.Println("tcp conn ended with:", err)

	state.mu.Lock()
	delete(state.connections, tcp)
	state.mu.Unlock()

	tcp.Close()
}

func handleConnection2(tcp net.Conn) error {
	{
		var r packet.JoinServerRequest
		err := binary.Read(tcp, binary.BigEndian, &r)
		if err != nil {
			return err
		}
		goon.Dump(r)

		if r.Version != 1 ||
			r.Passphrase != [16]byte{'s', 'o', 'm', 'e', 'r', 'a', 'n', 'd', 'o', 'm', 'p', 'a', 's', 's', '0', '1'} {

			{
				var p packet.JoinServerRefuse
				p.Type = packet.JoinServerRefuseType
				p.RefuseReason = 123 // TODO.

				p.Length = 1

				err := binary.Write(tcp, binary.BigEndian, &p)
				if err != nil {
					return err
				}

				return nil
			}
		}

		state.mu.Lock()
		state.connections[tcp] = Connection{Signature: r.Signature}
		state.mu.Unlock()
	}

	{
		var p packet.JoinServerAccept
		p.Type = packet.JoinServerAcceptType
		p.YourPlayerId = 0
		p.TotalPlayerCount = state.TotalPlayerCount - 1

		p.Length = 2

		err := binary.Write(tcp, binary.BigEndian, &p)
		if err != nil {
			return err
		}
	}

	{
		var r packet.LocalPlayerInfo
		err := binary.Read(tcp, binary.BigEndian, &r.TcpHeader)
		if err != nil {
			return err
		}
		err = binary.Read(tcp, binary.BigEndian, &r.NameLength)
		if err != nil {
			return err
		}
		r.Name = make([]byte, r.NameLength)
		err = binary.Read(tcp, binary.BigEndian, &r.Name)
		if err != nil {
			return err
		}
		err = binary.Read(tcp, binary.BigEndian, &r.CommandRate)
		if err != nil {
			return err
		}
		err = binary.Read(tcp, binary.BigEndian, &r.UpdateRate)
		if err != nil {
			return err
		}
		goon.Dump(r)
	}

	{
		var p packet.LoadLevel
		p.Type = packet.LoadLevelType
		p.LevelFilename = []byte("test3")

		p.Length = uint16(len(p.LevelFilename))

		err := binary.Write(tcp, binary.BigEndian, &p.TcpHeader)
		if err != nil {
			return err
		}
		err = binary.Write(tcp, binary.BigEndian, &p.LevelFilename)
		if err != nil {
			return err
		}
	}

	{
		var p packet.CurrentPlayersInfo
		p.Type = packet.CurrentPlayersInfoType
		p.Players = make([]packet.PlayerInfo, state.TotalPlayerCount)

		p.Length = 16 + uint16(len("shurcooL")) + 1

		err := binary.Write(tcp, binary.BigEndian, &p.TcpHeader)
		if err != nil {
			return err
		}
		for i := range p.Players {
			var playerInfo packet.PlayerInfo

			// TODO: Use actual players state.
			if i == 0 {
				playerInfo.Name = []byte("shurcooL")
				playerInfo.NameLength = uint8(len(playerInfo.Name))
				playerInfo.Team = 2
			}

			err = binary.Write(tcp, binary.BigEndian, &playerInfo.NameLength)
			if err != nil {
				return err
			}

			if playerInfo.NameLength != 0 {
				err = binary.Write(tcp, binary.BigEndian, &playerInfo.Name)
				if err != nil {
					return err
				}
				err = binary.Write(tcp, binary.BigEndian, &playerInfo.Team)
				if err != nil {
					return err
				}

				if playerInfo.Team != 2 {
					err = binary.Write(tcp, binary.BigEndian, playerInfo.State)
					if err != nil {
						return err
					}
				}
			}
		}
	}

	{
		var p packet.EnterGamePermission
		p.Type = packet.EnterGamePermissionType

		p.Length = 0

		err := binary.Write(tcp, binary.BigEndian, &p)
		if err != nil {
			return err
		}
	}

	{
		var r packet.EnteredGameNotification
		err := binary.Read(tcp, binary.BigEndian, &r)
		if err != nil {
			return err
		}
		goon.Dump(r)

		state.mu.Lock()
		c := state.connections[tcp]
		c.JoinStatus = IN_GAME
		state.connections[tcp] = c
		state.mu.Unlock()
	}

	for {
		var playerId uint8 = 0 // TODO: Non-hardcoded value.
		var team uint8

		{
			var r packet.JoinTeamRequest
			err := binary.Read(tcp, binary.BigEndian, &r.TcpHeader)
			if err != nil {
				return err
			}
			// TODO: Handle potential PlayerNumber.
			err = binary.Read(tcp, binary.BigEndian, &r.Team)
			if err != nil {
				return err
			}
			goon.Dump(r)

			team = r.Team
		}

		logicTime := float64(state.session.GlobalStateSequenceNumberTEST) + (time.Since(startedProcess).Seconds()-state.session.NextTickTime)*20
		fmt.Printf("%.3f: Pl#%v (%q) joined team %v at logic time %.2f/%v [server].\n", time.Since(startedProcess).Seconds(), playerId, "TODO: name", team, logicTime, state.session.GlobalStateSequenceNumberTEST)

		{
			var p packet.PlayerJoinedTeam
			p.Type = packet.PlayerJoinedTeamType
			p.PlayerId = playerId
			p.Team = team

			if p.Team != 2 {
				p.State = &packet.State{
					CommandSequenceNumber: state.session.GlobalStateSequenceNumberTEST - 1,
					X: player0State.X,
					Y: player0State.Y,
					Z: player0State.Z,
				}
			}

			p.Length = 2
			if p.State != nil {
				p.Length += 13
			}

			err := binary.Write(tcp, binary.BigEndian, &p.TcpHeader)
			if err != nil {
				return err
			}
			err = binary.Write(tcp, binary.BigEndian, &p.PlayerId)
			if err != nil {
				return err
			}
			err = binary.Write(tcp, binary.BigEndian, &p.Team)
			if err != nil {
				return err
			}
			if p.State != nil {
				err = binary.Write(tcp, binary.BigEndian, p.State)
				if err != nil {
					return err
				}
			}
		}
	}
}
