// eX0-go is a work in progress Go implementation of eX0.
//
// The client runs as a native desktop app and in browser.
package main

import (
	"flag"
	"fmt"
	"os"
	"sync"
)

var state sync.Mutex // TODO: Remove in favor of more specific mutexes.

// components defines pieces of the program that will be executed.
// It must not be modified after startComponents is called, otherwise there'll be races.
var components struct {
	server *server
	client *client
	view   *view
}

func startComponents() {
	// By now, all components have been set, so it's safe to start them.
	// Start server first, client second.
	if components.server != nil {
		components.server.start()
	}
	if components.client != nil {
		if components.server != nil && *hostFlag != "localhost" {
			fmt.Fprintln(os.Stderr, "host must be localhost if both server and client are started")
			flag.Usage()
			os.Exit(2)
		}
		components.client.start()
	}
	if components.view != nil {
		components.view.initAndMainLoop()
	}
}

func usage() {
	fmt.Fprintln(os.Stderr, `Usage: eX0-go [flags] [server]-[client]-[view]

Flags:`)
	flag.PrintDefaults()
	fmt.Fprintln(os.Stderr, "\nExample: eX0-go -host=example.com -name=shaGuar client-view")
}

func main() {
	flag.Usage = usage
	flag.Parse()

	clientNetwork, ok := newClientNetwork()
	if !ok {
		flag.Usage()
		os.Exit(2)
	}

	switch args := flag.Args(); {
	case len(args) == 1 && args[0] == "server":
		components.server = newServer()
		startComponents()
		select {}
	case len(args) == 1 && args[0] == "client":
		components.client = newClient(clientNetwork)
		startComponents()
		select {}
	case len(args) == 1 && (args[0] == "server-client" || args[0] == "client-server"):
		components.server = newServer()
		components.client = newClient(clientNetwork)
		startComponents()
		select {}
	case len(args) == 1 && args[0] == "server-view":
		components.server = newServer()
		components.view = newView(components.server.logic)
		startComponents()
	case len(args) == 1 && args[0] == "client-view":
		components.client = newClient(clientNetwork)
		components.view = newView(components.client.logic)
		startComponents()
	case len(args) == 1 && (args[0] == "server-client-view" || args[0] == "client-server-view"):
		components.server = newServer()
		components.client = newClient(clientNetwork)
		components.view = newView(components.client.logic)
		startComponents()
	default:
		flag.Usage()
		os.Exit(2)
	}
}
