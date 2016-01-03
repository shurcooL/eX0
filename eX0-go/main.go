// eX0-go is a work in progress Go implementation of eX0.
//
// The client runs as a native desktop app and in browser.
package main

import (
	"flag"
	"fmt"
	"os"
	"sync"
	"time"
)

const debugValidation = true

const commandRate = 20

var state sync.Mutex // TODO: Remove in favor of more specific mutexes.

// THINK: Is this the best way?
var components struct {
	server *server
	client *client
	view   *view
}

func main() {
	flag.Parse()

	switch args := flag.Args(); {
	case len(args) == 1 && args[0] == "client":
		components.client = startClient()
		time.Sleep(10 * time.Second) // Wait 10 seconds before exiting.
	case len(args) == 1 && args[0] == "server":
		components.server = startServer()
		select {}
	case len(args) == 1 && args[0] == "server-view":
		components.server = startServer()
		components.view = startView(components.server.logic)
		components.view.initAndMainLoop()
	case len(args) == 1 && args[0] == "client-view":
		components.client = startClient()
		components.view = startView(components.client.logic)
		components.view.initAndMainLoop()
	case len(args) == 1 && (args[0] == "client-server-view" || args[0] == "server-client-view"):
		components.server = startServer()
		components.client = startClient()
		components.view = startView(components.client.logic)
		components.view.initAndMainLoop()
	default:
		fmt.Fprintf(os.Stderr, "invalid usage: %q\n", args)
		os.Exit(2)
	}
}
