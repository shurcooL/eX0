#include "globals.h"

RCtrlRAuthPlayer::RCtrlRAuthPlayer()
	: CPlayer()
{
	printf("RCtrlRAuthPlayer(%p) Ctor.\n", this);
}

RCtrlRAuthPlayer::RCtrlRAuthPlayer(u_int nPlayerId)
	: CPlayer(nPlayerId)
{
	printf("RCtrlRAuthPlayer(%p) Ctor.\n", this);
}

RCtrlRAuthPlayer::~RCtrlRAuthPlayer()
{
	printf("RCtrlRAuthPlayer(%p) ~Dtor.\n", this);
}
