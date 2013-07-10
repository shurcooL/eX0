// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

GameSession * g_pGameSession = NULL;

GameSession::GameSession()
	: GlobalStateSequenceNumberTEST(0),
	  m_oLogicTimer(),
	  m_oRenderTimer()
{
	m_oLogicTimer.StartSyncingTimer(&m_oRenderTimer);
}

GameSession::~GameSession()
{
}

GameTimer & GameSession::LogicTimer()
{
	return m_oLogicTimer;
}

GameTimer & GameSession::RenderTimer()
{
	return m_oRenderTimer;
}
