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
	server *server
}

func main() {
	flag.Parse()

	switch args := flag.Args(); {
	case len(args) == 1 && args[0] == "client":
		client()
		time.Sleep(10 * time.Second) // Wait 10 seconds before exiting.
	case len(args) == 1 && args[0] == "server":
		components.server = startServer(true)
		select {}
	case len(args) == 1 && args[0] == "server-view":
		components.server = startServer(true)
		view(false)
	case len(args) == 1 && args[0] == "client-view":
		view(true)
	case len(args) == 1 && (args[0] == "client-server-view" || args[0] == "server-client-view"):
		components.server = startServer(false)
		view(true)
	default:
		fmt.Fprintln(os.Stderr, "invalid usage")
		os.Exit(2)
	}
}
