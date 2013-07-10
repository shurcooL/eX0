// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
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

void LocalStateAuther::ProcessCommands()
{
	// If the player is dead, ignore and remove any outstanding commands
	if (true == m_oPlayer.IsDead()) {
		while (!m_oPlayer.m_oCommandsQueue.empty())
			m_oPlayer.m_oCommandsQueue.pop();
	}

	while (!m_oPlayer.m_oCommandsQueue.empty() && m_oPlayer.m_pController->GetCommandRequests() > 0)
	{
		SequencedCommand_st oSequencedCommand = m_oPlayer.m_oCommandsQueue.front();
		m_oPlayer.m_oCommandsQueue.pop();

		// TODO: Check if we're missing some states, and fill in the blanks, instead of waiting for 255 more states
		if (oSequencedCommand.cSequenceNumber == m_oPlayer.oLatestAuthStateTEST.cSequenceNumber)
		{
			//printf("%f LSA: used command %d\n", g_pGameSession->LogicTimer().GetGameTime(), oSequencedCommand.cSequenceNumber);
			m_oPlayer.m_pController->UseUpCommandRequest();

			m_oPlayer.oLatestAuthStateTEST = m_oPlayer.PhysicsTickTEST(m_oPlayer.oLatestAuthStateTEST, oSequencedCommand);

			// Add the new state to player's state history
			AuthState_st oAuthState;
			oAuthState.oState = m_oPlayer.oLatestAuthStateTEST;
			oAuthState.bAuthed = true;
			m_oPlayer.PushStateHistory(oAuthState);
		}
		//else printf("%f LSA: popped (ignored) command %d\n", g_pGameSession->LogicTimer().GetGameTime(), oSequencedCommand.cSequenceNumber);
	}
}

void LocalStateAuther::ProcessWpnCommands()
{
	// If the player is dead, ignore and remove any outstanding commands
	if (true == m_oPlayer.IsDead()) {
		while (!m_oPlayer.m_oWpnCommandsQueue.empty())
			m_oPlayer.m_oWpnCommandsQueue.pop();
	}

	while (!m_oPlayer.m_oWpnCommandsQueue.empty())
	{
		/*const*/ WpnCommand_st oWpnCommand = m_oPlayer.m_oWpnCommandsQueue.front();

		if (m_oPlayer.GetSelWeaponTEST().IsCommandOutdated(oWpnCommand, nullptr == dynamic_cast<LocalController *>(m_oPlayer.m_pController)))
		{
			m_oPlayer.m_oWpnCommandsQueue.pop();
			if (nullptr == dynamic_cast<LocalController *>(m_oPlayer.m_pController))	// Not local controller
				printf("LSA: WARNING!!! wpn command %d outdated, ignoring (either client's cheating,\nor I mistakenly ignored his valid input!\n", static_cast<int>(oWpnCommand.nAction));
		}
		else if (m_oPlayer.GetSelWeaponTEST().IsReadyForNextCommand(oWpnCommand))
		{
			m_oPlayer.m_oWpnCommandsQueue.pop();

			//printf("chop at x=%f, y=%f, z=%f\n", oState.fX, oState.fY, oWpnCommand.fZ);
			bool bImportantCommand = m_oPlayer.GetSelWeaponTEST().ProcessWpnCommand(oWpnCommand);
			//printf("received on s oWpnCommand.dTime= %.20f\n", oWpnCommand.dTime);
			//printf("fire command USED by pl#%d\n", m_oPlayer.iID);

			if (bImportantCommand && nullptr != pGameServer) {
				// TEST: Send the Weapon Action packet
				CPacket oWpnActionPacket;
				oWpnActionPacket.pack("cc", (uint8)4, (uint8)m_oPlayer.iID);
				oWpnActionPacket.pack("cd", (uint8)oWpnCommand.nAction, oWpnCommand.dTime);
				if (WeaponSystem::FIRE == oWpnCommand.nAction) oWpnActionPacket.pack("f", oWpnCommand.Parameter.fZ);
				else if (WeaponSystem::CHANGE_WEAPON == oWpnCommand.nAction) oWpnActionPacket.pack("c", oWpnCommand.Parameter.WeaponNumber);
				ClientConnection::BroadcastUdpExcept(oWpnActionPacket, m_oPlayer.pConnection);
			}
		}
		else
		{
			//printf("only future command avail\n");
			break;
		}
	}
}

void LocalStateAuther::ProcessUpdates()
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
		for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
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
