// +build !js

package main

import (
	"flag"
	"fmt"
	"log"
	"math"
	"net"
	"net/http"
	"os"
	"sync"
	"time"

	"github.com/go-gl/mathgl/mgl32"
	"github.com/shurcooL/eX0/eX0-go/packet"
	"github.com/shurcooL/go-goon"
	"golang.org/x/net/websocket"
)

var (
	certFlag = flag.String("cert", "", "Cert file for wss on server, if empty then ws is used.")
	keyFlag  = flag.String("key", "", "Key file for wss on server, if empty then ws is used.")
)

type server struct {
	logic *logic

	connectionsMu sync.Mutex
	connections   []*Connection

	lastPingData uint32

	// TODO: Consider moving these into *Connection or Player or something that will "go away" when that connection dies.
	//       That way no need to clear/remove these entries manually when that player is gone.
	pingSentTimesMu sync.Mutex
	pingSentTimes   []map[uint32]time.Time // Player ID -> (PingData -> Time).
	lastLatenciesMu sync.Mutex
	lastLatencies   []uint16 // Index is Player ID. Units are 0.1 ms.

	chanListener      chan *Connection
	chanListenerReply chan struct{}
}

const serverLevelFilename = "test3"

func newServer() *server {
	return &server{
		logic:             newLogic(),
		chanListener:      make(chan *Connection),
		chanListenerReply: make(chan struct{}),
	}
}

func (s *server) start() {
	s.logic.start()
	state.Lock()
	s.logic.TotalPlayerCount = 16
	s.pingSentTimes = make([]map[uint32]time.Time, s.logic.TotalPlayerCount)
	s.lastLatencies = make([]uint16, s.logic.TotalPlayerCount)
	state.Unlock()

	if level, err := newLevel(serverLevelFilename + ".wwl"); err != nil {
		log.Fatalln("failed to load level:", err)
	} else {
		s.logic.level = level
	}

	// Normal TCP + UDP.
	go s.listenAndHandleTCP(tcpUDPNetwork{})
	go s.listenAndHandleUDP(tcpUDPNetwork{})

	// Virtual TCP and UDP via physical TCP or WebSocket.
	go s.listenAndHandleTCPRaw(tcpNetwork{useWebSocket: false})
	go s.listenAndHandleTCPWebSocket(tcpNetwork{useWebSocket: true})

	// TCP and UDP via local channels.
	go s.listenAndHandleChan(chanNetwork{})

	go s.broadcastPingPacket()

	time.Sleep(time.Millisecond) // HACK: Give some time for listeners to start.
	fmt.Println("Started server.")
}

func (s *server) listenAndHandleTCP(nw network) {
	ln, err := net.Listen("tcp", ":25045")
	if err != nil {
		panic(err)
	}

	for {
		tcp, err := ln.Accept()
		if err != nil {
			panic(err)
		}

		client := nw.newConnection()
		client.tcp = tcp
		client.JoinStatus = TCP_CONNECTED
		nw.dialedClient(client)
		s.connectionsMu.Lock()
		s.connections = append(s.connections, client)
		s.connectionsMu.Unlock()

		go s.handleTCPConnection(client)
		// Normal TCP + UDP. No need to handle UDP directly, since it will come in via the UDP mux.
	}
}
func (s *server) listenAndHandleUDP(nw network) {
	udpAddr, err := net.ResolveUDPAddr("udp", ":25045")
	if err != nil {
		panic(err)
	}
	ln, err := net.ListenUDP("udp", udpAddr)
	if err != nil {
		panic(err)
	}
	mux := &Connection{
		nw:  nw,
		udp: ln,
	}

	s.handleUDP(mux)
}

func (s *server) listenAndHandleTCPRaw(nw network) {
	ln, err := net.Listen("tcp", ":25046")
	if err != nil {
		panic(err)
	}

	for {
		tcp, err := ln.Accept()
		if err != nil {
			panic(err)
		}

		client := nw.newConnection()
		client.tcp = tcp
		client.JoinStatus = TCP_CONNECTED
		nw.dialedClient(client)
		s.connectionsMu.Lock()
		s.connections = append(s.connections, client)
		s.connectionsMu.Unlock()

		go s.handleTCPConnection(client)
		// TODO: Who should be responsible for stopping these? Currently having them quit on own in a hacky way, see what's the best way. Maybe context.Context?
		go s.handleUDP(client) // tcp-specific. Need to handle UDP directly on same connection, since there won't be a separate one.
	}
}

func (s *server) listenAndHandleTCPWebSocket(nw network) {
	h := websocket.Handler(func(conn *websocket.Conn) {
		// Why is this exported field undocumented?
		//
		// It needs to be set to websocket.BinaryFrame so that
		// the Write method sends bytes as binary rather than text frames.
		conn.PayloadType = websocket.BinaryFrame

		client := nw.newConnection()
		client.tcp = conn
		client.JoinStatus = TCP_CONNECTED
		nw.dialedClient(client)
		s.connectionsMu.Lock()
		s.connections = append(s.connections, client)
		s.connectionsMu.Unlock()

		// TODO: Who should be responsible for stopping these? Currently having them quit on own in a hacky way, see what's the best way. Maybe context.Context?
		go s.handleUDP(client) // tcp-specific. Need to handle UDP directly on same connection, since there won't be a separate one.
		s.handleTCPConnection(client)
		// Do not return until handleTCPConnection does, else WebSocket gets closed.
	})
	switch {
	case *certFlag == "" && *keyFlag == "":
		err := http.ListenAndServe(":25047", h)
		if err != nil {
			panic(err)
		}
	default:
		err := http.ListenAndServeTLS(":25047", *certFlag, *keyFlag, h)
		if err != nil {
			panic(err)
		}
	}
}

func (s *server) listenAndHandleChan(nw network) {
	for clientToServerConn := range s.chanListener {

		serverToClientConn := nw.newConnection()
		// Join server <-> client channel ends together.
		serverToClientConn.sendTCP = clientToServerConn.recvTCP
		serverToClientConn.recvTCP = clientToServerConn.sendTCP
		serverToClientConn.sendUDP = clientToServerConn.recvUDP
		serverToClientConn.recvUDP = clientToServerConn.sendUDP
		serverToClientConn.JoinStatus = TCP_CONNECTED
		nw.dialedClient(serverToClientConn)

		s.connectionsMu.Lock()
		s.connections = append(s.connections, serverToClientConn)
		s.connectionsMu.Unlock()

		go s.handleTCPConnection(serverToClientConn)
		// TODO: Who should be responsible for stopping these? Currently having them quit on own in a hacky way, see what's the best way. Maybe context.Context?
		go s.handleUDP(serverToClientConn) // chan-specific. Need to handle UDP directly on same connection, since there won't be a separate one.

		s.chanListenerReply <- struct{}{}
	}
}

func (s *server) handleUDP(mux *Connection) {
	for {
		p, c, udpAddr, err := receiveUDPPacketFrom2(s, mux, s.logic.TotalPlayerCount)
		if err != nil {
			fmt.Println("udp conn ended with:", err)
			return
		}

		err = s.processUDPPacket(p, c, udpAddr, mux)
		if err != nil {
			fmt.Println("handleUDPPacket:", err)
			if c != nil {
				c.tcp.Close()
			}
			// TODO: Should it return or continue on error? Depends on
			//       whether it's a per-connection or global UDP handler...
			return
		}
	}
}

func (s *server) processUDPPacket(r interface{}, from *Connection, udpAddr *net.UDPAddr, mux *Connection) error {
	switch r := r.(type) {
	case packet.Handshake:
		{
			r2 := r
			r2.Signature = 123
			goon.Dump(r2)
		}

		s.connectionsMu.Lock()
		for _, c := range s.connections {
			if c.Signature == r.Signature {
				c.JoinStatus = UDP_CONNECTED // TODO: Fix JoinStatus race with line 398.
				c.udp = mux.udp
				c.UDPAddr = udpAddr

				from = c
				break
			}
		}
		s.connectionsMu.Unlock()

		if from != nil {
			var p packet.UDPConnectionEstablished
			err := sendTCPPacket(from, &p)
			if err != nil {
				return err
			}
		}
	case packet.TimeRequest:
		{
			var p packet.TimeResponse
			p.SequenceNumber = r.SequenceNumber
			state.Lock()
			p.Time = time.Since(s.logic.started).Seconds()
			state.Unlock()

			err := sendUDPPacket(from, &p)
			if err != nil {
				return err
			}
		}
	case packet.Pong:
		localTimeAtPongReceive := time.Now()

		s.pingSentTimesMu.Lock()
		// Get the time sent of the matching Pong packet.
		if localTimeAtPingSend, ok := s.pingSentTimes[from.PlayerID][r.PingData]; ok {
			delete(s.pingSentTimes[from.PlayerID], r.PingData)

			latency := uint16(localTimeAtPongReceive.Sub(localTimeAtPingSend).Seconds() * 10000) // Units are 0.1 ms.
			s.lastLatenciesMu.Lock()
			s.lastLatencies[from.PlayerID] = latency
			s.lastLatenciesMu.Unlock()
		}
		s.pingSentTimesMu.Unlock()

		{
			var p packet.Pung
			p.PingData = r.PingData
			state.Lock() // For logic.started.
			p.Time = time.Since(s.logic.started).Seconds()
			state.Unlock()

			err := sendUDPPacket(from, &p)
			if err != nil {
				return err
			}
		}
	case packet.ClientCommand:
		// TODO: Properly process and authenticate new result states.
		{
			s.logic.playersStateMu.Lock()
			ps, ok := s.logic.playersState[from.PlayerID]
			if !ok { // TODO: Who should be responsible for stopping these? Currently having them quit on own in a hacky way, see what's the best way. Maybe context.Context?
				s.logic.playersStateMu.Unlock()
				return fmt.Errorf("player %d doesn't exist in s.logic.playersState", from.PlayerID)
			}
			lastState := ps.LatestAuthed()
			s.logic.playersStateMu.Unlock()

			lastMove := r.Moves[len(r.Moves)-1] // There's always at least one move in a ClientCommand packet.

			// TODO: Check that state sn == command sn, etc.
			lastState.SequenceNumber = r.CommandSequenceNumber // HACK.
			newState := s.logic.nextState(lastState, lastMove)

			s.logic.playersStateMu.Lock()
			ps = s.logic.playersState[from.PlayerID]
			ps.PushAuthed(s.logic, newState)
			s.logic.playersState[from.PlayerID] = ps
			s.logic.playersStateMu.Unlock()
		}
	case packet.WeaponCommand:
		// TODO: Deduplicate/unify with packet.WeaponAction processing?
		switch r.Action {
		case packet.Fire:
			// TODO: In the future, go through a player.weapon.fire() abstraction
			//       rather than creating bullets directly here ourselves.

			// Find where the player was at the time the weapon fired.
			s.logic.playersStateMu.Lock()
			ps, ok := s.logic.playersState[from.PlayerID]
			s.logic.playersStateMu.Unlock()
			if !ok {
				return fmt.Errorf("player %d doesn't exist in s.logic.playersState", from.PlayerID)
			}

			pos := ps.interpolated(gameMoment(r.Time))
			vel := mgl32.Vec2{float32(math.Sin(float64(r.Z))), float32(math.Cos(float64(r.Z)))}.Mul(275)

			s.logic.particles.Add(mgl32.Vec2{pos.X, pos.Y}, vel)

			// TODO: Factor out.
			{
				var p packet.WeaponAction
				p.PlayerID = from.PlayerID
				p.Action = packet.Fire
				p.Time = r.Time
				p.Z = r.Z

				// Broadcast the packet to all other connections with at least IN_GAME.
				var cs []*Connection
				s.connectionsMu.Lock()
				for _, c := range s.connections {
					if c.JoinStatus < IN_GAME || c == from {
						continue
					}
					cs = append(cs, c)
				}
				s.connectionsMu.Unlock()
				err := broadcastUDPPacket(cs, &p)
				if err != nil {
					return err
				}
			}
		default:
			goon.DumpExpr(r)
		}
	}
	return nil
}

func (s *server) sendServerUpdates(c *Connection) {
	for range time.Tick(time.Second / 20) {
		s.logic.playersStateMu.Lock()
		if _, ok := s.logic.playersState[c.PlayerID]; !ok { // TODO: Who should be responsible for stopping these? Currently having them quit on own in a hacky way, see what's the best way. Maybe context.Context?
			s.logic.playersStateMu.Unlock()
			return
		}
		ps := s.logic.playersState[c.PlayerID]
		ps.lastServerUpdateSequenceNumber++ // First update sent should have sequence number 1.
		s.logic.playersState[c.PlayerID] = ps
		s.logic.playersStateMu.Unlock()
		lastServerUpdateSequenceNumber := ps.lastServerUpdateSequenceNumber

		// Prepare a ServerUpdate packet.
		var p packet.ServerUpdate
		p.CurrentUpdateSequenceNumber = lastServerUpdateSequenceNumber
		state.Lock()
		p.PlayerUpdates = make([]packet.PlayerUpdate, s.logic.TotalPlayerCount)
		state.Unlock()
		s.logic.playersStateMu.Lock()
		for id, ps := range s.logic.playersState {
			if ps.conn == nil || ps.conn.JoinStatus < IN_GAME || ps.Team == packet.Spectator { // TODO: Fix JoinStatus race with line 246.
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
		s.logic.playersStateMu.Unlock()

		// Send the packet to this client.
		err := sendUDPPacket(c, &p)
		if err != nil {
			panic(err)
		}
	}
}

func (s *server) broadcastPingPacket() {
	const BROADCAST_PING_PERIOD = 2500 * time.Millisecond // How often to broadcast the Ping packet on the server.

	for range time.Tick(BROADCAST_PING_PERIOD) {
		// Prepare a Ping packet.
		var p packet.Ping
		p.PingData = s.lastPingData
		p.LastLatencies = make([]uint16, s.logic.TotalPlayerCount)
		s.lastLatenciesMu.Lock()
		copy(p.LastLatencies, s.lastLatencies)
		s.lastLatenciesMu.Unlock()

		b, err := p.MarshalBinary()
		if err != nil {
			panic(err)
		}

		// Broadcast the packet to all connections with at least IN_GAME.
		var cs []*Connection
		s.connectionsMu.Lock()
		for _, c := range s.connections {
			if c.JoinStatus < IN_GAME {
				continue
			}
			cs = append(cs, c)
		}
		s.connectionsMu.Unlock()
		for _, c := range cs {
			s.pingSentTimesMu.Lock()
			s.pingSentTimes[c.PlayerID][p.PingData] = time.Now()
			s.pingSentTimesMu.Unlock()

			err = c.nw.sendUDPPacketBytes(c, b) // TODO: See if this can/should be replaced with sendUDPPacket or sendTCPPacket, and not affect timing code above negatively.
			if err != nil {
				panic(err)
			}
		}

		s.lastPingData++
	}
}

func (s *server) handleTCPConnection(client *Connection) {
	err := s.handleTCPConnection2(client)
	fmt.Println("tcp conn ended with:", err)

	if client.JoinStatus >= PUBLIC_CLIENT {
		// Broadcast a PlayerLeftServer packet.
		err := func() error {
			var p packet.PlayerLeftServer
			p.PlayerID = client.PlayerID

			// Broadcast the packet to all other connections with at least PUBLIC_CLIENT.
			var cs []*Connection
			s.connectionsMu.Lock()
			for _, c := range s.connections {
				if c.JoinStatus < PUBLIC_CLIENT || c == client {
					continue
				}
				cs = append(cs, c)
			}
			s.connectionsMu.Unlock()
			err = broadcastTCPPacket(cs, &p)
			// TODO: This error handling is wrong. If fail to send to one client, should still send to others, etc.
			return err
		}()
		if err != nil {
			fmt.Println("error while broadcasting PlayerLeftServer packet:", err)
		}
	}

	if client.JoinStatus >= ACCEPTED {
		s.pingSentTimesMu.Lock()
		// Clear the map.
		for k := range s.pingSentTimes[client.PlayerID] {
			delete(s.pingSentTimes[client.PlayerID], k)
		}
		s.pingSentTimesMu.Unlock()

		s.lastLatenciesMu.Lock()
		s.lastLatencies[client.PlayerID] = 0
		s.lastLatenciesMu.Unlock()
	}

	s.connectionsMu.Lock()
	for i, connection := range s.connections {
		if connection == client {
			// Delete without preserving order.
			s.connections[i], s.connections[len(s.connections)-1], s.connections = s.connections[len(s.connections)-1], nil, s.connections[:len(s.connections)-1]
			break
		}
	}
	s.connectionsMu.Unlock()

	s.logic.playersStateMu.Lock()
	for id, ps := range s.logic.playersState {
		if ps.conn == client {
			delete(s.logic.playersState, id)
			break
		}
	}
	s.logic.playersStateMu.Unlock()

	client.tcp.Close()
}

func (s *server) handleTCPConnection2(client *Connection) error {
	var playerID uint8

	{
		p, err := receiveTCPPacket2(client, s.logic.TotalPlayerCount)
		if err != nil {
			return err
		}
		r, ok := p.(packet.JoinServerRequest)
		if !ok {
			return fmt.Errorf("unexpected packet type: %T", p)
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
				p.RefuseReason = 123 // TODO.
				err := sendTCPPacket(client, &p)
				return err
			}
		}

		s.connectionsMu.Lock()
		s.logic.playersStateMu.Lock()
		playerID = func /* allocatePlayerID */ () uint8 {
			for id := uint8(0); ; id++ {
				if _, taken := s.logic.playersState[id]; !taken {
					return id
				}
			}
		}()
		serverFull := playerID >= s.logic.TotalPlayerCount
		if !serverFull {
			s.logic.playersState[playerID] = playerState{conn: client}
			client.Signature = r.Signature
			client.PlayerID = playerID
			client.JoinStatus = ACCEPTED

			s.pingSentTimesMu.Lock()
			s.pingSentTimes[client.PlayerID] = make(map[uint32]time.Time)
			s.pingSentTimesMu.Unlock()
		}
		s.logic.playersStateMu.Unlock()
		s.connectionsMu.Unlock()

		if serverFull {
			{
				var p packet.JoinServerRefuse
				p.RefuseReason = 123 // TODO.
				err := sendTCPPacket(client, &p)
				return err
			}
		}
	}

	{
		var p packet.JoinServerAccept
		p.YourPlayerID = playerID
		state.Lock()
		p.TotalPlayerCount = s.logic.TotalPlayerCount - 1
		state.Unlock()
		err := sendTCPPacket(client, &p)
		if err != nil {
			return err
		}
	}

	{
		p, err := receiveTCPPacket2(client, s.logic.TotalPlayerCount)
		if err != nil {
			return err
		}
		r, ok := p.(packet.LocalPlayerInfo)
		if !ok {
			return fmt.Errorf("unexpected packet type: %T", p)
		}
		goon.Dump(r)

		// TODO: Ensure name is at least 1 character, because Current Players Info users empty name to indicate non-players...
		if len(r.Name) == 0 {
			r.Name = []byte("Unnamed Player")
		}

		s.connectionsMu.Lock()
		s.logic.playersStateMu.Lock()
		{
			ps := s.logic.playersState[playerID]
			ps.Name = string(r.Name)
			ps.Team = packet.Spectator
			s.logic.playersState[playerID] = ps
		}
		client.JoinStatus = PUBLIC_CLIENT
		s.logic.playersStateMu.Unlock()
		s.connectionsMu.Unlock()
	}

	{
		var p packet.LoadLevel
		p.LevelFilename = []byte(serverLevelFilename)
		err := sendTCPPacket(client, &p)
		if err != nil {
			return err
		}
	}

	// Include the client who's connecting and all clients with at least Public status.
	{
		var p packet.CurrentPlayersInfo
		p.Players = make([]packet.PlayerInfo, s.logic.TotalPlayerCount)
		state.Lock()
		s.logic.playersStateMu.Lock()
		for _, c := range s.connections {
			if c.JoinStatus < PUBLIC_CLIENT {
				continue
			}
			ps := s.logic.playersState[c.PlayerID]
			var playerInfo packet.PlayerInfo
			playerInfo.NameLength = uint8(len(ps.Name))
			if playerInfo.NameLength > 0 {
				playerInfo.Name = []byte(ps.Name)
				playerInfo.Team = ps.Team
				if playerInfo.Team != packet.Spectator {
					playerInfo.State = &packet.State{
						CommandSequenceNumber: ps.LatestAuthed().SequenceNumber,
						X: ps.LatestAuthed().X,
						Y: ps.LatestAuthed().Y,
						Z: ps.LatestAuthed().Z,
					}
				}
			}
			p.Players[c.PlayerID] = playerInfo
		}
		s.logic.playersStateMu.Unlock()
		state.Unlock()
		err := sendTCPPacket(client, &p)
		if err != nil {
			return err
		}
	}

	{
		var p packet.PlayerJoinedServer
		s.logic.playersStateMu.Lock()
		ps := s.logic.playersState[playerID]
		s.logic.playersStateMu.Unlock()
		p.PlayerID = playerID
		p.NameLength = uint8(len(ps.Name))
		p.Name = []byte(ps.Name)

		// Broadcast the packet to all other connections with at least PUBLIC_CLIENT.
		var cs []*Connection
		s.connectionsMu.Lock()
		for _, c := range s.connections {
			if c.JoinStatus < PUBLIC_CLIENT || c == client {
				continue
			}
			cs = append(cs, c)
		}
		s.connectionsMu.Unlock()
		err := broadcastTCPPacket(cs, &p)
		if err != nil {
			return err
		}
	}

	{
		var p packet.EnterGamePermission
		err := sendTCPPacket(client, &p)
		if err != nil {
			return err
		}
	}

	{
		p, err := receiveTCPPacket2(client, s.logic.TotalPlayerCount)
		if err != nil {
			return err
		}
		r, ok := p.(packet.EnteredGameNotification)
		if !ok {
			return fmt.Errorf("unexpected packet type: %T", p)
		}
		goon.Dump(r)

		s.connectionsMu.Lock()
		client.JoinStatus = IN_GAME // TODO: Fix JoinStatus race with line view.go:155.
		s.connectionsMu.Unlock()

		// TODO: Who should be responsible for stopping these? Currently having them quit on own in a hacky way, see what's the best way. Maybe context.Context?
		go s.sendServerUpdates(client)
	}

	for {
		p, err := receiveTCPPacket2(client, s.logic.TotalPlayerCount)
		if err != nil {
			return err
		}

		switch r := p.(type) {
		case packet.JoinTeamRequest:
			var team packet.Team

			{
				goon.Dump(r)

				team = r.Team
			}

			s.logic.playersStateMu.Lock()
			{
				ps := s.logic.playersState[playerID]
				ps.Team = team
				s.logic.playersState[playerID] = ps
			}
			s.logic.playersStateMu.Unlock()

			state.Lock()
			s.logic.playersStateMu.Lock()
			logicTime := float64(s.logic.GlobalStateSequenceNumber) + (time.Since(s.logic.started).Seconds()-s.logic.NextTickTime)*commandRate
			fmt.Fprintf(os.Stderr, "%.3f: Pl#%v (%q) joined team %v at logic time %.2f/%v [server].\n", time.Since(s.logic.started).Seconds(), playerID, s.logic.playersState[playerID].Name, team, logicTime, s.logic.GlobalStateSequenceNumber)
			s.logic.playersStateMu.Unlock()
			state.Unlock()

			{
				var p packet.PlayerJoinedTeam
				p.PlayerID = playerID
				p.Team = team

				state.Lock()
				s.logic.playersStateMu.Lock()
				ps := s.logic.playersState[playerID]
				{
					// TODO: Proper spawn location calculation.
					playerSpawn := playerPosVel{
						X: -25,
						Y: -160,
						Z: 6.0,
					}
					playerSpawn.X += float32(playerID) * 20

					ps.NewSeries()
					ps.PushAuthed(s.logic, sequencedPlayerPosVel{
						playerPosVel:   playerSpawn,
						SequenceNumber: s.logic.GlobalStateSequenceNumber - 1,
					})
					ps.Health = 100
				}
				s.logic.playersState[playerID] = ps
				s.logic.playersStateMu.Unlock()
				state.Unlock()

				if p.Team != packet.Spectator {
					p.State = &packet.State{
						CommandSequenceNumber: ps.LatestAuthed().SequenceNumber,
						X: ps.LatestAuthed().X,
						Y: ps.LatestAuthed().Y,
						Z: ps.LatestAuthed().Z,
					}
				}

				// Broadcast the packet to all connections with at least PUBLIC_CLIENT.
				var cs []*Connection
				s.connectionsMu.Lock()
				for _, c := range s.connections {
					if c.JoinStatus < PUBLIC_CLIENT {
						continue
					}
					cs = append(cs, c)
				}
				s.connectionsMu.Unlock()
				err := broadcastTCPPacket(cs, &p)
				if err != nil {
					return err
				}
			}
		default:
			fmt.Println("[server] got unsupported TCP packet type")
		}
	}
}
