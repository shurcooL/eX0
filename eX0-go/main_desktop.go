// +build !js

package main

import (
	"flag"
	"go/build"
	"log"
	"os"
)

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

// Set the working directory to the root of eX0-go package, so that its assets can be accessed.
func init() {
	// importPathToDir resolves the absolute path from importPath.
	// There doesn't need to be a valid Go package inside that import path,
	// but the directory must exist.
	importPathToDir := func(importPath string) (string, error) {
		p, err := build.Import(importPath, "", build.FindOnly)
		return p.Dir, err
	}

	dir, err := importPathToDir("github.com/shurcooL/eX0/eX0-go")
	if err != nil {
		log.Fatalln("Unable to find github.com/shurcooL/eX0/eX0-go package in your GOPATH, it's needed to load assets:", err)
	}
	err = os.Chdir(dir)
	if err != nil {
		log.Fatalln("os.Chdir:", err)
	}
}
