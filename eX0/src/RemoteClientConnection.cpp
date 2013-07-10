// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

RemoteClientConnection::RemoteClientConnection()
	: ClientConnection()
{
	printf("RemoteClientConnection() Ctor.\n");
}

RemoteClientConnection::~RemoteClientConnection()
{
	printf("RemoteClientConnection() ~Dtor.\n");
}
