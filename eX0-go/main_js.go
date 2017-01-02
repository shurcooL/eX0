// +build js

package main

import (
	"flag"
	"net/url"
	"os"
	"path"

	"github.com/gopherjs/gopherjs/js"
)

var networkFlag = flag.String("network", "tcp-ws", `Network for client to use (only choice is "tcp-ws").`)

func newClientNetwork() (network, bool) {
	switch *networkFlag {
	case "tcp-ws":
		return tcpNetwork{useWebSocket: true}, true
	default:
		return nil, false
	}
}

func queryToArgs() []string {
	u, err := url.Parse(js.Global.Get("location").Get("href").String())
	if err != nil {
		panic(err)
	}
	args := []string{path.Base(u.Path)} // First element is the process name.
	for k, vs := range u.Query() {
		for _, v := range vs {
			args = append(args, k)
			if v != "" {
				args = append(args, v)
			}
		}
	}
	return args
}

func init() {
	os.Args = queryToArgs()
	if len(os.Args) == 1 {
		os.Args = append(os.Args, "client-view") // Default mode when no parameters are provided.
	}
}
