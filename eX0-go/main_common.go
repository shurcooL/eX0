package main

import "time"

var startedProcess = time.Now()

// THINK: Is this the best way?
var components struct {
	logic  *logic
	server *server
	client *client
}
