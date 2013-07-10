// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

NetworkStateAuther::NetworkStateAuther(CPlayer & oPlayer)
	: PlayerStateAuther(oPlayer)
{
	cLastAckedCommandSequenceNumber = 0;
	cCurrentCommandSeriesNumber = 0;
}

NetworkStateAuther::~NetworkStateAuther()
{
}

void NetworkStateAuther::AfterTick()
{
	while (!m_oPlayer.m_oInputCmdsTEST.empty() && m_oPlayer.m_pController->GetCommandRequests() > 0)
	{
		m_oPlayer.m_pController->UseUpCommandRequest();

		SequencedCommand_t oSequencedCommand;
		m_oPlayer.m_oInputCmdsTEST.pop(oSequencedCommand);

		//if (oUnconfirmedMoves.size() < 100)
		//{
			// Set the inputs
			m_oPlayer.MoveDirection(oSequencedCommand.oCommand.cMoveDirection);
			m_oPlayer.SetStealth(oSequencedCommand.oCommand.cStealth != 0);
			m_oPlayer.SetZ(oSequencedCommand.oCommand.fZ);

			// Player tick
			m_oPlayer.CalcTrajs();
			m_oPlayer.CalcColResp();

			/*Input_t oInput;
			oInput.cMoveDirection = (char)nMoveDirection;
			oInput.cStealth = (u_char)iIsStealth;
			oInput.fZ = GetZ();*/
			//if (oUnconfirmedInputs.size() < 101)
			//oUnconfirmedInputs.push_back(oInput);
			Move_t oMove;
			oMove.oCommand = oSequencedCommand.oCommand;
			oMove.oState.fX = m_oPlayer.GetX(); oMove.oState.fY = m_oPlayer.GetY(); oMove.oState.fZ = m_oPlayer.GetZ();
			oUnconfirmedMoves.push(oMove, g_cCurrentCommandSequenceNumber);
			//printf("pushed a move on oUnconfirmedMoves, size() = %d, cur# => %d\n", oUnconfirmedMoves.size(), cCurrentCommandSequenceNumber);

			iTempInt = std::max<int>(iTempInt, (int)oUnconfirmedMoves.size() - 1);

			// Send the Client Command packet
			CPacket oClientCommandPacket;
			oClientCommandPacket.pack("cccc", (u_char)1,		// packet type
											  g_cCurrentCommandSequenceNumber,		// sequence number
											  this->cCurrentCommandSeriesNumber,		// series number
											  (u_char)(oUnconfirmedMoves.size() - 1));
			for (u_char it1 = oUnconfirmedMoves.begin(); it1 != oUnconfirmedMoves.end(); ++it1)
			{
				oClientCommandPacket.pack("ccf", oUnconfirmedMoves[it1].oCommand.cMoveDirection,
												 oUnconfirmedMoves[it1].oCommand.cStealth,
												 oUnconfirmedMoves[it1].oCommand.fZ);
			}
			if ((rand() % 100) >= 0 || iLocalPlayerID != 0) // DEBUG: Simulate packet loss
				pServer->SendUdp(oClientCommandPacket);

			// DEBUG: Keep state history for local player
			/*if ((rand() % 100) >= 0) {
				SequencedState_t oSequencedState;
				oSequencedState.cSequenceNumber = cCurrentCommandSequenceNumber;
				oSequencedState.oState = oMove.oState;
				PushStateHistory(oSequencedState);
			}*/
		//}
	}
}

void NetworkStateAuther::ProcessAuthUpdateTEST()
{
	while (!m_oPlayer.m_oAuthUpdatesTEST.empty()) {
		//eX0_assert(m_oAuthUpdatesTEST.size() == 1, "m_oAuthUpdatesTEST.size() is != 1!!!!\n");

		SequencedState_t oSequencedState;
		m_oPlayer.m_oAuthUpdatesTEST.pop(oSequencedState);
		//printf("popped %d\n", oSequencedState.cSequenceNumber);
		u_int nPlayer = m_oPlayer.iID;
		float fX, fY, fZ;

		{//begin section from Network.cpp
			PlayerGet(nPlayer)->cLatestAuthStateSequenceNumber = oSequencedState.cSequenceNumber;

			fX = oSequencedState.oState.fX;
			fY = oSequencedState.oState.fY;
			fZ = oSequencedState.oState.fZ;

			// Add the new state to player's state history
			PlayerGet(nPlayer)->PushStateHistory(oSequencedState);

			if (nPlayer == iLocalPlayerID)
			{
				if (g_cCurrentCommandSequenceNumber == pLocalPlayer->cLatestAuthStateSequenceNumber)
				{
					Vector2 oServerPosition(fX, fY);
					//Vector2 oClientPrediction(oUnconfirmedMoves.front().oState.fX, oUnconfirmedMoves.front().oState.fY);
					Vector2 oClientPrediction(pLocalPlayer->GetX(), pLocalPlayer->GetY());
					// If the client prediction differs from the server's value by more than a treshold amount, snap to server's value
					if ((oServerPosition - oClientPrediction).SquaredLength() > 0.001f)
						printf("Snapping-A to server's position (%f difference):\n  server = (%.4f, %.4f), client = (%.4f, %.4f)\n", (oServerPosition - oClientPrediction).Length(),
							oServerPosition.x, oServerPosition.y, oClientPrediction.x, oClientPrediction.y);

					// DEBUG: Should make sure the player can't see other players
					// through walls, if he accidentally gets warped through a wall
					pLocalPlayer->SetX(fX);
					pLocalPlayer->SetY(fY);

					// All moves have been confirmed now
					oUnconfirmedMoves.clear();
				}
				else if ((char)(g_cCurrentCommandSequenceNumber - pLocalPlayer->cLatestAuthStateSequenceNumber) > 0)
				{
					string str = (string)"commands empty; " + itos(g_cCurrentCommandSequenceNumber) + ", " + itos(pLocalPlayer->cLatestAuthStateSequenceNumber);
					//eX0_assert(!oLocallyPredictedInputs.empty(), str);
					eX0_assert(!oUnconfirmedMoves.empty(), str);

					// Discard all the locally predicted commands that got deprecated by this server update
					// TODO: There's a faster way to get rid of all old useless packets at once
					while (!oUnconfirmedMoves.empty()) {
						if ((char)(pLocalPlayer->cLatestAuthStateSequenceNumber - oUnconfirmedMoves.begin()) > 0)
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
						pLocalPlayer->SetX(fX);
						pLocalPlayer->SetY(fY);

						eX0_assert((char)(oUnconfirmedMoves.begin() - pLocalPlayer->cLatestAuthStateSequenceNumber) > 0, "outdated command being used");

						// Run the simulation for all locally predicted commands after this server update
						float fOriginalOldX = pLocalPlayer->GetOldX();
						float fOriginalOldY = pLocalPlayer->GetOldY();
						float fOriginalZ = pLocalPlayer->GetZ();
						Command_t oCommand;
						for (u_char it1 = oUnconfirmedMoves.begin(); it1 != oUnconfirmedMoves.end(); ++it1)
						{
							oCommand = oUnconfirmedMoves[it1].oCommand;

							// Set inputs
							pLocalPlayer->MoveDirection(oCommand.cMoveDirection);
							pLocalPlayer->SetStealth(oCommand.cStealth != 0);
							pLocalPlayer->SetZ(oCommand.fZ);

							// Run a tick
							pLocalPlayer->CalcTrajs();
							pLocalPlayer->CalcColResp();
						}
						pLocalPlayer->SetOldX(fOriginalOldX);
						pLocalPlayer->SetOldY(fOriginalOldY);
						pLocalPlayer->SetZ(fOriginalZ);
					}
				} else {
					printf("WTF - server is ahead of client?? confirmed command %d from the future (now %d) lol\n", pLocalPlayer->cLatestAuthStateSequenceNumber, g_cCurrentCommandSequenceNumber);
				}

				// Drop the moves that have been confirmed now
				// TODO: There's a faster way to get rid of all old useless packets at once
				while (!oUnconfirmedMoves.empty())
				{
					if ((char)(pLocalPlayer->cLatestAuthStateSequenceNumber - oUnconfirmedMoves.begin()) >= 0)
					{
						oUnconfirmedMoves.pop();
					} else
						break;
				}
			}
			else if (nPlayer != iLocalPlayerID)
			{
				/*PlayerGet(nPlayer)->Position(fX, fY);
				PlayerGet(nPlayer)->SetZ(fZ);*/

				/*PlayerGet(nPlayer)->SetOldX(PlayerGet(nPlayer)->GetIntX());
				PlayerGet(nPlayer)->SetOldY(PlayerGet(nPlayer)->GetIntY());
				PlayerGet(nPlayer)->SetX(fX);
				PlayerGet(nPlayer)->SetY(fY);
				PlayerGet(nPlayer)->SetZ(fZ);
				PlayerGet(nPlayer)->SetVelX(fVelX);
				PlayerGet(nPlayer)->SetVelY(fVelY);
				PlayerGet(nPlayer)->fTicks = 0.0f;*/

				/*PlayerGet(nPlayer)->SetOldX(PlayerGet(nPlayer)->GetIntX());
				PlayerGet(nPlayer)->SetOldY(PlayerGet(nPlayer)->GetIntY());
				PlayerGet(nPlayer)->SetX(fX + fVelX);
				PlayerGet(nPlayer)->SetY(fY + fVelY);
				PlayerGet(nPlayer)->fOldZ = PlayerGet(nPlayer)->GetZ();
				PlayerGet(nPlayer)->SetZ(fZ);
				PlayerGet(nPlayer)->SetVelX(fVelX);
				PlayerGet(nPlayer)->SetVelY(fVelY);
				PlayerGet(nPlayer)->fTicks = 0.0f;
				PlayerGet(nPlayer)->fUpdateTicks = 0.0f;

				PlayerGet(nPlayer)->CalcColResp();*/
			}
		}//end section from Network.cpp
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

		for (u_int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
		{
			oPacket.unpack("c", &cPlayerInfo);
			if (cPlayerInfo == 1 && PlayerGet(nPlayer) != NULL && PlayerGet(nPlayer)->GetTeam() != 2) {
				// Active player
				oPacket.unpack("c", &cLastCommandSequenceNumber);
				oPacket.unpack("fff", &fX, &fY, &fZ);

				bool bNewerUpdate = (cLastCommandSequenceNumber != static_cast<NetworkStateAuther *>(PlayerGet(nPlayer)->m_pStateAuther)->cLastAckedCommandSequenceNumber);
				if (bNewerUpdate) {
					static_cast<NetworkStateAuther *>(PlayerGet(nPlayer)->m_pStateAuther)->cLastAckedCommandSequenceNumber = cLastCommandSequenceNumber;

					SequencedState_t oSequencedState;
					oSequencedState.cSequenceNumber = cLastCommandSequenceNumber;
					oSequencedState.oState.fX = fX;
					oSequencedState.oState.fY = fY;
					oSequencedState.oState.fZ = fZ;

					eX0_assert(PlayerGet(nPlayer)->m_oAuthUpdatesTEST.push(oSequencedState), "m_oAuthUpdatesTEST.push(oSequencedState) failed, missed latest update!\n");
					//printf("pushed %d\n", cLastCommandSequenceNumber);
				}
			}
		}
	}
}
