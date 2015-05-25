// +build !chan,!tcp

package main

import "time"

// Normal TCP + UDP.
func testFullConnection() {
	components.logic = startLogic()
	components.server = startServer() // Wait for server to start listening.
	client()
	time.Sleep(10 * time.Second) // Wait 10 seconds before exiting.
}
