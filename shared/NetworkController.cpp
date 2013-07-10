// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

NetworkController::NetworkController(CPlayer & oPlayer)
	: PlayerController(oPlayer),
	  m_oMutex(glfwCreateMutex())
{
}

NetworkController::~NetworkController()
{
	glfwDestroyMutex(m_oMutex);
}

bool NetworkController::RequestInput(u_char cSequenceNumber)
{
	return false;
}
