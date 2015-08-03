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
	logic  *logic
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
		components.logic = startLogic()
		components.server = startServer()
		select {}
	case len(args) == 1 && args[0] == "server-view":
		components.logic = startLogic()
		components.server = startServer()
		components.view = runView(false)
	case len(args) == 1 && args[0] == "client-view":
		components.logic = startLogic()
		components.client = startClient()
		components.logic.client <- components.client // TODO: Do this in a nicer way.
		components.view = runView(true)
		components.logic.quit <- struct{}{}
		<-components.logic.quit
	case len(args) == 1 && (args[0] == "client-server-view" || args[0] == "server-client-view"):
		components.logic = startLogic()
		components.server = startServer()
		components.client = startClient()
		components.logic.client <- components.client // TODO: Do this in a nicer way.
		components.view = runView(true)
		components.logic.quit <- struct{}{}
		<-components.logic.quit
	default:
		fmt.Fprintf(os.Stderr, "invalid usage: %q\n", args)
		os.Exit(2)
	}
}
