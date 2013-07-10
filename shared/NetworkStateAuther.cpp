// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

NetworkStateAuther::NetworkStateAuther(CPlayer & oPlayer)
	: PlayerStateAuther(oPlayer),
	  cCurrentCommandSeriesNumber(0)
{
}

NetworkStateAuther::~NetworkStateAuther()
{
}

void NetworkStateAuther::ProcessCommands()
{
	while (!m_oPlayer.m_oCommandsQueue.empty() && m_oPlayer.m_pController->GetCommandRequests() > 0)
	{
		SequencedCommand_st oSequencedCommand = m_oPlayer.m_oCommandsQueue.front();
		m_oPlayer.m_oCommandsQueue.pop();

		// TODO: Check if we're missing some states, and fill in the blanks, instead of waiting for 255 more states
		if (oSequencedCommand.cSequenceNumber == m_oPlayer.oStateHistory.front().oState.cSequenceNumber)
		{
		//if (oUnconfirmedMoves.size() < 100)
		//{
			m_oPlayer.m_pController->UseUpCommandRequest();

			m_oPlayer.oUnconfirmedMoves.push_back(oSequencedCommand.oCommand, oSequencedCommand.cSequenceNumber);
			//printf("pushed a move on oUnconfirmedMoves, size() = %d, cur# => %d\n", oUnconfirmedMoves.size(), cCurrentCommandSequenceNumber);
			//iTempInt = std::max<int>(iTempInt, (int)m_oPlayer.oUnconfirmedMoves.size() - 1);

			// Predict next state
			AuthState_st oAuthState;
			oAuthState.oState = m_oPlayer.PhysicsTickTEST(m_oPlayer.oStateHistory.front().oState, oSequencedCommand);
			oAuthState.bAuthed = false;

			m_oPlayer.PushStateHistory(oAuthState);

			// Send the Client Command packet
			CPacket oClientCommandPacket;
			oClientCommandPacket.pack("cccc", (u_char)1,		// packet type
											  m_oPlayer.oUnconfirmedMoves.end() - 1,		// latest command sequence number
											  this->cCurrentCommandSeriesNumber,		// series number
											  (u_char)(m_oPlayer.oUnconfirmedMoves.size() - 1));		// normalize to 0 (since 1 is the lowest possible number)
			for (IndexedCircularBuffer<Command_st, u_char>::iterator it1 = m_oPlayer.oUnconfirmedMoves.begin(); it1 != m_oPlayer.oUnconfirmedMoves.end(); ++it1)
			{
				oClientCommandPacket.pack("cbf", m_oPlayer.oUnconfirmedMoves[it1].cMoveDirection,
												 m_oPlayer.oUnconfirmedMoves[it1].bStealth,
												 m_oPlayer.oUnconfirmedMoves[it1].fZ);
			}
			if ((rand() % 100) >= 0 || pLocalPlayer->iID != 0) // DEBUG: Simulate packet loss
				pServer->SendUdp(oClientCommandPacket);
		//}
		}
	}
}

void NetworkStateAuther::ProcessWpnCommands()
{
	while (!m_oPlayer.m_oWpnCommandsQueue.empty())
	{
		/*const*/ WpnCommand_st oWpnCommand = m_oPlayer.m_oWpnCommandsQueue.front();

		if (m_oPlayer.GetSelWeaponTEST().IsCommandOutdated(oWpnCommand, nullptr == dynamic_cast<LocalController *>(m_oPlayer.m_pController)))
		{
			m_oPlayer.m_oWpnCommandsQueue.pop();
			if (nullptr == dynamic_cast<LocalController *>(m_oPlayer.m_pController))	// Not local controller
				printf("NSA: WARNING!!! wpn command %d outdated, ignoring (either client's cheating,\nor I mistakenly ignored his valid input!\n", static_cast<int>(oWpnCommand.nAction));
		}
		else if (m_oPlayer.GetSelWeaponTEST().IsReadyForNextCommand(oWpnCommand))
		{
			m_oPlayer.m_oWpnCommandsQueue.pop();

			//printf("chop at x=%f, y=%f, z=%f\n", oState.fX, oState.fY, oWpnCommand.fZ);
			bool bImportantCommand = m_oPlayer.GetSelWeaponTEST().ProcessWpnCommand(oWpnCommand);
			//printf("sending c->s oWpnCommand.dTime= %.20f\n", oWpnCommand.dTime);

			// DEBUG: Is this the right if() statement condition here? Basically it's to prevent us from sending packets that we made a shot, which we didn't (but rather received from server for another player)
			//if (nullptr != m_oPlayer.m_pController) {
			if (bImportantCommand && nullptr != dynamic_cast<LocalController *>(m_oPlayer.m_pController)) {
				// TEST: Send a Weapon Command packet
				CPacket oWpnCommandPacket;
				oWpnCommandPacket.pack("c", (uint8)3);
				oWpnCommandPacket.pack("cd", (uint8)oWpnCommand.nAction, oWpnCommand.dTime);
																		 //oWpnCommand.dTime - (((rand() % 100) < 10) ? 0.0000001 : 0));
																		 //oWpnCommand.dTime - (((rand() % 100) < 10) ? 0.0500001 : 0));
				if (WeaponSystem::FIRE == oWpnCommand.nAction) oWpnCommandPacket.pack("f", oWpnCommand.Parameter.fZ);
				else if (WeaponSystem::CHANGE_WEAPON == oWpnCommand.nAction) oWpnCommandPacket.pack("c", oWpnCommand.Parameter.WeaponNumber);
				if ((rand() % 100) >= 0 || pLocalPlayer->iID != 0) // DEBUG: Simulate packet loss
					pServer->SendUdp(oWpnCommandPacket);
			}
		}
		else
		{
			break;
		}
	}
}

void NetworkStateAuther::ProcessUpdates()
{
	while (!m_oPlayer.m_oUpdatesQueue.empty()) {
		//eX0_assert(m_oUpdatesQueue.size() == 1, "m_oUpdatesQueue.size() is != 1!!!!\n");

		SequencedState_st oSequencedState = m_oPlayer.m_oUpdatesQueue.front();
		m_oPlayer.m_oUpdatesQueue.pop();
		//printf("popped %d\n", oSequencedState.cSequenceNumber);

		bool bNewerUpdate = (oSequencedState.cSequenceNumber != m_oPlayer.oLatestAuthStateTEST.cSequenceNumber);
		if (bNewerUpdate)
		{
			m_oPlayer.oLatestAuthStateTEST = oSequencedState;

			// Add the new state to player's state history
			AuthState_st oAuthState;
			oAuthState.oState = oSequencedState;
			oAuthState.bAuthed = true;
			m_oPlayer.PushStateHistory(oAuthState);
		}
		/* Outdated, moved logic inside PushStateHistory()
		{//begin section from Network.cpp
			m_oPlayer.oLatestAuthStateTEST = oSequencedState;

			float fX, fY, fZ;
			fX = oSequencedState.oState.fX;
			fY = oSequencedState.oState.fY;
			fZ = oSequencedState.oState.fZ;

			// Add the new state to player's state history
			m_oPlayer.PushStateHistory(oSequencedState);

			if (&m_oPlayer == pLocalPlayer)
			{
				if (pLocalPlayer->GlobalStateSequenceNumberTEST == pLocalPlayer->oLatestAuthStateTEST.cSequenceNumber)
				{
					Vector2 oServerPosition(fX, fY);
					Vector2 oClientPrediction(pLocalPlayer->oStateHistory.front().oState.oState.fX, pLocalPlayer->oStateHistory.front().oState.oState.fY);
					// If the client prediction differs from the server's value by more than a treshold amount, snap to server's value
					if ((oServerPosition - oClientPrediction).SquaredLength() > 0.001f)
						printf("Snapping-A to server's position (%f difference):\n  server = (%.4f, %.4f), client = (%.4f, %.4f)\n", (oServerPosition - oClientPrediction).Length(),
							oServerPosition.x, oServerPosition.y, oClientPrediction.x, oClientPrediction.y);

					// DEBUG: Should make sure the player can't see other players
					// through walls, if he accidentally gets warped through a wall
					pLocalPlayer->oLatestPredStateTEST.oState.fX = fX;
					pLocalPlayer->oLatestPredStateTEST.oState.fY = fY;

					// All moves have been confirmed now
					oUnconfirmedMoves.clear();
				}
				else if ((char)(pLocalPlayer->GlobalStateSequenceNumberTEST - pLocalPlayer->oLatestAuthStateTEST.cSequenceNumber) > 0)
				{
					string str = (string)"commands empty; " + itos(g_cCurrentCommandSequenceNumber) + ", " + itos(pLocalPlayer->oLatestAuthStateTEST.cSequenceNumber);
					//eX0_assert(!oLocallyPredictedInputs.empty(), str);
					eX0_assert(!oUnconfirmedMoves.empty(), str);

					// Discard all the locally predicted commands that got deprecated by this server update
					// TODO: There's a faster way to get rid of all old useless packets at once
					while (!oUnconfirmedMoves.empty()) {
						if ((char)(pLocalPlayer->oLatestAuthStateTEST.cSequenceNumber - oUnconfirmedMoves.begin()) > 0)
						{
							// This is an outdated predicted command, the server's update supercedes it, thus it's dropped
							oUnconfirmedMoves.pop();
						} else
							break;
					}

					Vector2 oServerPosition(fX, fY);
					Vector2 oClientPrediction(oUnconfirmedMoves.front().oState.fX, oUnconfirmedMoves.front().oState.fY);
					// If the client prediction differs from the server's value by more than a treshold amount, snap to server's value
					if ((oServerPosition - oClientPrediction).SquaredLength() > 0.001f)
					{
						printf("Snapping-B to server's position (%f difference).\n", (oServerPosition - oClientPrediction).Length());

						// DEBUG: Figure out why I have this here, I'm not sure if it's correct or its purpose
						oUnconfirmedMoves.pop();

						// DEBUG: Should make sure the player can't see other players
						// through walls, if he accidents gets warped through a wall
						pLocalPlayer->oLatestPredStateTEST.oState.fX = fX;
						pLocalPlayer->oLatestPredStateTEST.oState.fY = fY;

						eX0_assert((char)(oUnconfirmedMoves.begin() - pLocalPlayer->oLatestAuthStateTEST.cSequenceNumber) > 0, "outdated command being used");

						// Run the simulation for all locally predicted commands after this server update
						/*float fOriginalOldX = pLocalPlayer->GetOldX();
						float fOriginalOldY = pLocalPlayer->GetOldY();
						float fOriginalZ = pLocalPlayer->GetZ();* /
						Command_st oCommand;
						for (u_char it1 = oUnconfirmedMoves.begin(); it1 != oUnconfirmedMoves.end(); ++it1)
						{
							oCommand = oUnconfirmedMoves[it1].oCommand;

							// Set inputs
							/*pLocalPlayer->MoveDirection(oCommand.cMoveDirection);
							pLocalPlayer->SetStealth(oCommand.bStealth);
							pLocalPlayer->SetZ(oCommand.fZ);

							// Run a tick
							pLocalPlayer->CalcTrajs();
							pLocalPlayer->CalcColResp();* /
							SequencedCommand_st oSeqCommand;
							oSeqCommand.oCommand = oCommand;
							oSeqCommand.cSequenceNumber = -123;//DEBUG: Fix
							pLocalPlayer->oLatestPredStateTEST = pLocalPlayer->PhysicsTickTEST(pLocalPlayer->oLatestPredStateTEST, oSeqCommand);
						}
						/*pLocalPlayer->SetOldX(fOriginalOldX);
						pLocalPlayer->SetOldY(fOriginalOldY);
						pLocalPlayer->SetZ(fOriginalZ);* /
					}
				} else {
					printf("WTF - server is ahead of client?? confirmed command %d from the future (now %d) lol\n", pLocalPlayer->oLatestAuthStateTEST.cSequenceNumber, g_cCurrentCommandSequenceNumber);
				}

				// Drop the moves that have been confirmed now
				// TODO: There's a faster way to get rid of all old useless packets at once
				while (!oUnconfirmedMoves.empty())
				{
					if ((char)(pLocalPlayer->oLatestAuthStateTEST.cSequenceNumber - oUnconfirmedMoves.begin()) >= 0)
					{
						oUnconfirmedMoves.pop();
					} else
						break;
				}
			}
		}*///end section from Network.cpp
	}
}

void NetworkStateAuther::SendUpdate()
{
}

void NetworkStateAuther::ProcessUpdate(CPacket & oPacket)
{
	u_char cUpdateSequenceNumber;
	u_char cLastCommandSequenceNumber;
	u_char cPlayerInfo;
	float fX, fY, fZ;
	oPacket.unpack("c", &cUpdateSequenceNumber);

	if (cUpdateSequenceNumber == pServer->cLastUpdateSequenceNumber) {
		printf("Got a duplicate UDP update packet from the server, discarding.\n");
	} else if ((char)(cUpdateSequenceNumber - pServer->cLastUpdateSequenceNumber) < 0) {
		printf("Got an out of order UDP update packet from the server, discarding.\n");
	} else
	{
		++pServer->cLastUpdateSequenceNumber;
		if (cUpdateSequenceNumber != pServer->cLastUpdateSequenceNumber) {
			printf("Lost %d UDP update packet(s) from the server!\n", (char)(cUpdateSequenceNumber - pServer->cLastUpdateSequenceNumber));
		}
		pServer->cLastUpdateSequenceNumber = cUpdateSequenceNumber;

		for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
		{
			oPacket.unpack("c", &cPlayerInfo);
			if (cPlayerInfo == 1 && PlayerGet(nPlayer) != NULL && PlayerGet(nPlayer)->GetTeam() != 2) {
				// Active player
				oPacket.unpack("c", &cLastCommandSequenceNumber);
				oPacket.unpack("fff", &fX, &fY, &fZ);

				/*bool bNewerUpdate = (cLastCommandSequenceNumber != static_cast<NetworkStateAuther *>(PlayerGet(nPlayer)->m_pStateAuther)->cLastAckedCommandSequenceNumber);
				if (bNewerUpdate) {
					static_cast<NetworkStateAuther *>(PlayerGet(nPlayer)->m_pStateAuther)->cLastAckedCommandSequenceNumber = cLastCommandSequenceNumber;*/

				SequencedState_st oSequencedState;
				oSequencedState.cSequenceNumber = cLastCommandSequenceNumber;
				oSequencedState.oState.fX = fX;
				oSequencedState.oState.fY = fY;
				oSequencedState.oState.fZ = fZ;

				eX0_assert(PlayerGet(nPlayer)->m_oUpdatesQueue.push(oSequencedState), "m_oUpdatesQueue.push(oSequencedState) failed, missed latest update!\n");
				//printf("pushed %d\n", cLastCommandSequenceNumber);
			}
		}
	}
}
