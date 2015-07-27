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
		if len(b) < packet.TcpHeaderSize {
			panic(fmt.Errorf("sendTcpPacket: smaller than packet.TcpHeaderSize: %v", b))
		}
		var tcpHeader packet.TcpHeader
		err := binary.Read(bytes.NewReader(b[:packet.TcpHeaderSize]), binary.BigEndian, &tcpHeader)
		if err != nil {
			panic(err)
		}
		if len(b)-packet.TcpHeaderSize != int(tcpHeader.Length) {
			panic(fmt.Errorf("sendTcpPacket: invalid payload size: %v %+v", len(b)-packet.TcpHeaderSize, tcpHeader))
		}
	}

	return sendTcpPacket2(c, b)
}
