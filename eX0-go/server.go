// +build !js

package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"time"

	"github.com/shurcooL/eX0/eX0-go/packet"
	"github.com/shurcooL/go-goon"
	"golang.org/x/net/websocket"
)

type server struct {
	lastPingData uint32
	//pingSentTimes = make(map[uint32]time.Time) // TODO: Use.

	chanListener      chan *Connection
	chanListenerReply chan struct{}
}

func startServer() *server {
	s := &server{
		chanListener:      make(chan *Connection),
		chanListenerReply: make(chan struct{}),
	}

	go listenAndHandleTcp()
	go listenAndHandleUdp()
	go listenAndHandleWebSocket()
	go s.listenAndHandleChan()

	go s.broadcastPingPacket()

	time.Sleep(time.Millisecond) // HACK: Give some time for listeners to start.
	fmt.Println("Started server.")

	return s
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

func (s *server) listenAndHandleChan() {
	for clientToServerConn := range s.chanListener {

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

		s.chanListenerReply <- struct{}{}
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

		err = processUdpPacket(buf, c, udpAddr, mux)
		if err != nil {
			fmt.Println("handleUdpPacket:", err)
			if c != nil {
				c.tcp.Close()
			}
		}
	}
}

func processUdpPacket(buf io.Reader, c *Connection, udpAddr *net.UDPAddr, mux *Connection) error {
	var udpHeader packet.UdpHeader
	err := binary.Read(buf, binary.BigEndian, &udpHeader)
	if err != nil {
		return err
	}

	if c == nil && udpHeader.Type != packet.HandshakeType {
		return fmt.Errorf("nil c, unexpected udpHeader.Type: %v", udpHeader.Type)
	}

	switch udpHeader.Type {
	case packet.HandshakeType:
		var r packet.Handshake
		err = binary.Read(buf, binary.BigEndian, &r.Signature)
		if err != nil {
			return err
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
				return err
			}
			err = sendTcpPacket(c, buf.Bytes())
			if err != nil {
				return err
			}
		}
	case packet.TimeRequestType:
		var r packet.TimeRequest
		err = binary.Read(buf, binary.BigEndian, &r.SequenceNumber)
		if err != nil {
			return err
		}

		{
			var p packet.TimeResponse
			p.Type = packet.TimeResponseType
			p.SequenceNumber = r.SequenceNumber
			state.Lock()
			p.Time = time.Since(components.logic.started).Seconds()
			state.Unlock()

			var buf bytes.Buffer
			err := binary.Write(&buf, binary.BigEndian, &p)
			if err != nil {
				return err
			}
			err = sendUdpPacket(c, buf.Bytes())
			if err != nil {
				return err
			}
		}
	case packet.PongType:
		var r packet.Pong
		err = binary.Read(buf, binary.BigEndian, &r.PingData)
		if err != nil {
			return err
		}

		{
			var p packet.Pung
			p.Type = packet.PungType
			p.PingData = r.PingData
			p.Time = time.Since(components.logic.started).Seconds()

			var buf bytes.Buffer
			err := binary.Write(&buf, binary.BigEndian, &p)
			if err != nil {
				return err
			}
			err = sendUdpPacket(c, buf.Bytes())
			if err != nil {
				return err
			}
		}
	case packet.ClientCommandType:
		var r packet.ClientCommand
		err = binary.Read(buf, binary.BigEndian, &r.CommandSequenceNumber)
		if err != nil {
			return err
		}
		err = binary.Read(buf, binary.BigEndian, &r.CommandSeriesNumber)
		if err != nil {
			return err
		}
		err = binary.Read(buf, binary.BigEndian, &r.MovesCount)
		if err != nil {
			return err
		}
		r.MovesCount += 1 // De-normalize back to 1 (min value).
		r.Moves = make([]packet.Move, r.MovesCount)
		err = binary.Read(buf, binary.BigEndian, &r.Moves)
		if err != nil {
			return err
		}
		//goon.Dump(r)

		// TODO: Properly process and authenticate new result states.
		{
			playersStateMu.Lock()
			lastState := playersState[c.PlayerId].LatestAuthed()
			playersStateMu.Unlock()

			lastMove := r.Moves[len(r.Moves)-1] // There's always at least one move in a ClientCommand packet.

			// TODO: Check that state sn == command sn, etc.
			lastState.SequenceNumber = r.CommandSequenceNumber // HACK.
			newState := nextState(lastState, lastMove)

			playersStateMu.Lock()
			ps := playersState[c.PlayerId]
			ps.PushAuthed(newState)
			playersState[c.PlayerId] = ps
			playersStateMu.Unlock()
		}
	}
	return nil
}

func sendServerUpdates(c *Connection) {
	for ; true; time.Sleep(time.Second / 20) {
		playersStateMu.Lock()
		if _, ok := playersState[c.PlayerId]; !ok { // HACK.
			playersStateMu.Unlock()
			return
		}
		ps := playersState[c.PlayerId]
		ps.lastServerUpdateSequenceNumber++ // First update sent should have sequence number 1.
		playersState[c.PlayerId] = ps
		playersStateMu.Unlock()
		lastServerUpdateSequenceNumber := ps.lastServerUpdateSequenceNumber

		// Prepare a ServerUpdate packet.
		var p packet.ServerUpdate
		p.Type = packet.ServerUpdateType
		p.CurrentUpdateSequenceNumber = lastServerUpdateSequenceNumber
		state.Lock()
		p.PlayerUpdates = make([]packet.PlayerUpdate, state.TotalPlayerCount)
		state.Unlock()
		playersStateMu.Lock()
		for id, ps := range playersState {
			if ps.conn == nil || ps.conn.JoinStatus < IN_GAME || ps.Team == packet.Spectator {
				continue
			}
			p.PlayerUpdates[id] = packet.PlayerUpdate{
				ActivePlayer: 1,
				State: &packet.State{
					CommandSequenceNumber: ps.LatestAuthed().SequenceNumber,
					X: ps.LatestAuthed().X,
					Y: ps.LatestAuthed().Y,
					Z: ps.LatestAuthed().Z,
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

		// Send the packet to this client.
		err = sendUdpPacket(c, buf.Bytes())
		if err != nil {
			panic(err)
		}
	}
}

func (s *server) broadcastPingPacket() {
	const BROADCAST_PING_PERIOD = 2500 * time.Millisecond // How often to broadcast the Ping packet on the server.

	for ; true; time.Sleep(BROADCAST_PING_PERIOD) {
		// Prepare a Ping packet.
		var p packet.Ping
		p.Type = packet.PingType
		p.PingData = s.lastPingData
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

		// Broadcast the packet to all connections with at least IN_GAME.
		var cs []*Connection
		state.Lock()
		for _, c := range state.connections {
			if c.JoinStatus < IN_GAME {
				continue
			}
			cs = append(cs, c)
		}
		state.Unlock()
		for _, c := range cs {
			err = sendUdpPacket(c, buf.Bytes())
			if err != nil {
				panic(err)
			}
		}

		s.lastPingData++
	}
}

func handleTcpConnection(client *Connection) {
	err := handleTcpConnection2(client)
	fmt.Println("tcp conn ended with:", err)

	if client.JoinStatus >= PUBLIC_CLIENT {
		// Broadcast a PlayerLeftServer packet.
		err := func() error {
			var p packet.PlayerLeftServer
			p.Type = packet.PlayerLeftServerType

			p.Length = 1

			p.PlayerId = client.PlayerId

			var buf bytes.Buffer
			err := binary.Write(&buf, binary.BigEndian, &p.TcpHeader)
			if err != nil {
				return err
			}
			err = binary.Write(&buf, binary.BigEndian, &p.PlayerId)
			if err != nil {
				return err
			}

			// Broadcast the packet to all other connections with at least PUBLIC_CLIENT.
			var cs []*Connection
			state.Lock()
			for _, c := range state.connections {
				if c.JoinStatus < PUBLIC_CLIENT || c == client {
					continue
				}
				cs = append(cs, c)
			}
			state.Unlock()
			for _, c := range cs {
				err = sendTcpPacket(c, buf.Bytes())
				if err != nil {
					// TODO: This error handling is wrong. If fail to send to one client, should still send to others, etc.
					return err
				}
			}
			return nil
		}()
		if err != nil {
			fmt.Println("error while broadcasting PlayerLeftServer packet:", err)
		}
	}

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
	var playerId uint8

	{
		buf, _, err := receiveTcpPacket(client)
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
		playersStateMu.Lock()
		playerId = func /* allocatePlayerId */ () uint8 {
			for id := uint8(0); ; id++ {
				if _, taken := playersState[id]; !taken {
					return id
				}
			}
		}()
		serverFull := playerId >= state.TotalPlayerCount
		if !serverFull {
			playersState[playerId] = playerState{conn: client}
			client.Signature = r.Signature
			client.PlayerId = playerId
			client.JoinStatus = ACCEPTED
		}
		playersStateMu.Unlock()
		state.Unlock()

		if serverFull {
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
	}

	{
		var p packet.JoinServerAccept
		p.Type = packet.JoinServerAcceptType
		p.YourPlayerId = playerId
		state.Lock()
		p.TotalPlayerCount = state.TotalPlayerCount - 1
		state.Unlock()

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
		buf, _, err := receiveTcpPacket(client)
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

		// TODO: Ensure name is at least 1 character, because Current Players Info users empty name to indicate non-players...
		if len(r.Name) == 0 {
			r.Name = []byte("Unnamed Player")
		}

		state.Lock()
		playersStateMu.Lock()
		{
			ps := playersState[playerId]
			ps.Name = string(r.Name)
			ps.Team = packet.Spectator
			playersState[playerId] = ps
		}
		client.JoinStatus = PUBLIC_CLIENT
		playersStateMu.Unlock()
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

	// Include the client who's connecting and all clients with at least Public status.
	{
		var p packet.CurrentPlayersInfo
		p.Type = packet.CurrentPlayersInfoType
		p.Players = make([]packet.PlayerInfo, state.TotalPlayerCount)
		state.Lock()
		playersStateMu.Lock()
		p.Length += uint16(state.TotalPlayerCount)
		for _, c := range state.connections {
			if c.JoinStatus < PUBLIC_CLIENT {
				continue
			}
			ps := playersState[c.PlayerId]
			var playerInfo packet.PlayerInfo
			playerInfo.NameLength = uint8(len(ps.Name))
			if playerInfo.NameLength > 0 {
				playerInfo.Name = []byte(ps.Name)
				p.Length += uint16(playerInfo.NameLength)
				playerInfo.Team = ps.Team
				p.Length += 1
				if playerInfo.Team != packet.Spectator {
					playerInfo.State = &packet.State{
						CommandSequenceNumber: ps.LatestAuthed().SequenceNumber,
						X: ps.LatestAuthed().X,
						Y: ps.LatestAuthed().Y,
						Z: ps.LatestAuthed().Z,
					}
					p.Length += 1 + 4 + 4 + 4
				}
			}
			p.Players[c.PlayerId] = playerInfo
		}
		playersStateMu.Unlock()
		state.Unlock()

		var buf bytes.Buffer
		err := binary.Write(&buf, binary.BigEndian, &p.TcpHeader)
		if err != nil {
			return err
		}
		for _, playerInfo := range p.Players {
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

				if playerInfo.Team != packet.Spectator {
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
		var p packet.PlayerJoinedServer
		p.Type = packet.PlayerJoinedServerType

		playersStateMu.Lock()
		ps := playersState[playerId]
		playersStateMu.Unlock()

		p.PlayerId = playerId
		p.NameLength = uint8(len(ps.Name))
		p.Name = []byte(ps.Name)

		p.Length = 2 + uint16(len(p.Name))

		var buf bytes.Buffer
		err := binary.Write(&buf, binary.BigEndian, &p.TcpHeader)
		if err != nil {
			return err
		}
		err = binary.Write(&buf, binary.BigEndian, &p.PlayerId)
		if err != nil {
			return err
		}
		err = binary.Write(&buf, binary.BigEndian, &p.NameLength)
		if err != nil {
			return err
		}
		err = binary.Write(&buf, binary.BigEndian, &p.Name)
		if err != nil {
			return err
		}

		// Broadcast the packet to all other connections with at least PUBLIC_CLIENT.
		var cs []*Connection
		state.Lock()
		for _, c := range state.connections {
			if c.JoinStatus < PUBLIC_CLIENT || c == client {
				continue
			}
			cs = append(cs, c)
		}
		state.Unlock()
		for _, c := range cs {
			err = sendTcpPacket(c, buf.Bytes())
			if err != nil {
				return err
			}
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
		buf, _, err := receiveTcpPacket(client)
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

		// TODO: Who is responsible for stopping these? Currently having them quit on own in a hacky way, see what's the best way.
		go sendServerUpdates(client)
	}

	for {
		buf, tcpHeader, err := receiveTcpPacket(client)
		if err != nil {
			return err
		}

		switch tcpHeader.Type {
		case packet.JoinTeamRequestType:
			var team packet.Team

			{
				var r = packet.JoinTeamRequest{TcpHeader: tcpHeader}
				_, err = io.CopyN(ioutil.Discard, buf, packet.TcpHeaderSize)
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

			playersStateMu.Lock()
			{
				ps := playersState[playerId]
				ps.Team = team
				playersState[playerId] = ps
			}
			playersStateMu.Unlock()

			state.Lock()
			playersStateMu.Lock()
			logicTime := float64(state.session.GlobalStateSequenceNumberTEST) + (time.Since(components.logic.started).Seconds()-state.session.NextTickTime)*commandRate
			fmt.Fprintf(os.Stderr, "%.3f: Pl#%v (%q) joined team %v at logic time %.2f/%v [server].\n", time.Since(components.logic.started).Seconds(), playerId, playersState[playerId].Name, team, logicTime, state.session.GlobalStateSequenceNumberTEST)
			playersStateMu.Unlock()
			state.Unlock()

			{
				var p packet.PlayerJoinedTeam
				p.Type = packet.PlayerJoinedTeamType
				p.PlayerId = playerId
				p.Team = team

				state.Lock()
				playersStateMu.Lock()
				ps := playersState[playerId]
				{
					// TODO: Proper spawn location calculation.
					playerSpawn := playerPosVel{
						X: 25,
						Y: -220,
						Z: 6.0,
					}
					playerSpawn.X += float32(playerId) * 20

					ps.NewSeries()
					ps.PushAuthed(sequencedPlayerPosVel{
						playerPosVel:   playerSpawn,
						SequenceNumber: state.session.GlobalStateSequenceNumberTEST - 1,
					})
				}
				playersState[playerId] = ps
				playersStateMu.Unlock()
				state.Unlock()

				state.Lock()
				if p.Team != packet.Spectator {
					p.State = &packet.State{
						CommandSequenceNumber: ps.LatestAuthed().SequenceNumber,
						X: ps.LatestAuthed().X,
						Y: ps.LatestAuthed().Y,
						Z: ps.LatestAuthed().Z,
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

				// Broadcast the packet to all connections with at least PUBLIC_CLIENT.
				var cs []*Connection
				state.Lock()
				for _, c := range state.connections {
					if c.JoinStatus < PUBLIC_CLIENT {
						continue
					}
					cs = append(cs, c)
				}
				state.Unlock()
				for _, c := range cs {
					err = sendTcpPacket(c, buf.Bytes())
					if err != nil {
						return err
					}
				}
			}
		default:
			fmt.Println("[server] got unsupported tcp packet type:", tcpHeader.Type)
		}
	}
}
