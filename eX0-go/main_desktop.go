// +build !js

package main

import "flag"

var networkFlag = flag.String("network", "tcp+udp", `Network for client to use (one of "tcp+udp", "tcp-raw", "tcp-ws", "chan").`)

func newClientNetwork() (network, bool) {
	switch *networkFlag {
	case "tcp+udp":
		return tcpUDPNetwork{}, true
	case "tcp-raw":
		return tcpNetwork{useWebSocket: false}, true
	case "tcp-ws":
		return tcpNetwork{useWebSocket: true}, true
	case "chan":
		return chanNetwork{}, true
	default:
		return nil, false
	}
}
