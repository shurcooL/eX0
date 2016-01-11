package main

import (
	"bytes"
	"encoding/binary"
	"fmt"

	"github.com/shurcooL/eX0/eX0-go/packet"
)

func sendTCPPacket(c *Connection, b []byte) error {
	// Validate packet size (for debugging).
	if debugValidation {
		if len(b) < packet.TCPHeaderSize {
			panic(fmt.Errorf("sendTCPPacket: smaller than packet.TCPHeaderSize: %v", b))
		}
		var tcpHeader packet.TCPHeader
		err := binary.Read(bytes.NewReader(b[:packet.TCPHeaderSize]), binary.BigEndian, &tcpHeader)
		if err != nil {
			panic(err)
		}
		if len(b)-packet.TCPHeaderSize != int(tcpHeader.Length) {
			panic(fmt.Errorf("sendTCPPacket: invalid payload size: %v %+v", len(b)-packet.TCPHeaderSize, tcpHeader))
		}
	}

	return sendTCPPacketValidated(c, b)
}
