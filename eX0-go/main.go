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

	switch {
	case len(flag.Args()) == 1 && flag.Args()[0] == "client":
		client(nil)
	case len(flag.Args()) == 1 && flag.Args()[0] == "server":
		server(true)
	case len(flag.Args()) == 1 && flag.Args()[0] == "view":
		view()
	case len(flag.Args()) == 1 && flag.Args()[0] == "both":
		go server(false)
		view()
	default:
		fmt.Fprintln(os.Stderr, "invalid usage")
		os.Exit(2)
	}
}
