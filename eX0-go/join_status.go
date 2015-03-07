package main

type JoinStatus uint8

const (
	DISCONNECTED JoinStatus = iota
	TCP_CONNECTED
	ACCEPTED
	UDP_CONNECTED
	PUBLIC_CLIENT // This state means that all clients are now aware (or should be aware) of this client, so we'll need to notify them again if he leaves/changes team, etc.
	IN_GAME
)
