// +build !js

package main

import "flag"

var networkFlag = flag.String("network", "tcp+udp", `Network for client and server to use (one of "tcp+udp", "tcp-raw", "tcp-ws", "chan").`)

func setupNetwork() bool {
	switch *networkFlag {
	case "tcp+udp":
		nw = tcpUDPNetwork{}
		return true
	case "tcp-raw":
		nw = tcpNetwork{useWebSocket: false}
		return true
	case "tcp-ws":
		nw = tcpNetwork{useWebSocket: true}
		return true
	case "chan":
		nw = chanNetwork{}
		return true
	default:
		return false
	}
}
