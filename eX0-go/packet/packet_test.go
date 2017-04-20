package packet_test

import (
	"encoding/binary"
	"testing"

	"github.com/shurcooL/eX0/eX0-go/packet"
)

func TestSize(t *testing.T) {
	if packet.TCPHeaderSize != binary.Size(packet.TCPHeader{}) {
		t.Error("packet.TCPHeaderSize != binary.Size(packet.TCPHeader{})")
	}
	if packet.UDPHeaderSize != binary.Size(packet.UDPHeader{}) {
		t.Error("packet.UDPHeaderSize != binary.Size(packet.UDPHeader{})")
	}
}
