// +build !js

// eX0-go is a work in progress Go implementation of eX0.
//
// The client runs as a native desktop app and in browser.
package main

import (
	"flag"
	"fmt"
	"os"
	"time"
)

var startedProcess = time.Now()

// THINK: Is this the best way?
var components struct {
	logic  *logic
	server *server
	client *client
}

func main() {
	flag.Parse()

	switch args := flag.Args(); {
	case len(args) == 1 && args[0] == "client":
		components.client = startClient()
		time.Sleep(10 * time.Second) // Wait 10 seconds before exiting.
	case len(args) == 1 && args[0] == "server":
		components.logic = startLogic()
		components.server = startServer()
		select {}
	case len(args) == 1 && args[0] == "server-view":
		components.logic = startLogic()
		components.server = startServer()
		view(false)
	case len(args) == 1 && args[0] == "client-view":
		view(true)
	case len(args) == 1 && (args[0] == "client-server-view" || args[0] == "server-client-view"):
		components.server = startServer()
		view(true)
	default:
		fmt.Fprintln(os.Stderr, "invalid usage")
		os.Exit(2)
	}
}
