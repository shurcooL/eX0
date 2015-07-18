// +build js

package main

import (
	"flag"
	"net/url"
	"os"
	"path"

	"github.com/gopherjs/gopherjs/js"
)

func queryToArgs() []string {
	href := js.Global.Get("location").Get("href").String()
	u, _ := url.Parse(href)
	args := []string{path.Base(u.Path)} // First element is the process name.
	for key, values := range u.Query() {
		key = "-" + key
		for _, value := range values {
			args = append(args, key)
			args = append(args, value)
		}
	}
	return args
}

func init() {
	os.Args = queryToArgs()
}

func main() {
	flag.Parse()

	components.client = startClient()
	view(true)
}
