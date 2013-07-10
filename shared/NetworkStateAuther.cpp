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
					string str = (string)"inputs empty; " + itos(g_cCurrentCommandSequenceNumber) + ", " + itos(pLocalPlayer->cLatestAuthStateSequenceNumber);
					//eX0_assert(!oLocallyPredictedInputs.empty(), str);
					eX0_assert(!oUnconfirmedMoves.empty(), str);

					// Discard all the locally predicted inputs that got deprecated by this server update
					// TODO: There's a faster way to get rid of all old useless packets at once
					while (!oUnconfirmedMoves.empty()) {
						if ((char)(pLocalPlayer->cLatestAuthStateSequenceNumber - oUnconfirmedMoves.begin()) > 0)
						{
							// This is an outdated predicted input, the server's update supercedes it, thus it's dropped
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

						eX0_assert((char)(oUnconfirmedMoves.begin() - pLocalPlayer->cLatestAuthStateSequenceNumber) > 0, "outdated input being used");

						// Run the simulation for all locally predicted inputs after this server update
						float fOriginalOldX = pLocalPlayer->GetOldX();
						float fOriginalOldY = pLocalPlayer->GetOldY();
						float fOriginalZ = pLocalPlayer->GetZ();
						Input_t oInput;
						for (u_char it1 = oUnconfirmedMoves.begin(); it1 != oUnconfirmedMoves.end(); ++it1)
						{
							oInput = oUnconfirmedMoves[it1].oInput;

							// Set inputs
							pLocalPlayer->MoveDirection(oInput.cMoveDirection);
							pLocalPlayer->SetStealth(oInput.cStealth != 0);
							pLocalPlayer->SetZ(oInput.fZ);

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
