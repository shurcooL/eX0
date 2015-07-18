// +build !js

package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"math"
	"net"
	"net/http"
	"os"
	"time"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/shurcooL/eX0/eX0-go/packet"
	"github.com/shurcooL/go-goon"
	"golang.org/x/net/websocket"
)

func startServer() *server {
	{
		go listenAndHandleTcp()
	}

	{
		go listenAndHandleUdp()
	}

	{
		go listenAndHandleWebSocket()
	}

	{
		go listenAndHandleChan()
	}

	go sendServerUpdates()
	go broadcastPingPacket()

	time.Sleep(time.Millisecond) // HACK: Give some time for listeners to start.
	fmt.Println("Started server.")

	return &server{}
}

func listenAndHandleTcp() {
	ln, err := net.Listen("tcp", ":25045")
	if err != nil {
		panic(err)
	}

	for {
		tcp, err := ln.Accept()
		if err != nil {
			panic(err)
		}

		client := newConnection()
		client.tcp = tcp
		client.JoinStatus = TCP_CONNECTED
		client.dialedClient()
		state.Lock()
		state.connections = append(state.connections, client)
		state.Unlock()

		if shouldHandleUdpDirectly {
			go handleUdp(client)
		}
		go handleTcpConnection(client)
	}
}

func listenAndHandleWebSocket() {
	h := websocket.Handler(func(conn *websocket.Conn) {
		// Why is this exported field undocumented?
		//
		// It needs to be set to websocket.BinaryFrame so that
		// the Write method sends bytes as binary rather than text frames.
		conn.PayloadType = websocket.BinaryFrame

		client := newConnection()
		client.tcp = conn
		client.JoinStatus = TCP_CONNECTED
		client.dialedClient()
		state.Lock()
		state.connections = append(state.connections, client)
		state.Unlock()

		if shouldHandleUdpDirectly {
			go handleUdp(client)
		}
		handleTcpConnection(client)
		// Do not return until handleTcpConnection does, else WebSocket gets closed.
	})
	err := http.ListenAndServe(":25046", h)
	if err != nil {
		panic(err)
	}
}

var chanListener = make(chan *Connection)
var chanListenerReply = make(chan struct{})

func listenAndHandleChan() {
	for clientToServerConn := range chanListener {

		serverToClientConn := newConnection()
		// Join server <-> client channel ends together.
		serverToClientConn.sendTcp = clientToServerConn.recvTcp
		serverToClientConn.recvTcp = clientToServerConn.sendTcp
		serverToClientConn.sendUdp = clientToServerConn.recvUdp
		serverToClientConn.recvUdp = clientToServerConn.sendUdp
		serverToClientConn.JoinStatus = TCP_CONNECTED

		state.Lock()
		state.connections = append(state.connections, serverToClientConn)
		state.Unlock()

		if shouldHandleUdpDirectly {
			go handleUdp(serverToClientConn)
		}
		go handleTcpConnection(serverToClientConn)

		chanListenerReply <- struct{}{}
	}
}

func listenAndHandleUdp() {
	udpAddr, err := net.ResolveUDPAddr("udp", ":25045")
	if err != nil {
		panic(err)
	}
	ln, err := net.ListenUDP("udp", udpAddr)
	if err != nil {
		panic(err)
	}
	mux := &Connection{udp: ln}

	handleUdp(mux)
}

func handleUdp(mux *Connection) {
	for {
		buf, c, udpAddr, err := receiveUdpPacketFrom(mux)
		if err != nil {
			if shouldHandleUdpDirectly { // HACK: This isn't a real mux but rather the client directly, so return.
				fmt.Println("udp conn ended with:", err)
				return
			}
			panic(err)
		}

		var udpHeader packet.UdpHeader
		err = binary.Read(buf, binary.BigEndian, &udpHeader)
		if err != nil {
			panic(err)
		}

		if c == nil && udpHeader.Type != packet.HandshakeType {
			continue
		}

		switch udpHeader.Type {
		case packet.HandshakeType:
			var r packet.Handshake
			err = binary.Read(buf, binary.BigEndian, &r.Signature)
			if err != nil {
				panic(err)
			}
			{
				r2 := r
				r2.Signature = 123
				goon.Dump(r2)
			}

			state.Lock()
			for _, connection := range state.connections {
				if connection.Signature == r.Signature {
					connection.JoinStatus = UDP_CONNECTED
					connection.udp = mux.udp
					connection.UdpAddr = udpAddr

					c = connection
					break
				}
			}
			state.Unlock()

			if c != nil {
				var p packet.UdpConnectionEstablished
				p.Type = packet.UdpConnectionEstablishedType

				p.Length = 0

				var buf bytes.Buffer
				err := binary.Write(&buf, binary.BigEndian, &p)
				if err != nil {
					panic(err)
				}
				err = sendTcpPacket(c, buf.Bytes())
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
				state.Lock()
				p.Time = time.Since(startedProcess).Seconds()
				state.Unlock()

				var buf bytes.Buffer
				err := binary.Write(&buf, binary.BigEndian, &p)
				if err != nil {
					panic(err)
				}
				err = sendUdpPacket(c, buf.Bytes())
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
				err = sendUdpPacket(c, buf.Bytes())
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
			r.MovesCount += 1 // De-normalize back to 1 (min value).
			r.Moves = make([]packet.Move, r.MovesCount)
			err = binary.Read(buf, binary.BigEndian, &r.Moves)
			if err != nil {
				panic(err)
			}
			//goon.Dump(r)

			// TODO: Properly process and authenticate new result states.
			if len(r.Moves) > 0 {
				lastMove := r.Moves[len(r.Moves)-1]

				var playerId int
				playersStateMu.Lock()
				for id, ps := range playersState {
					if ps.conn == c {
						playerId = id
						break
					}
				}
				playersStateMu.Unlock()

				{
					const TOP_SPEED = 3.5

					var TargetVel mgl32.Vec2

					if lastMove.MoveDirection == 255 {
					} else if lastMove.MoveDirection >= 0 && lastMove.MoveDirection < 8 {
						direction := float64(lastMove.Z) + Tau*float64(lastMove.MoveDirection)/8
						speed := TOP_SPEED
						if lastMove.Stealth != 0 {
							speed -= 2.25
						}

						TargetVel[0] = float32(math.Sin(direction) * speed)
						TargetVel[1] = float32(math.Cos(direction) * speed)
					} else {
						log.Printf("WARNING: Invalid nMoveDirection = %v!\n", lastMove.MoveDirection)
					}

					var CurrentVel = mgl32.Vec2{playersState[playerId].VelX, playersState[playerId].VelY}
					var Delta = TargetVel.Sub(CurrentVel)
					if DeltaLength := float64(Delta.Len()); DeltaLength >= 0.001 {
						Delta = Delta.Normalize()

						var Move1 = DeltaLength * DeltaLength * 0.03
						var Move2 = math.Min(0.2, DeltaLength)

						CurrentVel = CurrentVel.Add(Delta.Mul(float32(math.Max(Move1, Move2))))
					}

					playersStateMu.Lock()
					playersState[playerId] = playerState{
						VelX: CurrentVel[0],
						VelY: CurrentVel[1],

						X: playersState[playerId].X + CurrentVel[0],
						Y: playersState[playerId].Y + CurrentVel[1],
						Z: lastMove.Z,

						conn: playersState[playerId].conn,
					}
					playersStateMu.Unlock()
				}

				// It takes State #0 and Command #0 to produce State #1.
				playersStateMu.Lock()
				serverLastAckedCmdSequenceNumber = r.CommandSequenceNumber + 1
				playersStateMu.Unlock()
			}
		}
	}
}

var serverLastAckedCmdSequenceNumber uint8
var lastUpdateSequenceNumber uint8 = 1

func sendServerUpdates() {
	for ; true; time.Sleep(time.Second / 20) {
		// Prepare a ServerUpdate packet.
		var p packet.ServerUpdate
		p.Type = packet.ServerUpdateType
		p.CurrentUpdateSequenceNumber = lastUpdateSequenceNumber
		state.Lock()
		p.PlayerUpdates = make([]packet.PlayerUpdate, state.TotalPlayerCount)
		state.Unlock()
		playersStateMu.Lock()
		for i, ps := range playersState {
			p.PlayerUpdates[i] = packet.PlayerUpdate{
				ActivePlayer: 1,
				State: &packet.State{
					CommandSequenceNumber: serverLastAckedCmdSequenceNumber,
					X: ps.X,
					Y: ps.Y,
					Z: ps.Z,
				},
			}
		}
		playersStateMu.Unlock()

		var buf bytes.Buffer
		err := binary.Write(&buf, binary.BigEndian, &p.UdpHeader)
		if err != nil {
			panic(err)
		}
		err = binary.Write(&buf, binary.BigEndian, &p.CurrentUpdateSequenceNumber)
		if err != nil {
			panic(err)
		}
		for _, playerUpdate := range p.PlayerUpdates {
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

		// Get a list of all connections to broadcast to.
		var cs []*Connection
		state.Lock()
		for _, c := range state.connections {
			if c.JoinStatus < IN_GAME {
				continue
			}
			cs = append(cs, c)
		}
		state.Unlock()

		// Broadcast the packet.
		for _, c := range cs {
			err = sendUdpPacket(c, buf.Bytes())
			if err != nil {
				panic(err)
			}
		}

		lastUpdateSequenceNumber++
	}
}

var lastPingData uint32
var pingSentTimes = make(map[uint32]time.Time) // TODO: Use.

func broadcastPingPacket() {
	const BROADCAST_PING_PERIOD = 2500 * time.Millisecond // How often to broadcast the Ping packet on the server.

	for ; true; time.Sleep(BROADCAST_PING_PERIOD) {
		// Prepare a Ping packet.
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

		// Get a list of all connections to broadcast to.
		var cs []*Connection
		state.Lock()
		for _, c := range state.connections {
			if c.JoinStatus < IN_GAME {
				continue
			}
			cs = append(cs, c)
		}
		state.Unlock()

		// Broadcast the packet.
		for _, c := range cs {
			err = sendUdpPacket(c, buf.Bytes())
			if err != nil {
				panic(err)
			}
		}

		lastPingData++
	}
}

func handleTcpConnection(client *Connection) {
	err := handleTcpConnection2(client)
	fmt.Println("tcp conn ended with:", err)

	state.Lock()
	for i, connection := range state.connections {
		if connection == client {
			// Delete without preserving order.
			state.connections[i], state.connections[len(state.connections)-1], state.connections = state.connections[len(state.connections)-1], nil, state.connections[:len(state.connections)-1]
			break
		}
	}
	state.Unlock()

	playersStateMu.Lock()
	for id, ps := range playersState {
		if ps.conn == client {
			delete(playersState, id)
			break
		}
	}
	playersStateMu.Unlock()

	client.tcp.Close()
}

func handleTcpConnection2(client *Connection) error {
	{
		buf, err := receiveTcpPacket(client)
		if err != nil {
			return err
		}
		var r packet.JoinServerRequest
		err = binary.Read(buf, binary.BigEndian, &r)
		if err != nil {
			return err
		}
		{
			r2 := r
			r2.Signature = 123
			goon.Dump(r2)
		}

		if r.Version != 1 ||
			r.Passphrase != [16]byte{'s', 'o', 'm', 'e', 'r', 'a', 'n', 'd', 'o', 'm', 'p', 'a', 's', 's', '0', '1'} {

			{
				var p packet.JoinServerRefuse
				p.Type = packet.JoinServerRefuseType
				p.RefuseReason = 123 // TODO.

				p.Length = 1

				var buf bytes.Buffer
				err := binary.Write(&buf, binary.BigEndian, &p)
				if err != nil {
					return err
				}
				err = sendTcpPacket(client, buf.Bytes())
				if err != nil {
					return err
				}

				return nil
			}
		}

		state.Lock()
		client.Signature = r.Signature
		client.JoinStatus = ACCEPTED
		state.Unlock()
	}

	{
		var p packet.JoinServerAccept
		p.Type = packet.JoinServerAcceptType
		p.YourPlayerId = 0
		p.TotalPlayerCount = state.TotalPlayerCount - 1

		p.Length = 2

		var buf bytes.Buffer
		err := binary.Write(&buf, binary.BigEndian, &p)
		if err != nil {
			return err
		}
		err = sendTcpPacket(client, buf.Bytes())
		if err != nil {
			return err
		}
	}

	{
		buf, err := receiveTcpPacket(client)
		if err != nil {
			return err
		}
		var r packet.LocalPlayerInfo
		err = binary.Read(buf, binary.BigEndian, &r.TcpHeader)
		if err != nil {
			return err
		}
		err = binary.Read(buf, binary.BigEndian, &r.NameLength)
		if err != nil {
			return err
		}
		r.Name = make([]byte, r.NameLength)
		err = binary.Read(buf, binary.BigEndian, &r.Name)
		if err != nil {
			return err
		}
		err = binary.Read(buf, binary.BigEndian, &r.CommandRate)
		if err != nil {
			return err
		}
		err = binary.Read(buf, binary.BigEndian, &r.UpdateRate)
		if err != nil {
			return err
		}
		goon.Dump(r)

		state.Lock()
		client.JoinStatus = PUBLIC_CLIENT
		state.Unlock()
	}

	{
		var p packet.LoadLevel
		p.Type = packet.LoadLevelType
		p.LevelFilename = []byte("test3")

		p.Length = uint16(len(p.LevelFilename))

		var buf bytes.Buffer
		err := binary.Write(&buf, binary.BigEndian, &p.TcpHeader)
		if err != nil {
			return err
		}
		err = binary.Write(&buf, binary.BigEndian, &p.LevelFilename)
		if err != nil {
			return err
		}
		err = sendTcpPacket(client, buf.Bytes())
		if err != nil {
			return err
		}
	}

	{
		var p packet.CurrentPlayersInfo
		p.Type = packet.CurrentPlayersInfoType
		p.Players = make([]packet.PlayerInfo, state.TotalPlayerCount)

		p.Length = 16 + uint16(len("shurcooL")) + 1

		var buf bytes.Buffer
		err := binary.Write(&buf, binary.BigEndian, &p.TcpHeader)
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

			err = binary.Write(&buf, binary.BigEndian, &playerInfo.NameLength)
			if err != nil {
				return err
			}

			if playerInfo.NameLength != 0 {
				err = binary.Write(&buf, binary.BigEndian, &playerInfo.Name)
				if err != nil {
					return err
				}
				err = binary.Write(&buf, binary.BigEndian, &playerInfo.Team)
				if err != nil {
					return err
				}

				if playerInfo.Team != 2 {
					err = binary.Write(&buf, binary.BigEndian, playerInfo.State)
					if err != nil {
						return err
					}
				}
			}
		}
		err = sendTcpPacket(client, buf.Bytes())
		if err != nil {
			return err
		}
	}

	{
		var p packet.EnterGamePermission
		p.Type = packet.EnterGamePermissionType

		p.Length = 0

		var buf bytes.Buffer
		err := binary.Write(&buf, binary.BigEndian, &p)
		if err != nil {
			return err
		}
		err = sendTcpPacket(client, buf.Bytes())
		if err != nil {
			return err
		}
	}

	{
		buf, err := receiveTcpPacket(client)
		if err != nil {
			return err
		}
		var r packet.EnteredGameNotification
		err = binary.Read(buf, binary.BigEndian, &r)
		if err != nil {
			return err
		}
		goon.Dump(r)

		state.Lock()
		client.JoinStatus = IN_GAME
		state.Unlock()
	}

	for {
		var team uint8

		{
			buf, err := receiveTcpPacket(client)
			if err != nil {
				return err
			}
			var r packet.JoinTeamRequest
			err = binary.Read(buf, binary.BigEndian, &r.TcpHeader)
			if err != nil {
				return err
			}
			// TODO: Handle potential PlayerNumber.
			err = binary.Read(buf, binary.BigEndian, &r.Team)
			if err != nil {
				return err
			}
			goon.Dump(r)

			team = r.Team
		}

		var playerId uint8 = func /*allocatePlayerId*/ () uint8 {
			playersStateMu.Lock()
			defer playersStateMu.Unlock()
			for id := 0; ; id++ {
				if _, ok := playersState[id]; !ok {
					return uint8(id)
				}
			}
		}()
		playersStateMu.Lock()
		playersState[int(playerId)] = playerState{conn: client}
		playersStateMu.Unlock()

		state.Lock()
		logicTime := float64(state.session.GlobalStateSequenceNumberTEST) + (time.Since(startedProcess).Seconds()-state.session.NextTickTime)*20
		fmt.Fprintf(os.Stderr, "%.3f: Pl#%v (%q) joined team %v at logic time %.2f/%v [server].\n", time.Since(startedProcess).Seconds(), playerId, "TODO: name", team, logicTime, state.session.GlobalStateSequenceNumberTEST)
		state.Unlock()

		{
			var p packet.PlayerJoinedTeam
			p.Type = packet.PlayerJoinedTeamType
			p.PlayerId = playerId
			p.Team = team

			// TODO: Proper spawn location calculation.
			ps := player0Spawn
			ps.X += float32(playerId) * 20
			ps.conn = playersState[int(playerId)].conn
			playersStateMu.Lock()
			playersState[int(playerId)] = ps
			playersStateMu.Unlock()

			state.Lock()
			if p.Team != 2 {
				p.State = &packet.State{
					CommandSequenceNumber: state.session.GlobalStateSequenceNumberTEST - 1,
					X: ps.X,
					Y: ps.Y,
					Z: ps.Z,
				}
			}
			state.Unlock()

			p.Length = 2
			if p.State != nil {
				p.Length += 13
			}

			var buf bytes.Buffer
			err := binary.Write(&buf, binary.BigEndian, &p.TcpHeader)
			if err != nil {
				return err
			}
			err = binary.Write(&buf, binary.BigEndian, &p.PlayerId)
			if err != nil {
				return err
			}
			err = binary.Write(&buf, binary.BigEndian, &p.Team)
			if err != nil {
				return err
			}
			if p.State != nil {
				err = binary.Write(&buf, binary.BigEndian, p.State)
				if err != nil {
					return err
				}
			}
			err = sendTcpPacket(client, buf.Bytes())
			if err != nil {
				return err
			}
		}
	}
}
