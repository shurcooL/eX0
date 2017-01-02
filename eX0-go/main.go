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

var state sync.Mutex // TODO: Remove in favor of more specific mutexes.

// THINK: Is this the best way?
var components struct {
	server *server
	client *client
	view   *view
}

func usage() {
	fmt.Fprintln(os.Stderr, "Usage: eX0-go [server]-[client]-[view] (e.g., eX0-go client-view)")
	flag.PrintDefaults()
}

func main() {
	flag.Usage = usage
	flag.Parse()

	clientNetwork, ok := newClientNetwork()
	if !ok {
		flag.Usage()
		os.Exit(2)
		return
	}

	switch args := flag.Args(); {
	case len(args) == 1 && args[0] == "client":
		components.client = startClient(clientNetwork)
		time.Sleep(10 * time.Second) // Wait 10 seconds before exiting.
	case len(args) == 1 && args[0] == "server":
		components.server = startServer()
		select {}
	case len(args) == 1 && args[0] == "server-view":
		components.server = startServer()
		components.view = startView(components.server.logic)
		components.view.initAndMainLoop()
	case len(args) == 1 && args[0] == "client-view":
		components.client = startClient(clientNetwork)
		components.view = startView(components.client.logic)
		components.view.initAndMainLoop()
	case len(args) == 1 && (args[0] == "client-server-view" || args[0] == "server-client-view"):
		components.server = startServer()
		components.client = startClient(clientNetwork)
		components.view = startView(components.client.logic)
		components.view.initAndMainLoop()
	default:
		flag.Usage()
		os.Exit(2)
		return
	}
}
