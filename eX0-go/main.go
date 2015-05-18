package main

import (
	"flag"
	"fmt"
	"os"
	"time"
)

var startedProcess = time.Now()

func main() {
	flag.Parse()

	switch args := flag.Args(); {
	case len(args) == 1 && args[0] == "client":
		client(false)
		time.Sleep(10 * time.Second) // Wait 10 seconds before exiting.
	case len(args) == 1 && args[0] == "server":
		server(true)
	case len(args) == 1 && args[0] == "server-view":
		go server(true)
		view(false)
	case len(args) == 1 && args[0] == "client-view":
		view(true)
	case len(args) == 1 && (args[0] == "client-server-view" || args[0] == "server-client-view"):
		go server(false)
		view(true)
	default:
		fmt.Fprintln(os.Stderr, "invalid usage")
		os.Exit(2)
	}
}
