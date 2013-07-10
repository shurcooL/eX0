#include "globals.h"

LCtrlRAuthPlayer::LCtrlRAuthPlayer()
	: CPlayer()
{
	printf("LCtrlRAuthPlayer(%p) Ctor.\n", this);
}

LCtrlRAuthPlayer::LCtrlRAuthPlayer(u_int nPlayerId)
	: CPlayer(nPlayerId)
{
	printf("LCtrlRAuthPlayer(%p) Ctor.\n", this);
}

LCtrlRAuthPlayer::~LCtrlRAuthPlayer()
{
	printf("LCtrlRAuthPlayer(%p) ~Dtor.\n", this);
}
