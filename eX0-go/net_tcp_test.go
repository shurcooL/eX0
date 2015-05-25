// +build tcp,!chan

package main

import "time"

// Virtual TCP and UDP via physical TCP. Requires `go test -tags=tcp`.
func testFullConnection() {
	components.server = startServer(true) // Wait for server to start listening.
	client()
	time.Sleep(10 * time.Second) // Wait 10 seconds before exiting.
}
