#include "globals.h"

RemoteAuthPlayer::RemoteAuthPlayer()
	: CPlayer()
{
	printf("RemoteAuthPlayer(%p) Ctor.\n", this);
}

RemoteAuthPlayer::RemoteAuthPlayer(u_int nPlayerId)
	: CPlayer(nPlayerId)
{
	printf("RemoteAuthPlayer(%p) Ctor.\n", this);
}

RemoteAuthPlayer::~RemoteAuthPlayer()
{
	printf("RemoteAuthPlayer(%p) ~Dtor.\n", this);
}
