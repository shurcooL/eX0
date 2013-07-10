// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

GameSession * g_pGameSession = NULL;

GameSession::GameSession()
	: m_oGameTimer(),
	  m_oRenderTimer()
{
}

GameSession::~GameSession()
{
}

GameTimer & GameSession::LogicTimer()
{
	return m_oGameTimer;
}

GameTimer & GameSession::RenderTimer()
{
	return m_oRenderTimer;
}
