// +build js

package main

import (
	"net/url"
	"os"
	"path"

	"github.com/gopherjs/gopherjs/js"
)

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
