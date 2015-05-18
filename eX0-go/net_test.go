// +build !chan,!tcp

package main

import "time"

// Normal TCP + UDP.
func testFullConnection() {
	go server(true)
	time.Sleep(time.Millisecond) // HACK: Wait for server to start listening.
	client(false)
	time.Sleep(10 * time.Second) // Wait 10 seconds before exiting.
}
