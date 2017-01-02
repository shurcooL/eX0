// +build js

package main

type server struct {
	// logic is needed to resolve a build error for GOARCH=js:
	//
	// 	main.go:48:31: invalid operation: components.server (variable of type *server) has no field or method logic
	logic *logic
}

func startServer() *server {
	panic("server not implemented on js architecture")
}
