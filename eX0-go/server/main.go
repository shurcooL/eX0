// eX0 server test.
package main

import (
	"bytes"
	"encoding/binary"
	"net"
	"sync"
	"time"

	"github.com/shurcooL/eX0/eX0-go/packet"
	"github.com/shurcooL/go-goon"
)

func main() {
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

	mu          sync.Mutex
	connections map[net.Conn]Connection // TcpConn -> Connection.
}{
	TotalPlayerCount: 16,
	connections:      make(map[net.Conn]Connection),
}

type Connection struct {
	Signature uint64
	UdpAddr   *net.UDPAddr
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
		}
	}
}

func handleConnection(tcp net.Conn) {
	defer tcp.Close()

	{
		var r packet.JoinServerRequest
		err := binary.Read(tcp, binary.BigEndian, &r)
		if err != nil {
			panic(err)
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
					panic(err)
				}

				return
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
			panic(err)
		}
	}

	{
		var r packet.LocalPlayerInfo
		err := binary.Read(tcp, binary.BigEndian, &r.TcpHeader)
		if err != nil {
			panic(err)
		}
		err = binary.Read(tcp, binary.BigEndian, &r.NameLength)
		if err != nil {
			panic(err)
		}
		r.Name = make([]byte, r.NameLength)
		err = binary.Read(tcp, binary.BigEndian, &r.Name)
		if err != nil {
			panic(err)
		}
		err = binary.Read(tcp, binary.BigEndian, &r.CommandRate)
		if err != nil {
			panic(err)
		}
		err = binary.Read(tcp, binary.BigEndian, &r.UpdateRate)
		if err != nil {
			panic(err)
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
			panic(err)
		}
		err = binary.Write(tcp, binary.BigEndian, &p.LevelFilename)
		if err != nil {
			panic(err)
		}
	}

	{
		var p packet.CurrentPlayersInfo
		p.Type = packet.CurrentPlayersInfoType
		p.Players = make([]packet.PlayerInfo, state.TotalPlayerCount)

		p.Length = 16 + uint16(len("shurcooL")) + 1

		err := binary.Write(tcp, binary.BigEndian, &p.TcpHeader)
		if err != nil {
			panic(err)
		}
		for i := range p.Players {
			var playerInfo packet.PlayerInfo

			// TODO: Use actual players state.
			if i == 0 {
				playerInfo.Name = []byte("shurcooL")
				playerInfo.NameLength = uint8(len(playerInfo.Name))
				playerInfo.Team = 2
			}

			err := binary.Write(tcp, binary.BigEndian, &playerInfo.NameLength)
			if err != nil {
				panic(err)
			}

			if playerInfo.NameLength != 0 {
				err = binary.Write(tcp, binary.BigEndian, &playerInfo.Name)
				if err != nil {
					panic(err)
				}
				err = binary.Write(tcp, binary.BigEndian, &playerInfo.Team)
				if err != nil {
					panic(err)
				}

				if playerInfo.Team != 2 {
					err = binary.Write(tcp, binary.BigEndian, playerInfo.State)
					if err != nil {
						panic(err)
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
			panic(err)
		}
	}

	{
		var r packet.EnteredGameNotification
		err := binary.Read(tcp, binary.BigEndian, &r)
		if err != nil {
			panic(err)
		}
		goon.Dump(r)
	}

	select {}
}
