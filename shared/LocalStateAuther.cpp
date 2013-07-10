// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

LocalStateAuther::LocalStateAuther(CPlayer & oPlayer)
	: PlayerStateAuther(oPlayer)
{
}

LocalStateAuther::~LocalStateAuther()
{
}

void LocalStateAuther::AfterTick()
{
	while (!m_oPlayer.m_oCommandsQueue.empty() && m_oPlayer.m_pController->GetCommandRequests() > 0)
	{
		SequencedCommand_t oSequencedCommand;
		m_oPlayer.m_oCommandsQueue.pop(oSequencedCommand);

		// TODO: Check if we're missing some states, and fill in the blanks, instead of waiting for 255 more states
		if (oSequencedCommand.cSequenceNumber == m_oPlayer.oLatestAuthStateTEST.cSequenceNumber)
		{
			//printf("%f LSA: used command %d\n", g_pGameSession->LogicTimer().GetGameTime(), oSequencedCommand.cSequenceNumber);
			m_oPlayer.m_pController->UseUpCommandRequest();

			m_oPlayer.oLatestAuthStateTEST = m_oPlayer.PhysicsTickTEST(m_oPlayer.oLatestAuthStateTEST, oSequencedCommand);

			// Add the new state to player's state history
			AuthState_t oAuthState;
			oAuthState.oState = m_oPlayer.oLatestAuthStateTEST;
			oAuthState.bAuthed = true;
			m_oPlayer.PushStateHistory(oAuthState);
		}
		//else printf("%f LSA: popped (ignored) command %d\n", g_pGameSession->LogicTimer().GetGameTime(), oSequencedCommand.cSequenceNumber);
	}
}

void LocalStateAuther::ProcessAuthUpdateTEST()
{
}

void LocalStateAuther::SendUpdate()
{
	if (m_oPlayer.pConnection == NULL || m_oPlayer.pConnection->GetJoinStatus() < IN_GAME)
		return;

	while (g_pGameSession->LogicTimer().GetTime() >= m_oPlayer.m_dNextUpdateTime)
	{
		m_oPlayer.m_dNextUpdateTime += 1.0 / g_cUpdateRate;

		++m_oPlayer.pConnection->cCurrentUpdateSequenceNumber;		// First update is sent with cCurrentUpdateSequenceNumber == 1

		// Send the Update Others Position packet
		CPacket oServerUpdatePacket;
		oServerUpdatePacket.pack("cc", (u_char)2, m_oPlayer.pConnection->cCurrentUpdateSequenceNumber);
		for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
		{
			if (PlayerGet(nPlayer) != NULL && (PlayerGet(nPlayer)->pConnection == NULL || PlayerGet(nPlayer)->pConnection->GetJoinStatus() == IN_GAME)
				&& PlayerGet(nPlayer)->GetTeam() != 2)
			{
				oServerUpdatePacket.pack("c", (u_char)1);

				oServerUpdatePacket.pack("c", PlayerGet(nPlayer)->oLatestAuthStateTEST.cSequenceNumber);
				oServerUpdatePacket.pack("fff", PlayerGet(nPlayer)->oLatestAuthStateTEST.oState.fX,
					PlayerGet(nPlayer)->oLatestAuthStateTEST.oState.fY, PlayerGet(nPlayer)->oLatestAuthStateTEST.oState.fZ);
			} else {
				oServerUpdatePacket.pack("c", (u_char)0);
			}
		}
		if ((rand() % 100) >= 0 || m_oPlayer.pConnection->GetPlayerID() != 0) // DEBUG: Simulate packet loss
			m_oPlayer.pConnection->SendUdp(oServerUpdatePacket);
	}
}
