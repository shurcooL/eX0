package main

import (
	"bytes"
	"encoding/binary"
	"fmt"

	"github.com/shurcooL/eX0/eX0-go/packet"
)

func sendTcpPacket(c *Connection, b []byte) error {
	// Validate packet size (for debugging).
	if debugValidation {
		const tcpHeaderSize = 3 // TODO: This shouldn't be hardcoded here.
		if len(b) < tcpHeaderSize {
			panic(fmt.Errorf("sendTcpPacket: smaller than tcpHeaderSize: %v", b))
		}
		var tcpHeader packet.TcpHeader
		err := binary.Read(bytes.NewReader(b[:tcpHeaderSize]), binary.BigEndian, &tcpHeader)
		if err != nil {
			panic(err)
		}
		if len(b) != tcpHeaderSize+int(tcpHeader.Length) {
			panic(fmt.Errorf("sendTcpPacket: invalid size: %v %+v", len(b), tcpHeader))
		}
	}

	return sendTcpPacket2(c, b)
}
