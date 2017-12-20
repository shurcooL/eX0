package main

import (
	"flag"
	"fmt"
	"log"
	"math"
	"os"
	"time"

	"github.com/shurcooL/eX0/eX0-go/packet"
	"github.com/shurcooL/go-goon"
)

var (
	hostFlag   = flag.String("host", "localhost", "Server host (without port) for client to connect to.")
	secureFlag = flag.Bool("secure", false, "Secure connection to server (e.g., use wss instead of ws).")
	nameFlag   = flag.String("name", "Unnamed Player", "Local client player name.")
)

type client struct {
	logic *logic

	playerID uint8

	ZOffset float32

	serverConn *Connection

	sentTimeRequestPacketTimes map[uint8]float64
	trpReceived                int
	shortestLatency            float64
	shortestLatencyLocalTime   float64
	shortestLatencyRemoteTime  float64
	finishedSyncingClock       chan struct{}

	pongSentTimes map[uint32]time.Time // PingData -> Time.
	lastLatencies []uint16             // Index is Player ID. Units are 0.1 ms.
}

func newClient(nw network) *client {
	return &client{
		logic:                      newLogic(),
		serverConn:                 nw.newConnection(),
		sentTimeRequestPacketTimes: make(map[uint8]float64),
		shortestLatency:            math.MaxFloat64,
		finishedSyncingClock:       make(chan struct{}),
		pongSentTimes:              make(map[uint32]time.Time),
	}
}

func (c *client) start() {
	c.logic.start()
	c.logic.Input <- func(logic *logic) packet.Move {
		return packet.Move{
			MoveDirection: -1,
			Z:             logic.playersState[c.playerID].LatestAuthed().Z,
		}
	}
	c.connectToServer()
}

func (c *client) connectToServer() {
	s := c.serverConn

	s.nw.dialServer(s)

	s.Signature = uint64(time.Now().UnixNano())

	{
		var p = packet.JoinServerRequest{
			TCPHeader: packet.TCPHeader{
				Type: packet.JoinServerRequestType,
			},
			Version:    1,
			Passphrase: [16]byte{'s', 'o', 'm', 'e', 'r', 'a', 'n', 'd', 'o', 'm', 'p', 'a', 's', 's', '0', '1'},
			Signature:  s.Signature,
		}
		err := sendTCPPacket(s, &p)
		if err != nil {
			panic(err)
		}
	}

	{
		p, err := receiveTCPPacket2(s, c.logic.TotalPlayerCount)
		if err != nil {
			panic(err)
		}
		r, ok := p.(packet.JoinServerAccept)
		if !ok {
			panic(fmt.Errorf("unexpected packet type: %T", p))
		}
		goon.Dump(r)

		state.Lock()
		c.playerID = r.YourPlayerID
		c.logic.TotalPlayerCount = r.TotalPlayerCount + 1
		c.lastLatencies = make([]uint16, c.logic.TotalPlayerCount)
		s.JoinStatus = ACCEPTED
		state.Unlock()
	}

	// Upgrade connection to UDP at this point, start listening for UDP packets.
	go c.handleUDP(s)

	{
		// TODO: Try sending multiple times, else it might not get received.
		var p packet.Handshake
		p.Signature = s.Signature
		err := sendUDPPacket(s, &p)
		if err != nil {
			panic(err)
		}
	}

	{
		p, err := receiveTCPPacket2(s, c.logic.TotalPlayerCount)
		if err != nil {
			panic(err)
		}
		r, ok := p.(packet.UDPConnectionEstablished)
		if !ok {
			panic(fmt.Errorf("unexpected packet type: %T", p))
		}
		goon.Dump(r)

		state.Lock()
		s.JoinStatus = UDP_CONNECTED
		state.Unlock()
	}

	{
		// Start syncing the clock, send a Time Request packet every 50 ms.
		go func() {
			var sn uint8

			for ; ; time.Sleep(50 * time.Millisecond) {
				select {
				case <-c.finishedSyncingClock:
					return
				default:
				}

				var p packet.TimeRequest
				p.SequenceNumber = sn
				sn++

				state.Lock()
				c.sentTimeRequestPacketTimes[p.SequenceNumber] = time.Since(c.logic.started).Seconds()
				state.Unlock()

				err := sendUDPPacket(s, &p)
				if err != nil {
					panic(err)
				}
			}
		}()
	}

	{
		var p packet.LocalPlayerInfo
		p.NameLength = uint8(len(*nameFlag))
		p.Name = []byte(*nameFlag)
		p.CommandRate = 20
		p.UpdateRate = 20
		err := sendTCPPacket(s, &p)
		if err != nil {
			panic(err)
		}

		state.Lock()
		s.JoinStatus = PUBLIC_CLIENT
		state.Unlock()
	}

	{
		p, err := receiveTCPPacket2(s, c.logic.TotalPlayerCount)
		if err != nil {
			panic(err)
		}
		r, ok := p.(packet.LoadLevel)
		if !ok {
			panic(fmt.Errorf("unexpected packet type: %T", p))
		}
		goon.Dump(r)
		goon.Dump(string(r.LevelFilename))

		if level, err := newLevel(string(r.LevelFilename) + ".wwl"); err != nil {
			log.Fatalln("failed to load level:", err)
		} else {
			c.logic.level = level
		}
	}

	{
		p, err := receiveTCPPacket2(s, c.logic.TotalPlayerCount)
		if err != nil {
			panic(err)
		}
		r, ok := p.(packet.CurrentPlayersInfo)
		if !ok {
			panic(fmt.Errorf("unexpected packet type: %T", p))
		}
		goon.Dump(r)

		state.Lock()
		c.logic.playersStateMu.Lock()
		for id, p := range r.Players {
			if p.NameLength == 0 {
				continue
			}
			ps := playerState{
				Name:   string(p.Name),
				Team:   p.Team,
				Health: 100, // TODO: Transmit information from server; players may have less health.
			}
			if p.State != nil {
				ps.PushAuthed(c.logic, sequencedPlayerPosVel{
					playerPosVel: playerPosVel{
						X: p.State.X,
						Y: p.State.Y,
						Z: p.State.Z,
					},
					SequenceNumber: p.State.CommandSequenceNumber,
				})
			}
			c.logic.playersState[uint8(id)] = ps
		}
		c.logic.playersStateMu.Unlock()
		state.Unlock()
	}

	{
		p, err := receiveTCPPacket2(s, c.logic.TotalPlayerCount)
		if err != nil {
			panic(err)
		}
		r, ok := p.(packet.EnterGamePermission)
		if !ok {
			panic(fmt.Errorf("unexpected packet type: %T", p))
		}
		goon.Dump(r)
	}

	<-c.finishedSyncingClock

	state.Lock()
	s.JoinStatus = IN_GAME
	state.Unlock()

	{
		var p packet.EnteredGameNotification
		err := sendTCPPacket(s, &p)
		if err != nil {
			panic(err)
		}
	}

	time.Sleep(3 * time.Second)

	fmt.Println("Client connected and joining team.")
	var debugFirstJoin = true

	{
		var p packet.JoinTeamRequest
		p.Team = packet.Red
		err := sendTCPPacket(s, &p)
		if err != nil {
			panic(err)
		}
	}

	go func() {
		for {
			r, err := receiveTCPPacket2(s, c.logic.TotalPlayerCount)
			if err != nil {
				panic(err)
			}

			switch r := r.(type) {
			case packet.PlayerJoinedServer:
				ps := playerState{
					Name: string(r.Name),
					Team: packet.Spectator,
				}
				c.logic.playersStateMu.Lock()
				c.logic.playersState[r.PlayerID] = ps
				c.logic.playersStateMu.Unlock()

				fmt.Printf("%v is entering the game.\n", ps.Name)
			case packet.PlayerLeftServer:
				c.logic.playersStateMu.Lock()
				ps := c.logic.playersState[r.PlayerID]
				delete(c.logic.playersState, r.PlayerID)
				c.logic.playersStateMu.Unlock()

				fmt.Printf("%v left the game.\n", ps.Name)
			case packet.PlayerJoinedTeam:
				state.Lock()
				c.logic.playersStateMu.Lock()
				logicTime := float64(c.logic.GlobalStateSequenceNumber) + (time.Since(c.logic.started).Seconds()-c.logic.NextTickTime)*commandRate
				fmt.Fprintf(os.Stderr, "%.3f: Pl#%v (%q) joined team %v at logic time %.2f/%v [client].\n", time.Since(c.logic.started).Seconds(), r.PlayerID, c.logic.playersState[r.PlayerID].Name, r.Team, logicTime, c.logic.GlobalStateSequenceNumber)
				c.logic.playersStateMu.Unlock()
				state.Unlock()

				if debugFirstJoin {
					debugFirstJoin = false
					r2 := r
					r2.State = &packet.State{CommandSequenceNumber: 123, X: 1.0, Y: 2.0, Z: 3.0} // Override with deterministic value so test passes.
					goon.Dump(r2)
				}

				c.logic.playersStateMu.Lock()
				ps := c.logic.playersState[r.PlayerID]
				if r.State != nil {
					ps.NewSeries()
					ps.PushAuthed(c.logic, sequencedPlayerPosVel{
						playerPosVel: playerPosVel{
							X: r.State.X,
							Y: r.State.Y,
							Z: r.State.Z,
						},
						SequenceNumber: r.State.CommandSequenceNumber,
					})
					ps.Health = 100
					if r.PlayerID == c.playerID {
						c.ZOffset = 0
					}
				}
				ps.Team = r.Team
				c.logic.playersState[r.PlayerID] = ps
				c.logic.playersStateMu.Unlock()

				fmt.Printf("%v joined %v.\n", ps.Name, ps.Team)
			case packet.BroadcastTextMessage:
				c.logic.playersStateMu.Lock()
				ps := c.logic.playersState[r.PlayerID]
				c.logic.playersStateMu.Unlock()

				fmt.Printf("%s: %s\n", ps.Name, r.Message)
			case packet.PlayerWasHit:
				c.logic.playersStateMu.Lock()
				ps := c.logic.playersState[r.PlayerID]
				ps.Health += r.HealthGiven
				if ps.Health < 0 {
					ps.Health = 0
				}
				if ps.Health == 0 {
					ps.DeadState = ps.Interpolated(c.logic, r.PlayerID)
				}
				c.logic.playersState[r.PlayerID] = ps
				c.logic.playersStateMu.Unlock()

				fmt.Fprintf(os.Stderr, "[weapons] Player %v was hit for %v.\n", r.PlayerID, r.HealthGiven)
			default:
				fmt.Println("[client] got unsupported TCP packet type")
			}
		}
	}()
}

func (c *client) handleUDP(s *Connection) {
	for {
		r, err := receiveUDPPacket2(s, c.logic.TotalPlayerCount)
		if err != nil {
			panic(err)
		}

		switch r := r.(type) {
		case packet.TimeResponse:
			logicTimeAtReceive := time.Since(c.logic.started).Seconds()

			c.trpReceived++

			if c.trpReceived <= 30 {
				state.Lock()
				latency := logicTimeAtReceive - c.sentTimeRequestPacketTimes[r.SequenceNumber]
				state.Unlock()
				if latency <= c.shortestLatency {
					c.shortestLatency = latency
					c.shortestLatencyLocalTime = logicTimeAtReceive
					c.shortestLatencyRemoteTime = r.Time + 0.5*c.shortestLatency // Remote time now is what server said plus half round-trip time.
				}
			}

			if c.trpReceived == 30 {
				// Adjust our logic clock.
				delta := c.shortestLatencyLocalTime - c.shortestLatencyRemoteTime
				state.Lock()
				c.logic.started = c.logic.started.Add(time.Duration(delta * float64(time.Second)))
				fmt.Fprintf(os.Stderr, "delta: %.3f seconds, started: %v\n", delta, c.logic.started)
				logicTime := time.Since(c.logic.started).Seconds()
				c.logic.GlobalStateSequenceNumber = uint8(logicTime * commandRate) // TODO: Adjust this (https://github.com/shurcooL/eX0/commit/2d3f33ade3765cfc55f6b31c235ab29adad3a8f2).
				c.logic.NextTickTime = 0                                           // TODO: Adjust this.
				for c.logic.NextTickTime+1.0/commandRate < logicTime {
					c.logic.NextTickTime += 1.0 / commandRate
				}
				state.Unlock()

				close(c.finishedSyncingClock)
			}
		case packet.Ping:
			copy(c.lastLatencies, r.LastLatencies)

			{
				var p packet.Pong
				p.PingData = r.PingData

				c.pongSentTimes[r.PingData] = time.Now()

				err := sendUDPPacket(s, &p)
				if err != nil {
					panic(err)
				}
			}
		case packet.Pung:
			localTimeAtPungReceive := time.Now()

			{
				// Get the time sent of the matching Pong packet.
				if localTimeAtPongSend, ok := c.pongSentTimes[r.PingData]; ok {
					delete(c.pongSentTimes, r.PingData)

					// Calculate own latency and update it on the scoreboard.
					latency := localTimeAtPungReceive.Sub(localTimeAtPongSend)
					log.Printf("Latency %.5f ms %v.\n", latency.Seconds()*1000, c.lastLatencies)
				}
			}
		case packet.ServerUpdate:
			// TODO: Verify r.CurrentUpdateSequenceNumber.

			c.logic.playersStateMu.Lock()
			for id, pu := range r.PlayerUpdates {
				if pu.ActivePlayer != 0 {
					ps := c.logic.playersState[uint8(id)]
					ps.PushAuthed(c.logic, sequencedPlayerPosVel{
						playerPosVel: playerPosVel{
							X: pu.State.X,
							Y: pu.State.Y,
							Z: pu.State.Z,
						},
						SequenceNumber: pu.State.CommandSequenceNumber,
					})
					c.logic.playersState[uint8(id)] = ps
				}
			}
			c.logic.playersStateMu.Unlock()

		case packet.WeaponAction:
			goon.DumpExpr(r)
		default:
			fmt.Println("[client] got unsupported UDP packet type")
		}
	}
}
