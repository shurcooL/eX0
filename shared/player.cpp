// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

u_int	nPlayerCount = 0;
//CPlayer	*oPlayers[32] = { NULL };
#ifdef EX0_CLIENT
u_int		iLocalPlayerID = 0;
CPlayer *	pLocalPlayer = NULL;
string		sLocalPlayerName = "New Player";
#endif

vector<CPlayer *> CPlayer::m_oPlayers;

//float	fPlayerTickTime;// = 0.025f;
//float	fPlayerTickTime = 0.050f;
//float	fPlayerTickTime = 0.100f;
//float	fPlayerTickTime = 1.0f;

// implementation of the player class
CPlayer::CPlayer()
{
	printf("CPlayer(%p) Ctor.\n", this);

	// init vars
	fX = 0;
	fY = 0;
	fOldX = 0;
	fOldY = 0;
	fVelX = 0;
	fVelY = 0;
	fIntX = 0;
	fIntY = 0;
	fZ = 0;
	fOldZ = 0;
	iIsStealth = 0;
	nMoveDirection = -1;
	iSelWeapon = 2;
	fAimingDistance = 200.0;
	fHealth = 100;
	sName = "Unnamed Player";
	m_nTeam = 0;
	bEmptyClicked = true;
	fTicks = 0;
	fTickTime = 0;

	// Network related
#ifdef EX0_CLIENT
	m_nLastLatency = 0;
	cCurrentCommandSeriesNumber = 0;
#else
	pConnection = NULL;
#endif

	Add(this);

	InitWeapons();
}

CPlayer::CPlayer(u_int nPlayerId)
{
	printf("CPlayer(%p) Ctor.\n", this);

	// init vars
	fX = 0;
	fY = 0;
	fOldX = 0;
	fOldY = 0;
	fVelX = 0;
	fVelY = 0;
	fIntX = 0;
	fIntY = 0;
	fZ = 0;
	fOldZ = 0;
	iIsStealth = 0;
	nMoveDirection = -1;
	iSelWeapon = 2;
	fAimingDistance = 200.0;
	fHealth = 100;
	sName = "Unnamed Player";
	m_nTeam = 0;
	bEmptyClicked = true;
	fTicks = 0;
	fTickTime = 0;

	// Network related
#ifdef EX0_CLIENT
	m_nLastLatency = 0;
	cCurrentCommandSeriesNumber = 0;
#else
	pConnection = NULL;
#endif

	Add(this, nPlayerId);

	InitWeapons();
}

CPlayer::~CPlayer()
{
	Remove(this);

	printf("CPlayer(%p) ~Dtor.\n", this);
}

#ifdef EX0_CLIENT
void CPlayer::FakeTick()
{
	fTicks = (float)(dCurTime - (dNextTickTime - 1.0 / cCommandRate));
	while (dCurTime >= dNextTickTime)
	{
		dNextTickTime += 1.0 / cCommandRate;
		fTicks = (float)(dCurTime - (dNextTickTime - 1.0 / cCommandRate));

		++cCurrentCommandSequenceNumber;
	}
}
#endif

void CPlayer::Tick()
{
#ifdef EX0_CLIENT
	// is the player dead?
	if (IsDead()) {
		return;
	}

	oWeapons[iSelWeapon].Tick();

	//fTicks += (double)dTimePassed;
	fTicks = (float)(dCurTime - (dNextTickTime - 1.0 / cCommandRate));
	//while (fTicks >= fTickTime)
	while (dCurTime >= dNextTickTime)
	{
		//fTicks -= fTickTime;
		dNextTickTime += 1.0 / cCommandRate;
		/*if ((cCurrentCommandSequenceNumber + 1) == 255) {	// cCurrentCommandSequenceNumber will be incremented by 1 this tick
			//double d = glfwGetTime() / (256.0 / cCommandRate);
			printf("%.8lf sec: NxtTk=%.15lf, NxtTk/12.8=%.15lf\n", glfwGetTime(), dNextTickTime, dNextTickTime / (256.0 / cCommandRate));
		}*/
		fTicks = (float)(dCurTime - (dNextTickTime - 1.0 / cCommandRate));

		if (iID == iLocalPlayerID) {
			if (oUnconfirmedMoves.size() < 100)
			{
				// calculate player trajectory
				CalcTrajs();

				// do collision response for player
				CalcColResp();

				++cCurrentCommandSequenceNumber;
				Input_t oInput;
				oInput.cMoveDirection = (char)nMoveDirection;
				oInput.cStealth = (u_char)iIsStealth;
				oInput.fZ = GetZ();
				//if (oUnconfirmedInputs.size() < 101)
				//oUnconfirmedInputs.push_back(oInput);
				Move_t oMove;
				oMove.oInput = oInput;
				oMove.oState.fX = GetX(); oMove.oState.fY = GetY(); oMove.oState.fZ = GetZ();
				oUnconfirmedMoves.push(oMove, cCurrentCommandSequenceNumber);
//printf("pushed a move on oUnconfirmedMoves, size() = %d, cur# => %d\n", oUnconfirmedMoves.size(), cCurrentCommandSequenceNumber);

				iTempInt = max<int>(iTempInt, (int)oUnconfirmedMoves.size() - 1);

				// Send the Client Command packet
				CPacket oClientCommandPacket;
				oClientCommandPacket.pack("cccc", (u_char)1,		// packet type
												 cCurrentCommandSequenceNumber,		// sequence number
												 cCurrentCommandSeriesNumber,		// series number
												 (u_char)(oUnconfirmedMoves.size() - 1));
				for (u_char it1 = oUnconfirmedMoves.begin(); it1 != oUnconfirmedMoves.end(); ++it1)
				{
					oClientCommandPacket.pack("ccf", oUnconfirmedMoves[it1].oInput.cMoveDirection,
													 oUnconfirmedMoves[it1].oInput.cStealth,
													 oUnconfirmedMoves[it1].oInput.fZ);
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
			}
		} else {
			// calculate player trajectory
			//CalcTrajs();
			/*fOldX = fX;
			fOldY = fY;
			fX += fVelX;
			fY += fVelY;

			// do collision response for player
			CalcColResp();*/
			//++cCurrentCommandSequenceNumber;
		}
	}

	UpdateInterpolatedPos();
#endif
}

void CPlayer::ProcessAuthUpdateTEST()
{
#ifdef EX0_CLIENT
	while (!m_oAuthUpdatesTEST.empty()) {
		//eX0_assert(m_oAuthUpdatesTEST.size() == 1, "m_oAuthUpdatesTEST.size() is != 1!!!!\n");

		SequencedState_t oSequencedState;
		m_oAuthUpdatesTEST.pop(oSequencedState);
		//printf("popped %d\n", oSequencedState.cSequenceNumber);
		u_int nPlayer = this->iID;
		float fX, fY, fZ;

		{//begin section from Network.cpp
			//PlayerGet(nPlayer)->cLastAckedCommandSequenceNumber = cLastCommandSequenceNumber;
			PlayerGet(nPlayer)->cLastAckedCommandSequenceNumber = oSequencedState.cSequenceNumber;

			fX = oSequencedState.oState.fX;
			fY = oSequencedState.oState.fY;
			fZ = oSequencedState.oState.fZ;

			// Add the new state to player's state history
			/*SequencedState_t oSequencedState;
			oSequencedState.cSequenceNumber = PlayerGet(nPlayer)->cLastAckedCommandSequenceNumber;
			oSequencedState.oState.fX = fX;
			oSequencedState.oState.fY = fY;
			oSequencedState.oState.fZ = fZ;*/
			PlayerGet(nPlayer)->PushStateHistory(oSequencedState);

			if (nPlayer == iLocalPlayerID)
			{
				if (cCurrentCommandSequenceNumber == pLocalPlayer->cLastAckedCommandSequenceNumber)
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
				else if ((char)(cCurrentCommandSequenceNumber - pLocalPlayer->cLastAckedCommandSequenceNumber) > 0)
				{
					string str = (string)"inputs empty; " + itos(cCurrentCommandSequenceNumber) + ", " + itos(pLocalPlayer->cLastAckedCommandSequenceNumber);
					//eX0_assert(!oLocallyPredictedInputs.empty(), str);
					eX0_assert(!oUnconfirmedMoves.empty(), str);

					// Discard all the locally predicted inputs that got deprecated by this server update
					// TODO: There's a faster way to get rid of all old useless packets at once
					while (!oUnconfirmedMoves.empty()) {
						if ((char)(pLocalPlayer->cLastAckedCommandSequenceNumber - oUnconfirmedMoves.begin()) > 0)
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

						eX0_assert((char)(oUnconfirmedMoves.begin() - pLocalPlayer->cLastAckedCommandSequenceNumber) > 0, "outdated input being used");

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
					printf("WTF - server is ahead of client?? confirmed command %d from the future (now %d) lol\n", pLocalPlayer->cLastAckedCommandSequenceNumber, cCurrentCommandSequenceNumber);
				}

				// Drop the moves that have been confirmed now
				// TODO: There's a faster way to get rid of all old useless packets at once
				while (!oUnconfirmedMoves.empty())
				{
					if ((char)(pLocalPlayer->cLastAckedCommandSequenceNumber - oUnconfirmedMoves.begin()) >= 0)
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
#endif // EX0_CLIENT
}

// returns number of clips left in the selected weapon
int CPlayer::GetSelClips()
{
	return oWeapons[iSelWeapon].GetClips();
}
int CPlayer::GetSelClipAmmo()
{
	return oWeapons[iSelWeapon].GetClipAmmo();
}

// fire selected weapon
void CPlayer::Fire()
{
	// is the player dead?
	if (IsDead()) return;

	// fire the selected weapon
	oWeapons[iSelWeapon].Fire();
}

// reload selected weapon
void CPlayer::Reload()
{
	// is the player dead?
	if (IsDead()) return;

	oWeapons[iSelWeapon].StartReloading();
}

bool CPlayer::IsReloading()
{
	return oWeapons[iSelWeapon].IsReloading();
}

// buy a clip for selected weapon
void CPlayer::BuyClip()
{
	// is the player dead?
	if (IsDead()) return;

	// TODO: some kind of money system?
	// just give it to whoever
	oWeapons[iSelWeapon].GiveClip();
}

void CPlayer::SetTeam(int nTeam)
{
	m_nTeam = nTeam;
}

int CPlayer::GetTeam()
{
	return m_nTeam;
}

void CPlayer::SetStealth(bool bOn)
{
	// is the player dead?
	if (IsDead()) return;

	iIsStealth = (int)bOn;
}

#ifdef EX0_CLIENT
// A partial reset of the player state that happens when the player respawns (or changes team, etc.)
void CPlayer::RespawnReset()
{
	fX = 0;
	fY = 0;
	fOldX = 0;
	fOldY = 0;
	fVelX = 0;
	fVelY = 0;
	fIntX = 0;
	fIntY = 0;
	fZ = 0;
	fOldZ = 0;
	//iIsStealth = 0;
	nMoveDirection = -1;
	iSelWeapon = 2;
	fHealth = 100.0f;

	// Increment the Command packet series
	cCurrentCommandSeriesNumber += 1;
}
#endif

bool CPlayer::IsDead()
{
	return (fHealth <= 0);
}

float CPlayer::GetHealth()
{
	return fHealth;
}

void CPlayer::GiveHealth(float fValue)
{
	// is the player dead?
	if (IsDead()) return;

	fHealth += fValue;
	if (fHealth < 0) fHealth = 0;
}

void CPlayer::CalcTrajs()
{
	// is the player dead?
	if (IsDead()) return;

#if 0
	// Update the player velocity (acceleration)
	if (nMoveDirection == -1)
	{
		fVelX = 0.0;
		fVelY = 0.0;
	}
	else
	{
		fVelX = (fTickTime / 0.050f) * Math::Sin((float)nMoveDirection * 0.785398f + fZ) * (3.5f - iIsStealth * 2.5f);
		fVelY = (fTickTime / 0.050f) * Math::Cos((float)nMoveDirection * 0.785398f + fZ) * (3.5f - iIsStealth * 2.5f);
	}
#else
	// DEBUG - this is STILL not finished, need to redo properly
	// need to do linear acceleration and deceleration
	if (nMoveDirection == -1)
	{
		Vector2 oVel(fVelX, fVelY);
		float fLength = oVel.Unitize();
		fLength -= 0.25f;
		if (fLength > 0) oVel *= fLength; else oVel *= 0;
		fVelX = oVel.x;
		fVelY = oVel.y;
	}
	else if (nMoveDirection >= 0 && nMoveDirection < 8)
	{
		fVelX += 0.25f * Math::Sin((float)nMoveDirection * 0.785398f + fZ);
		fVelY += 0.25f * Math::Cos((float)nMoveDirection * 0.785398f + fZ);
		Vector2 oVel(fVelX, fVelY);
		float fLength = oVel.Unitize();
		if (fLength - 0.5f > 3.5f - iIsStealth * 1.5f) {
			fLength -= 0.5f;
			oVel *= fLength;
			fVelX = oVel.x;
			fVelY = oVel.y;
		} else if (fLength > 3.5f - iIsStealth * 1.5f) {
			oVel *= 3.5f - iIsStealth * 1.5f;
			fVelX = oVel.x;
			fVelY = oVel.y;
		}
	}
	else printf("WARNING: Invalid nMoveDirection = %d!\n", nMoveDirection);
#endif

	// Update the player positions
	fOldX = fX;
	fOldY = fY;
	fX += fVelX;
	fY += fVelY;
}

void CPlayer::CalcColResp()
{
	// is the player dead?
	if (IsDead()) return;

	int			iWhichCont, iWhichVert;
	Vector2		oVector, oClosestPoint;
	Real		oShortestDistance;
	//Real		oDistance;
	//float		fVelXPercentage, fVelYPercentage;

	while (true)
	{
		// check for collision
		if (!ColHandCheckPlayerPos(&fX, &fY, &oShortestDistance, &oClosestPoint, &iWhichCont, &iWhichVert))
		{
			// DEBUG - player-player collision
			/*for (u_int iLoop1 = 0; iLoop1 < nPlayerCount; iLoop1++)
			{
				// dont check for collision with yourself
				if (iLoop1 == iID)
					continue;

				// calculate the displacement
				oVector.x = oPlayers[iID]->GetX() - PlayerGet(iLoop1)->GetX();
				oVector.y = oPlayers[iID]->GetY() - PlayerGet(iLoop1)->GetY();
				oDistance = oVector.Length();

				if (oDistance < oShortestDistance)
				{
					oShortestDistance = oDistance;
					oClosestPoint.x = PlayerGet(iLoop1)->GetX();
					oClosestPoint.y = PlayerGet(iLoop1)->GetY();
				}
			}*/

			oVector.x = oClosestPoint.x - fX;
			oVector.y = oClosestPoint.y - fY;
			//fX = fX - (oVector.x / (oShortestDistance / PLAYER_HALF_WIDTH) - oVector.x);
			//fY = fY - (oVector.y / (oShortestDistance / PLAYER_HALF_WIDTH) - oVector.y);
			fX -= (float)(oVector.x * PLAYER_HALF_WIDTH / oShortestDistance - oVector.x);
			fY -= (float)(oVector.y * PLAYER_HALF_WIDTH / oShortestDistance - oVector.y);
		}
		else
			break;
	}

	// player-player collision - only check if we're moving
	/*if (fVelX || fVelY)
	{
		for (u_int iLoop1 = 0; iLoop1 < nPlayerCount; iLoop1++)
		{
			// dont check for collision with yourself
			if (iLoop1 == iID)
				continue;

			// calculate the displacement
			oVector.x = oPlayers[iID]->GetX() - PlayerGet(iLoop1)->GetX();
			oVector.y = oPlayers[iID]->GetY() - PlayerGet(iLoop1)->GetY();
			oShortestDistance = oVector.Length();

			if (PlayerGet(iLoop1)->GetVelX() || PlayerGet(iLoop1)->GetVelY())
			// the other player is moving
			{
				if (oShortestDistance < PLAYER_WIDTH - PLAYER_COL_DET_TOLERANCE)
				// we're colliding with the other player
				{
					fVelXPercentage = Math::FAbs(fVelX / (Math::FAbs(fVelX) + Math::FAbs(PlayerGet(iLoop1)->GetVelX())));
					fVelYPercentage = Math::FAbs(fVelY / (Math::FAbs(fVelY) + Math::FAbs(PlayerGet(iLoop1)->GetVelY())));

					// move us back slightly
					oPlayers[iID]->SetX(oPlayers[iID]->GetX() + (oVector.x * PLAYER_WIDTH / oShortestDistance - oVector.x) * fVelXPercentage);
					oPlayers[iID]->SetY(oPlayers[iID]->GetY() + (oVector.y * PLAYER_WIDTH / oShortestDistance - oVector.y) * fVelYPercentage);

					// move the other player away slightly
					PlayerGet(iLoop1)->SetX(PlayerGet(iLoop1)->GetX() - (oVector.x * PLAYER_WIDTH / oShortestDistance - oVector.x) * (1.0 - fVelXPercentage));
					PlayerGet(iLoop1)->SetY(PlayerGet(iLoop1)->GetY() - (oVector.y * PLAYER_WIDTH / oShortestDistance - oVector.y) * (1.0 - fVelYPercentage));

					CalcColResp();
				}
			}
			else
			// the other player is NOT moving
			{
				if (oShortestDistance < PLAYER_WIDTH - PLAYER_COL_DET_TOLERANCE)
				// we're colliding with the other player
				{
					// move us back
					fX += oVector.x * PLAYER_WIDTH / oShortestDistance - oVector.x;
					fY += oVector.y * PLAYER_WIDTH / oShortestDistance - oVector.y;

					// check for collision
					if (!ColHandCheckPlayerPos(&fX, &fY))
					{
						//fX = fOldX;
						//fY = fOldY;
						CalcColResp();
					}
				}
			}
		}
	}*/
}

float CPlayer::GetIntX()
{
	return fIntX;
}

float CPlayer::GetIntY()
{
	return fIntY;
}

float CPlayer::GetX()
{
	return fX;
}

float CPlayer::GetY()
{
	return fY;
}

float CPlayer::GetOldX()
{
	return fOldX;
}

float CPlayer::GetOldY()
{
	return fOldY;
}

void CPlayer::SetX(float fValue)
{
	// is the player dead?
	if (IsDead()) return;

	fX = fValue;
}

void CPlayer::SetY(float fValue)
{
	// is the player dead?
	if (IsDead()) return;

	fY = fValue;
}

void CPlayer::SetOldX(float fValue)
{
	// is the player dead?
	if (IsDead()) return;

	fOldX = fValue;
}

void CPlayer::SetOldY(float fValue)
{
	// is the player dead?
	if (IsDead()) return;

	fOldY = fValue;
}

void CPlayer::Position(float fNewX, float fNewY, float fNewZ)
{
	// is the player dead?
	if (IsDead()) return;

	fIntX = fOldX = fX = fNewX;
	fIntY = fOldY = fY = fNewY;
	fZ = fOldZ = fNewZ;

	fVelX = 0.0f;
	fVelY = 0.0f;

#ifdef EX0_CLIENT
	// Reset state history
	oStateHistory.clear();
	oOnlyKnownState.fX = fNewX;
	oOnlyKnownState.fY = fNewY;
	oOnlyKnownState.fZ = fNewZ;
#endif
}

#ifdef EX0_CLIENT
void CPlayer::Position(float fNewX, float fNewY, float fNewZ, u_char cSequenceNumber)
{
	// is the player dead?
	if (IsDead()) return;

	fIntX = fOldX = fX = fNewX;
	fIntY = fOldY = fY = fNewY;
	fZ = fOldZ = fNewZ;

	fVelX = 0.0f;
	fVelY = 0.0f;

	// Reset state history
	oStateHistory.clear();
	SequencedState_t oSequencedState;
	oSequencedState.cSequenceNumber = cSequenceNumber;
	oSequencedState.oState.fX = fNewX;
	oSequencedState.oState.fY = fNewY;
	oSequencedState.oState.fZ = fNewZ;
	oStateHistory.push_front(oSequencedState);
}
#endif

float CPlayer::GetVelX()
{
	return fVelX;
}

float CPlayer::GetVelY()
{
	return fVelY;
}

void CPlayer::SetVelX(float fValue)
{
	// is the player dead?
	if (IsDead()) return;

	fVelX = fValue;
}

void CPlayer::SetVelY(float fValue)
{
	// is the player dead?
	if (IsDead()) return;

	fVelY = fValue;
}

float CPlayer::GetZ()
{
	// DEBUG - yet another hack.. replace it with some proper network-syncronyzed view bobbing
	//return fZ + Math::Sin(glfwGetTime() * 7.5) * GetVelocity() * 0.005;
	return fZ;
}

float CPlayer::GetVelocity()
{
	// is the player dead?
	if (fHealth <= 0.0f) return 0.0f;

	//return (fVelX || fVelY) ? 3.5f - iIsStealth * 2.0f : 0.0f;
	return Math::Sqrt(fVelX * fVelX + fVelY * fVelY);
}

void CPlayer::MoveDirection(int nDirection)
{
	eX0_assert(nDirection >= -1 && nDirection < 8, "nDirection >= -1 && nDirection < 8");

	nMoveDirection = nDirection;
}

void CPlayer::Rotate(float fAmount)
{
	// is the player dead?
	if (IsDead()) return;

	SetZ(fZ + fAmount);
}

void CPlayer::SetZ(float fValue)
{
	// is the player dead?
	if (IsDead()) return;

	fZ = fValue;
	if (fZ >= Math::TWO_PI) fZ -= Math::TWO_PI;
	if (fZ < 0.0) fZ += Math::TWO_PI;
}

#ifdef EX0_CLIENT
void CPlayer::SetLastLatency(u_short nLastLatency) { m_nLastLatency = nLastLatency; }
u_short CPlayer::GetLastLatency() const { return m_nLastLatency; }
#endif

void CPlayer::Render()
{
#ifdef EX0_CLIENT
	// select player color
	/*if (iID != iLocalPlayerID && !Trace(pLocalPlayer->GetIntX(), pLocalPlayer->GetIntY(), fIntX, fIntY))
		glColor3f(0.2, 0.5, 0.2);
	else */if (IsDead())
		glColor3f(0.1f, 0.1f, 0.1f);
	else if (m_nTeam == 0)
		glColor3f(1, 0, 0);
	else if (m_nTeam == 1)
		glColor3f(0, 0, 1);
	else
		glColor3f(0, 1, 0);

	if (bWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (iID == iLocalPlayerID)
	// local player
	{
		//OglUtilsSetMaskingMode(NO_MASKING_MODE);
		RenderOffsetCamera(true);
		gluPartialDisk(oQuadricObj, 6, 8, 12, 1, 30.0, 300.0);
		//OglUtilsSetMaskingMode(WITH_MASKING_MODE);

		// Render the weapon
		oWeapons[iSelWeapon].Render();

		// Draw the aiming-guide
		glLineWidth(1.0);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glShadeModel(GL_SMOOTH);
		glBegin(GL_LINES);
			glColor4f(0.9f, 0.2f, 0.1f, 0.5f);
			glVertex2i(0, 11);
			glColor4f(0.9f, 0.2f, 0.1f, 0.0f);
			glVertex2i(0, 600);
		glEnd();
		glShadeModel(GL_FLAT);
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
		glLineWidth(2.0);

		// Draw the cross section of the aiming-guide
		/*glBegin(GL_LINES);
			glColor3f(0.9, 0.2, 0.1);
			glVertex2i(-5, fAimingDistance);
			glVertex2i(5, fAimingDistance);
		glEnd();*/


		RenderOffsetCamera(false);
	}
	else
	// not local player
	{
		//if (Trace(pLocalPlayer->GetIntX(), pLocalPlayer->GetIntY(), fIntX, fIntY))
		//{
			glPushMatrix();

			RenderOffsetCamera(false);
			glTranslatef(fIntX, fIntY, 0);
			glRotatef(this->GetZ() * Math::RAD_TO_DEG, 0, 0, -1);
			gluPartialDisk(oQuadricObj, 6, 8, 12, 1, 30.0, 300.0);

			// Render the weapon
			oWeapons[iSelWeapon].Render();

			glPopMatrix();
		//}
	}
	if (bWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/*glBegin(GL_POINTS);
		glColor3f(0, 1, 0);
		glVertex2i(GetX(), GetY());
		glVertex2i(GetOldX(), GetOldY());
	glEnd();*/

	/*Ray2	oRay;
	oRay.Origin().x = fIntX;
	oRay.Origin().y = fIntY;
	oRay.Direction().x = -Math::Sin(fZ) * 100.0;
	oRay.Direction().y = -Math::Cos(fZ) * 100.0;
	Vector2	oVector = ColHandTrace(oRay);
	glBegin(GL_LINES);
		//glVertex2i((oRay.Origin() + oRay.Direction()).x, (oRay.Origin() + oRay.Direction()).y);
		glVertex2i(oVector.x, oVector.y);
		glVertex2i(oRay.Origin().x, oRay.Origin().y);
	glEnd();*/
#endif
}

#ifdef EX0_CLIENT
void CPlayer::PushStateHistory(SequencedState_t & oSequencedState)
{
	// Insert the only known state if history is empty
	if (oStateHistory.empty()) {
		SequencedState_t oFirstSequencedState;
		oFirstSequencedState.cSequenceNumber = (u_char)(oSequencedState.cSequenceNumber - 1);
		oFirstSequencedState.oState = oOnlyKnownState;
		oStateHistory.push_front(oFirstSequencedState);
	}

	if (oSequencedState.cSequenceNumber != oStateHistory.front().cSequenceNumber)
	{
		if (oStateHistory.size() >= 1000) oStateHistory.pop_back();
		oStateHistory.push_front(oSequencedState);
	}
}

State_t CPlayer::GetStateInPast(float fTimeAgo)
{
	float fHistoryX;
	float fHistoryY;
	float fHistoryZ;

	/*if (fTimeAgo == 0.0f) {
		fHistoryX = GetIntX();
		fHistoryY = GetIntY();
		fHistoryZ = GetZ();
	} else if (oStateHistory.size() == 1) {
		fHistoryX = oStateHistory.front().fX;
		fHistoryY = oStateHistory.front().fY;
		fHistoryZ = oStateHistory.front().fZ;
	} else {
		u_char cHistoryPoint = cCurrentCommandSequenceNumber;
		while (fTimeAgo > fTicks) {
			if (cHistoryPoint == oStateHistory.begin()) {
				break;
			} else {
				--cHistoryPoint;
				fTimeAgo -= fTickTime;
			}
		}
		if (cHistoryPoint != oStateHistory.begin()) {
			State_t oToState = oStateHistory[cHistoryPoint];
			--cHistoryPoint;
			State_t oFromState = oStateHistory[cHistoryPoint];

			fHistoryX = oFromState.fX + (oToState.fX - oFromState.fX) * (fTicks - fTimeAgo) / fTickTime;
			fHistoryY = oFromState.fY + (oToState.fY - oFromState.fY) * (fTicks - fTimeAgo) / fTickTime;

			float fDiffZ = oToState.fZ - oFromState.fZ;
			if (fDiffZ >= Math::PI) fDiffZ -= Math::TWO_PI;
			if (fDiffZ < -Math::PI) fDiffZ += Math::TWO_PI;
			fHistoryZ = oFromState.fZ + fDiffZ * (fTicks - fTimeAgo) / fTickTime;
		} else {
			fHistoryX = oStateHistory.front().fX;
			fHistoryY = oStateHistory.front().fY;
			fHistoryZ = oStateHistory.front().fZ;
		}
		if (fHistoryX == 0.0f || fHistoryY == 0.0f)
		{
			printf("Whoa %d %d %d %f %f\n", iID, cCurrentCommandSequenceNumber, cHistoryPoint, fTimeAgo, fTicks);
		}
	}*/
	/*if (fTimeAgo == 0.0f) {
		fHistoryX = GetIntX();
		fHistoryY = GetIntY();
		fHistoryZ = GetZ();
	} else */
	if (oStateHistory.size() == 0) {
		fHistoryX = oOnlyKnownState.fX;
		fHistoryY = oOnlyKnownState.fY;
		fHistoryZ = oOnlyKnownState.fZ;
	} else if (oStateHistory.size() == 1) {
		fHistoryX = oStateHistory.front().oState.fX;
		fHistoryY = oStateHistory.front().oState.fY;
		fHistoryZ = oStateHistory.front().oState.fZ;
	} else {
		float	fCurrentTimepoint = (u_char)(cCurrentCommandSequenceNumber - 1) + pLocalPlayer->fTicks / fTickTime;
		float	fHistoryPoint = fCurrentTimepoint - fTimeAgo / fTickTime;
		if (fHistoryPoint < 0) fHistoryPoint += 256;
		int		nTicksAgo = (int)floorf(fTimeAgo / fTickTime - pLocalPlayer->fTicks / fTickTime) + 1;
		//int		nTicksAgo = (int)(floorf(fTimeAgo / fTickTime - pLocalPlayer->fTicks / fTickTime) + 0.1) + 1;
		char	cLastKnownTickAgo = (char)(cCurrentCommandSequenceNumber - oStateHistory.front().cSequenceNumber);
		list<SequencedState_t>::iterator oHistoryTo = oStateHistory.begin();
		list<SequencedState_t>::iterator oHistoryFrom = oStateHistory.begin(); ++oHistoryFrom;
		//if ((char)((u_char)floorf(fHistoryPoint) - oStateHistory.front().cSequenceNumber) >= 5)
		if (cLastKnownTickAgo > nTicksAgo)
		// Extrapolate into the future
		{
			//printf("future %d %d\n", cLastKnownTickAgo, nTicksAgo);
			/*fHistoryX = GetIntX();
			fHistoryY = GetIntY();
			fHistoryZ = GetZ();*/

			State_t oStateTo = oHistoryTo->oState;
			State_t oStateFrom = oHistoryFrom->oState;

			float fHistoryTicks = fHistoryPoint - oHistoryFrom->cSequenceNumber;
			if (fHistoryTicks < 0) fHistoryTicks += 256;
			if (fHistoryTicks > ((u_char)(oHistoryTo->cSequenceNumber - oHistoryFrom->cSequenceNumber) + pLocalPlayer->fTicks + kfMaxExtrapolate) / fTickTime) {
				// Max forward extrapolation time
				fHistoryTicks = ((u_char)(oHistoryTo->cSequenceNumber - oHistoryFrom->cSequenceNumber) + pLocalPlayer->fTicks + kfMaxExtrapolate) / fTickTime;

				printf("Exceeding max EXTERP time (player %d).\n", iID);
				printf("cur state = %d; last known state = %d\n", cCurrentCommandSequenceNumber, oStateHistory.front().cSequenceNumber);
				printf("cLastKnownTickAgo %d > nTicksAgo %d\n", cLastKnownTickAgo, nTicksAgo);
			}// else
			//	printf("Exterping into the future (player %d).\n", iID);

			u_char cHistoryTickTime = (u_char)(oHistoryTo->cSequenceNumber - oHistoryFrom->cSequenceNumber);
			fHistoryX = oStateFrom.fX + (oStateTo.fX - oStateFrom.fX) * fHistoryTicks / cHistoryTickTime;
			fHistoryY = oStateFrom.fY + (oStateTo.fY - oStateFrom.fY) * fHistoryTicks / cHistoryTickTime;

			float fDiffZ = oStateTo.fZ - oStateFrom.fZ;
			if (fDiffZ >= Math::PI) fDiffZ -= Math::TWO_PI;
			if (fDiffZ < -Math::PI) fDiffZ += Math::TWO_PI;
			if (fHistoryTicks > 1.0f) fHistoryTicks = sqrtf(fHistoryTicks);
			fHistoryZ = oStateFrom.fZ + fDiffZ * fHistoryTicks / cHistoryTickTime;
		}
		/*else if ((char)((u_char)floorf(fHistoryPoint) - oStateHistory.back().cSequenceNumber) < 0)
		// DEBUG: Need a better way to figure out if outside history range (for > 256 values
		// Way in the past, not enough history to interpolate
		{
			fHistoryX = oStateHistory.back().oState.fX;
			fHistoryY = oStateHistory.back().oState.fY;
			fHistoryZ = oStateHistory.back().oState.fZ;
		}*/
		else
		// Use the known history to interpolate
		{
			//bool bUseHistory = true;
			u_char cPeriodLength = 0;
			nTicksAgo -= cLastKnownTickAgo;
			while (oHistoryFrom != oStateHistory.end() &&
				//(char)((u_char)floorf(fHistoryPoint) - oHistoryFrom->cSequenceNumber) < 0) {
				nTicksAgo >= (cPeriodLength = (u_char)(oHistoryTo->cSequenceNumber - oHistoryFrom->cSequenceNumber))) {
				/*if (oHistoryFrom == oStateHistory.end()) {
					fHistoryX = oStateHistory.back().oState.fX;
					fHistoryY = oStateHistory.back().oState.fY;
					fHistoryZ = oStateHistory.back().oState.fZ;
					bUseHistory = false;
					break;
				} else {
					++oHistoryTo;
					++oHistoryFrom;
				}*/
				nTicksAgo -= cPeriodLength;
				++oHistoryTo;
				++oHistoryFrom;
			}

			if (oHistoryFrom == oStateHistory.end()) {
				fHistoryX = oStateHistory.back().oState.fX;
				fHistoryY = oStateHistory.back().oState.fY;
				fHistoryZ = oStateHistory.back().oState.fZ;
			} else {
				eX0_assert(oHistoryFrom->cSequenceNumber != oHistoryTo->cSequenceNumber, "dup!");
				if (oHistoryFrom->cSequenceNumber < oHistoryTo->cSequenceNumber)
				{
					/*eX0_assert(oHistoryFrom->cSequenceNumber <= fHistoryPoint
						&& fHistoryPoint <= oHistoryTo->cSequenceNumber,
						"not in seq");*/
					if (!(oHistoryFrom->cSequenceNumber <= fHistoryPoint
						&& fHistoryPoint <= oHistoryTo->cSequenceNumber))
					printf("%d <= %f <= %d\n", oHistoryFrom->cSequenceNumber,
											   fHistoryPoint,
											   oHistoryTo->cSequenceNumber);
					//else
					//	printf("%d <= %f <= %d ALL OK\n", oHistoryFrom->cSequenceNumber,
					//						   fHistoryPoint,
					//						   oHistoryTo->cSequenceNumber);
				}

				State_t oStateTo = oHistoryTo->oState;
				State_t oStateFrom = oHistoryFrom->oState;

				//float fHistoryTicks = fHistoryPoint - oHistoryFrom->cSequenceNumber;
				float fHistoryTicks = fHistoryPoint - oHistoryFrom->cSequenceNumber;
				if (fHistoryTicks < 0) fHistoryTicks += 256;
				u_char cHistoryTickTime = (u_char)(oHistoryTo->cSequenceNumber - oHistoryFrom->cSequenceNumber);
				//eX0_assert(fHistoryTicks < cHistoryTickTime, "fHistoryTicks not in range");
				if (!(fHistoryTicks <= cHistoryTickTime))
					printf("fHistoryTicks not in range: %f\n", fHistoryTicks / cHistoryTickTime);
				//else
				//	printf("fHistoryTicks IS OK: %f\n", fHistoryTicks / cHistoryTickTime);
				fHistoryX = oStateFrom.fX + (oStateTo.fX - oStateFrom.fX) * fHistoryTicks / cHistoryTickTime;
				fHistoryY = oStateFrom.fY + (oStateTo.fY - oStateFrom.fY) * fHistoryTicks / cHistoryTickTime;

				float fDiffZ = oStateTo.fZ - oStateFrom.fZ;
				if (fDiffZ >= Math::PI) fDiffZ -= Math::TWO_PI;
				if (fDiffZ < -Math::PI) fDiffZ += Math::TWO_PI;
				fHistoryZ = oStateFrom.fZ + fDiffZ * fHistoryTicks / cHistoryTickTime;
			}
		}
	}

	State_t oState;
	oState.fX = fHistoryX;
	oState.fY = fHistoryY;
	oState.fZ = fHistoryZ;
	return oState;
}

void CPlayer::RenderInPast(float fTimeAgo)
{
	State_t oState = GetStateInPast(fTimeAgo);

	// select player color
	if (IsDead())
		glColor3f(0.1f, 0.1f, 0.1f);
	else if (m_nTeam == 0)
		glColor3f(1, 0, 0);
	else if (m_nTeam == 1)
		glColor3f(0, 0, 1);
	else
		glColor3f(0, 1, 0);

	if (iID == iLocalPlayerID)
		glColor3f(0.5f, 0.5f, 0.5f);

	if (bWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPushMatrix();
	if (iID == iLocalPlayerID) OglUtilsSetMaskingMode(NO_MASKING_MODE);
	RenderOffsetCamera(false);
	glTranslatef(oState.fX, oState.fY, 0);
	glRotatef(oState.fZ * Math::RAD_TO_DEG, 0, 0, -1);
	gluPartialDisk(oQuadricObj, 6, 8, 12, 1, 30.0, 300.0);
	glBegin(GL_QUADS);
		glVertex2i(-1, 11);
		glVertex2i(-1, 3);
		glVertex2i(1, 3);
		glVertex2i(1, 11);
	glEnd();
	if (iID == iLocalPlayerID) OglUtilsSetMaskingMode(WITH_MASKING_MODE);
	glPopMatrix();
	if (bWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
#endif

void CPlayer::InitWeapons()
{
	oWeapons[0].Init(iID, 0);
	oWeapons[1].Init(iID, 0);
	oWeapons[2].Init(iID, 1);
	oWeapons[3].Init(iID, 2);
}

void CPlayer::UpdateInterpolatedPos()
{
	// is the player dead?
	if (IsDead()) {printf("assertion failed\n");return;}

	fIntX = fOldX + (fX - fOldX) * fTicks / fTickTime;
	fIntY = fOldY + (fY - fOldY) * fTicks / fTickTime;
	//fIntX = fX;
	//fIntY = fY;
}

string & CPlayer::GetName(void) { return sName; }
void CPlayer::SetName(string & sNewName) {
	if (sNewName.length() > 0) sName = sNewName.substr(0, 32);
	else sName = "Unnamed Player";
}

void CPlayer::Add(CPlayer * pPlayer)
{
	printf("Before Add():"); for (vector<CPlayer *>::const_iterator cit1 = m_oPlayers.begin(); cit1 < m_oPlayers.end(); ++cit1) {
		printf(" %p", *cit1);
	} printf("\n");
	m_oPlayers.at(pPlayer->iID = NextFreePlayerId()) = pPlayer;
	printf("After Add():"); for (vector<CPlayer *>::const_iterator cit1 = m_oPlayers.begin(); cit1 < m_oPlayers.end(); ++cit1) {
		printf(" %p", *cit1);
	} printf("\n");
}

void CPlayer::Add(CPlayer * pPlayer, u_int nPlayerId)
{
	printf("Before Add(int):"); for (vector<CPlayer *>::const_iterator cit1 = m_oPlayers.begin(); cit1 < m_oPlayers.end(); ++cit1) {
		printf(" %p", *cit1);
	} printf("\n");
	if (nPlayerId >= m_oPlayers.size())
		m_oPlayers.resize(nPlayerId + 1);
	else if (m_oPlayers.at(nPlayerId) != NULL) throw 1;
	m_oPlayers.at(pPlayer->iID = nPlayerId) = pPlayer;
	printf("After Add(int):"); for (vector<CPlayer *>::const_iterator cit1 = m_oPlayers.begin(); cit1 < m_oPlayers.end(); ++cit1) {
		printf(" %p", *cit1);
	} printf("\n");
}

u_int CPlayer::NextFreePlayerId()
{
	// Look for a free slot
	for (vector<CPlayer *>::const_iterator cit1 = m_oPlayers.begin(); cit1 < m_oPlayers.end(); ++cit1) {
		if (*cit1 == NULL)
			return cit1 - m_oPlayers.begin();
	}

	// No free slots in the current array, so extend it by 1
	m_oPlayers.push_back(NULL);
	return m_oPlayers.size() - 1;
}

void CPlayer::Remove(CPlayer * pPlayer)
{
	for (vector<CPlayer *>::iterator it1 = m_oPlayers.begin(); it1 < m_oPlayers.end(); ++it1) {
		if (*it1 == pPlayer) {
			*it1 = NULL;
			return;
		}
	}
}

void CPlayer::RemoveAll()
{
	for (vector<CPlayer *>::iterator it1 = m_oPlayers.begin(); it1 < m_oPlayers.end(); ++it1) {
		if (*it1 != NULL) {
			delete *it1;
			*it1 = NULL;
		}
	}
	m_oPlayers.clear();
}

// Returns a player
CPlayer * PlayerGet(u_int nPlayerId)
{
	if (nPlayerId >= CPlayer::m_oPlayers.size()) return NULL;
	else return CPlayer::m_oPlayers.at(nPlayerId);
}

void PlayerTick()
{
#ifdef EX0_CLIENT
	for (u_int iLoop1 = 0; iLoop1 < nPlayerCount; ++iLoop1)
	{
		if (PlayerGet(iLoop1) != NULL)
			//PlayerGet(iLoop1)->Tick();
			PlayerGet(iLoop1)->ProcessAuthUpdateTEST();
	}

	// update local player interpolated pos
	//pLocalPlayer->UpdateInterpolatedPos();
#endif // EX0_CLIENT
}
