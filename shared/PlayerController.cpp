// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

PlayerController::PlayerController(CPlayer & oPlayer)
	: m_oPlayer(oPlayer)
{
}

PlayerController::~PlayerController()
{
}
