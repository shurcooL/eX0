// +build !chan,!tcp

package main

import "time"

func testFullConnection() {
	// Normal TCP + UDP.
	go server(true)
	time.Sleep(time.Millisecond) // HACK: Wait for server to start listening.
	client(nil)
}
