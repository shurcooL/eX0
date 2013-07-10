// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

PlayerController::PlayerController(CPlayer & oPlayer)
	: m_oPlayer(oPlayer),
	  m_nCommandRequests(0)
{
}

PlayerController::~PlayerController()
{
}

void PlayerController::RequestNextCommand()
{
	++m_nCommandRequests;

	ProvideNextCommand();
}

u_int PlayerController::GetCommandRequests()
{
	return m_nCommandRequests;
}

void PlayerController::UseUpCommandRequest()
{
	eX0_assert(m_nCommandRequests > 0);

	--m_nCommandRequests;
}
/*bool PlayerController::TryUseUpCommandRequest()
{
	if (m_nCommandRequests > 0) {
		--m_nCommandRequests;
		return true;
	} else {
		return false;
	}
}*/

void PlayerController::RequestNextWpnCommand()
{
	ProvideNextWpnCommand();
}

void PlayerController::Reset()
{
	m_nCommandRequests = 0;

	SubReset();
}
