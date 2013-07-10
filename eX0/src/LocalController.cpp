// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

LocalController::LocalController(CPlayer & oPlayer)
	: PlayerController(oPlayer)
{
}

LocalController::~LocalController()
{
}
