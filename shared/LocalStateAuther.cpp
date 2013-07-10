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
	while (!m_oPlayer.m_oInputCmdsTEST.empty() && m_oPlayer.m_pController->GetCommandRequests() > 0)
	{
		//eX0_assert(m_oInputCmdsTEST.size() == 1, "m_oAuthUpdatesTEST.size() is != 1!!!!\n");

		m_oPlayer.m_pController->UseUpCommandRequest();

		SequencedCommand_t oSequencedCommand;
		m_oPlayer.m_oInputCmdsTEST.pop(oSequencedCommand);
		//static u_char cmdNum = 0; printf("popped a command %d\n", cmdNum++);

		{//begin section from Network.cpp
			// Set the inputs
			m_oPlayer.MoveDirection(oSequencedCommand.oCommand.cMoveDirection);
			m_oPlayer.SetStealth(oSequencedCommand.oCommand.cStealth != 0);
			m_oPlayer.SetZ(oSequencedCommand.oCommand.fZ);

			// Player tick
			m_oPlayer.CalcTrajs();
			m_oPlayer.CalcColResp();

			m_oPlayer.cLatestAuthStateSequenceNumber = oSequencedCommand.cSequenceNumber;
		}//end section from Network.cpp

		// DEBUG: Used for server rendering purposes only, may not be needed later
		{
			SequencedState_t oSequencedState;
			oSequencedState.oState.fX = m_oPlayer.GetX();
			oSequencedState.oState.fY = m_oPlayer.GetY();
			oSequencedState.oState.fZ = m_oPlayer.GetZ();
			oSequencedState.cSequenceNumber = m_oPlayer.cLatestAuthStateSequenceNumber;

			// Add the new state to player's state history
			m_oPlayer.PushStateHistory(oSequencedState);
		}
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

				oServerUpdatePacket.pack("c", PlayerGet(nPlayer)->cLatestAuthStateSequenceNumber);
				oServerUpdatePacket.pack("fff", PlayerGet(nPlayer)->GetX(),
					PlayerGet(nPlayer)->GetY(), PlayerGet(nPlayer)->GetZ());
			} else {
				oServerUpdatePacket.pack("c", (u_char)0);
			}
		}
		if ((rand() % 100) >= 0 || m_oPlayer.pConnection->GetPlayerID() != 0) // DEBUG: Simulate packet loss
			m_oPlayer.pConnection->SendUdp(oServerUpdatePacket);
	}
}
