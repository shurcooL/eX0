package main

import (
	"flag"
	"fmt"
	"os"
)

func main() {
	flag.Parse()

	switch {
	case len(flag.Args()) == 1 && flag.Args()[0] == "client":
		client()
	case len(flag.Args()) == 1 && flag.Args()[0] == "server":
		server()
	case len(flag.Args()) == 1 && flag.Args()[0] == "view":
		view()
	default:
		fmt.Fprintln(os.Stderr, "invalid usage")
		os.Exit(2)
	}
}
