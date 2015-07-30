// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

uint8	nPlayerCount = 0;
#ifdef EX0_CLIENT
CPlayer *	pLocalPlayer = NULL;
string		sLocalPlayerName = "New Player";
#endif

std::vector<CPlayer *> CPlayer::m_oPlayers;
u_int CPlayer::m_nPlayerCount = 0;

// implementation of the player class
CPlayer::CPlayer()
	: m_pController(NULL),
	  m_pStateAuther(NULL),
	  m_dNextUpdateTime(GLFW_INFINITY),
	  oWeapons(NextFreePlayerId())
{
	//printf("CPlayer(%p) Ctor.\n", this);

	// init vars
	RespawnReset();
	sName = "Unnamed Player";
	m_nTeam = 2;

	// Network related
	pConnection = NULL;

	Add(this);
	++m_nPlayerCount;
}

CPlayer::CPlayer(uint8 nPlayerId)
	: m_pController(NULL),
	  m_pStateAuther(NULL),
	  m_dNextUpdateTime(GLFW_INFINITY),
	  oWeapons(nPlayerId)
{
	//printf("CPlayer(%p) Ctor.\n", this);

	// init vars
	RespawnReset();
	sName = "Unnamed Player";
	m_nTeam = 2;

	// Network related
	pConnection = NULL;

	Add(this, nPlayerId);
	++m_nPlayerCount;
}

CPlayer::~CPlayer()
{
	Remove(this);
	--m_nPlayerCount;

	delete m_pController;
	delete m_pStateAuther;

	// DEBUG: Rethink this... Make a clear ownership relationship, and make sure this works on deinit and normal player leave/kick event
	//        i.e. Connection owns Player. If Connection deleted, it deletes Player.
	//             If Player is deleted, it lets Connection know (but doesn't directly do stuff in it).
	if (pConnection != NULL && pConnection->HasPlayer() && !pConnection->IsMultiPlayer()) {
		pConnection->RemovePlayer();
	}

	printf("CPlayer(%p) ~Dtor.\n", this);
}

void CPlayer::SeekRealtimeInput(double dTimePassed)
{
	// is the player dead?
	if (GetTeam() == 2 || IsDead()) return;

	LocalController * pLocalController = dynamic_cast<LocalController *>(m_pController);

	if (NULL != pLocalController) {
		pLocalController->ProvideRealtimeInput(dTimePassed);
	}
}

void CPlayer::Tick()
{
//#ifdef EX0_CLIENT
#if 0
	// is the player dead?
	if (IsDead()) {
		return;
	}

	//oWeapons[iSelWeapon].Tick();

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

		// DEBUG: A work in progress
		m_pPlayer->RequestInput(0);

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
				oInput.bStealth = (bool)iIsStealth;
				oInput.fZ = GetZ();
				//if (oUnconfirmedInputs.size() < 101)
				//oUnconfirmedInputs.push_back(oInput);
				Move_t oMove;
				oMove.oInput = oInput;
				oMove.oState.fX = GetX(); oMove.oState.fY = GetY(); oMove.oState.fZ = GetZ();
				oUnconfirmedMoves.push(oMove, cCurrentCommandSequenceNumber);
//printf("pushed a move on oUnconfirmedMoves, size() = %d, cur# => %d\n", oUnconfirmedMoves.size(), cCurrentCommandSequenceNumber);

				iTempInt = std::max<int>(iTempInt, (int)oUnconfirmedMoves.size() - 1);

				// Send the Client Command packet
				CPacket oClientCommandPacket;
				oClientCommandPacket.pack("cccc", (u_char)1,		// packet type
												 cCurrentCommandSequenceNumber,		// sequence number
												 cCurrentCommandSeriesNumber,		// series number
												 (u_char)(oUnconfirmedMoves.size() - 1));
				for (u_char it1 = oUnconfirmedMoves.begin(); it1 != oUnconfirmedMoves.end(); ++it1)
				{
					oClientCommandPacket.pack("cbf", oUnconfirmedMoves[it1].oInput.cMoveDirection,
													 oUnconfirmedMoves[it1].oInput.bStealth,
													 oUnconfirmedMoves[it1].oInput.fZ);
				}
				if ((rand() % 100) >= 0 || iLocalPlayerID != 0) // DEBUG: Simulate packet loss
					pServer->SendUdp(oClientCommandPacket);

				// DEBUG: Keep state history for local player
				/*if ((rand() % 100) >= 0) {
					SequencedState_st oSequencedState;
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
#endif // 0

#ifdef EX0_CLIENT
	//if (pLocalPlayer == NULL || pLocalPlayer->GetTeam() == 2 || pLocalPlayer->IsDead())
	if (GetTeam() == 2 || IsDead())
		return;

	if (m_pController != NULL)
	{
		// TODO: This is why it needed to be an uint64... Or does it?
		//while (this->GlobalStateSequenceNumberTEST < g_pGameSession->GlobalStateSequenceNumberTEST)
		while (static_cast<char>(g_pGameSession->GlobalStateSequenceNumberTEST - this->GlobalStateSequenceNumberTEST) > 0)
		{
			m_pController->RequestNextCommand();

			++this->GlobalStateSequenceNumberTEST;
		}
	}

	/*
	//if (iID == iLocalPlayerID)
	//if (this == pLocalPlayer)
	if (true == m_pController->IsLocal())
	{
		// DEBUG: A work in progress...
		//if (pLocalServer == NULL && pServer != NULL)
		if (false == m_pStateAuther->IsLocal())
		{
			//if (oUnconfirmedMoves.size() < 100)
			//{
				// calculate player trajectory
				CalcTrajs();

				// do collision response for player
				CalcColResp();

				Input_t oInput;
				oInput.cMoveDirection = (char)nMoveDirection;
				oInput.bStealth = (u_char)iIsStealth;
				oInput.fZ = GetZ();
				//if (oUnconfirmedInputs.size() < 101)
				//oUnconfirmedInputs.push_back(oInput);
				Move_t oMove;
				oMove.oInput = oInput;
				oMove.oState.fX = GetX(); oMove.oState.fY = GetY(); oMove.oState.fZ = GetZ();
				oUnconfirmedMoves.push(oMove, g_cCurrentCommandSequenceNumber);
	//printf("pushed a move on oUnconfirmedMoves, size() = %d, cur# => %d\n", oUnconfirmedMoves.size(), cCurrentCommandSequenceNumber);

				iTempInt = std::max<int>(iTempInt, (int)oUnconfirmedMoves.size() - 1);

				// Send the Client Command packet
				CPacket oClientCommandPacket;
				oClientCommandPacket.pack("cccc", (u_char)1,		// packet type
												  g_cCurrentCommandSequenceNumber,		// sequence number
												  static_cast<NetworkStateAuther *>(m_pStateAuther)->cCurrentCommandSeriesNumber,		// series number
												  (u_char)(oUnconfirmedMoves.size() - 1));
				for (u_char it1 = oUnconfirmedMoves.begin(); it1 != oUnconfirmedMoves.end(); ++it1)
				{
					oClientCommandPacket.pack("cbf", oUnconfirmedMoves[it1].oInput.cMoveDirection,
													 oUnconfirmedMoves[it1].oInput.bStealth,
													 oUnconfirmedMoves[it1].oInput.fZ);
				}
				if ((rand() % 100) >= 0 || iLocalPlayerID != 0) // DEBUG: Simulate packet loss
					pServer->SendUdp(oClientCommandPacket);

				// DEBUG: Keep state history for local player
				/*if ((rand() % 100) >= 0) {
					SequencedState_st oSequencedState;
					oSequencedState.cSequenceNumber = cCurrentCommandSequenceNumber;
					oSequencedState.oState = oMove.oState;
					PushStateHistory(oSequencedState);
				}* /
			//}
		}
		else
		{
			SequencedInput_t oSequencedInput;

			// Set the inputs
			oSequencedInput.oInput.cMoveDirection = (char)this->nMoveDirection;
			oSequencedInput.oInput.bStealth = (u_char)this->iIsStealth;
			oSequencedInput.oInput.fZ = this->GetZ();

			oSequencedInput.cSequenceNumber = g_cCurrentCommandSequenceNumber;

			eX0_assert(m_oCommandsQueue.push(oSequencedInput), "m_oCommandsQueue.push(oInput) failed, lost input!!\n");
		}
	} else {
		// calculate player trajectory
		//CalcTrajs();
		/*fOldX = fX;
		fOldY = fY;
		fX += fVelX;
		fY += fVelY;

		// do collision response for player
		CalcColResp();* /
		//++cCurrentCommandSequenceNumber;
	}
	*/
#endif
}

void CPlayer::WeaponTickTEST()
{
	//if (bWeaponFireTEST) Fire();
	//oWeapons[iSelWeapon].Tick();

	if (GetTeam() == 2 || IsDead())
		return;

	if (m_pController != NULL)
	{
		// while (static_cast<char>(g_pGameSession->GlobalStateSequenceNumberTEST - this->GlobalStateSequenceNumberTEST) > 0)
		{
			m_pController->RequestNextWpnCommand();

			// ++this->GlobalStateSequenceNumberTEST;
		}
	}
}

uint32 CPlayer::GetAmmo()
{
	return oWeapons.GetAmmo();
}
// returns number of clips left in the selected weapon
uint32 CPlayer::GetClips()
{
	return oWeapons.GetClips();
}

bool CPlayer::IsReloading()
{
	return oWeapons.IsReloading();
}

// buy a clip for selected weapon
void CPlayer::BuyClip()
{
	// is the player dead?
	if (IsDead()) return;

	// TODO: some kind of money system?
	// just give it to whoever
	oWeapons.GiveClip();
}

void CPlayer::SetTeam(int nTeam)
{
	m_nTeam = nTeam;
}

int CPlayer::GetTeam()
{
	return m_nTeam;
}

// A partial reset of the player state that happens when the player respawns (or changes team, etc.)
void CPlayer::RespawnReset()
{
	fZ = 0;
	fHealth = 100;

	InitWeapons();

	if (nullptr != m_pController)
		m_pController->Reset();
}

bool CPlayer::IsDead()
{
	return (fHealth <= 0);
}

float CPlayer::GetHealth()
{
	return fHealth;
}

void CPlayer::GiveHealth(float fValue, int /*nSourcePlayerId*/)
{
	// is the player dead?
	if (IsDead()) return;

	if (GetTeam() != 2 && fHealth + fValue <= 0) {
		if (nullptr != dynamic_cast<LocalController *>(m_pController)) {
			m_oDeadState = GetStateInPast(0, &g_pGameSession->LogicTimer());
			m_oDeadState.fZ = GetZ();
		} else
			m_oDeadState = GetStateInPast(kfInterpolate, &g_pGameSession->LogicTimer());
	}

	if (nullptr != pGameServer) {
		// TEST: Send the Player Was Hit packet
		CPacket oPlayerWasHit;
		oPlayerWasHit.pack("hccf", 0, (uint8)40, (uint8)iID, (float)fValue);
		oPlayerWasHit.CompleteTpcPacketSize();
		ClientConnection::BroadcastTcp(oPlayerWasHit);
	}

	fHealth += fValue;
	if (fHealth < 0) fHealth = 0;

	if (fHealth > 0) {
		PlaySound("data/sounds/hit-1.wav");
	} else {
		// Dead sound
		PlaySound("data/sounds/die-1.wav");
	}
}

SequencedState_st CPlayer::PhysicsTickTEST(SequencedState_st oStartingState, SequencedCommand_st oCommand)
{
	eX0_assert(oStartingState.cSequenceNumber == oCommand.cSequenceNumber, "PhysicsTickTEST(): StateSN and CmdSN don't match");

	// Set inputs (command)
	m_oCommand = oCommand.oCommand;

	// Calculate the next resulting state:
	// Starting State + Associated Command => Next State
	CalcTrajs(oStartingState.oState);
	CalcColResp(oStartingState.oState);

	oStartingState.cSequenceNumber += 1;		// Next state has the sequence number one higher

	return oStartingState;
}

void CPlayer::CalcTrajs(State_st & oState)
{
	//float fVelX, fVelY;

	#define TOP_SPEED (3.5f)
#if 1
	// Update the player velocity (acceleration)
	if (m_oCommand.cMoveDirection == -1)
	{
		oState.fVelX = 0.0;
		oState.fVelY = 0.0;
	}
	else
	{
		oState.fVelX = (20.0f / g_cCommandRate) * Math::Sin((float)m_oCommand.cMoveDirection * 0.785398f + m_oCommand.fZ) * (TOP_SPEED - m_oCommand.bStealth * 1.5f);
		oState.fVelY = (20.0f / g_cCommandRate) * Math::Cos((float)m_oCommand.cMoveDirection * 0.785398f + m_oCommand.fZ) * (TOP_SPEED - m_oCommand.bStealth * 1.5f);
	}
#elif 0
	// DEBUG - this is STILL not finished, need to redo properly
	// need to do linear acceleration and deceleration
	if (m_oCommand.cMoveDirection == -1)
	{
		Vector2 oVel(oState.fVelX, oState.fVelY);
		float fLength = oVel.Unitize();
		fLength -= 0.25f;
		if (fLength > 0) oVel *= fLength; else oVel *= 0;
		oState.fVelX = oVel.x;
		oState.fVelY = oVel.y;
	}
	else if (m_oCommand.cMoveDirection >= 0 && m_oCommand.cMoveDirection < 8)
	{
		oState.fVelX += 0.25f * Math::Sin((float)m_oCommand.cMoveDirection * 0.785398f + m_oCommand.fZ);
		oState.fVelY += 0.25f * Math::Cos((float)m_oCommand.cMoveDirection * 0.785398f + m_oCommand.fZ);
		Vector2 oVel(oState.fVelX, oState.fVelY);
		float fLength = oVel.Unitize();
		if (fLength - 0.5f > TOP_SPEED - m_oCommand.bStealth * 1.5f) {
			fLength -= 0.5f;
			oVel *= fLength;
			oState.fVelX = oVel.x;
			oState.fVelY = oVel.y;
		} else if (fLength > TOP_SPEED - m_oCommand.bStealth * 1.5f) {
			oVel *= TOP_SPEED - m_oCommand.bStealth * 1.5f;
			oState.fVelX = oVel.x;
			oState.fVelY = oVel.y;
		}
	}
	else printf("WARNING: Invalid nMoveDirection = %d!\n", m_oCommand.cMoveDirection);
#else
	Vector2 TargetVel(Vector2::ZERO);

	if (m_oCommand.cMoveDirection == -1)
	{
		/// TargetVel.x = 0.0f;
		/// TargetVel.y = 0.0f;
	}
	else if (m_oCommand.cMoveDirection >= 0 && m_oCommand.cMoveDirection < 8)
	{
		TargetVel.x = (20.0f / g_cCommandRate) * Math::Sin((float)m_oCommand.cMoveDirection * 0.785398f + m_oCommand.fZ) * (TOP_SPEED - m_oCommand.bStealth * 2.25f);
		TargetVel.y = (20.0f / g_cCommandRate) * Math::Cos((float)m_oCommand.cMoveDirection * 0.785398f + m_oCommand.fZ) * (TOP_SPEED - m_oCommand.bStealth * 2.25f);
	}
	else printf("WARNING: Invalid nMoveDirection = %d!\n", m_oCommand.cMoveDirection);

	Vector2 CurrentVel(oState.fVelX, oState.fVelY);
	Vector2 Delta(TargetVel - CurrentVel);
	float DeltaLength = Delta.Unitize();

	float Move1 = DeltaLength * DeltaLength * 0.03f;
	float Move2 = std::min(0.2f, DeltaLength);

	CurrentVel += Delta * std::max(Move1, Move2);

	oState.fVelX = CurrentVel.x;
	oState.fVelY = CurrentVel.y;
#endif

	// Update the player positions
	m_fOldX = oState.fX;
	m_fOldY = oState.fY;
	oState.fX += oState.fVelX;
	oState.fY += oState.fVelY;
	oState.fZ = m_oCommand.fZ;
}

void CPlayer::CalcColResp(State_st & oState)
{
	int			iWhichCont, iWhichVert;
	Vector2		oVector, oClosestPoint;
	Real		oShortestDistance;
	//Real		oDistance;
	//float		fVelXPercentage, fVelYPercentage;

	State_st		oOriginalStateTEST = oState;

	int nTries = 30;
	while (nTries-- > 0)
	{
		// check for collision
		if (!ColHandCheckPlayerPos(&oState.fX, &oState.fY, &oShortestDistance, &oClosestPoint, &iWhichCont, &iWhichVert))
		{
			// DEBUG - player-player collision
			/*for (u_int iLoop1 = 0; iLoop1 < nPlayerCount; iLoop1++)
			{
				// dont check for collision with yourself
				if (iLoop1 == iID || PlayerGet(iLoop1) == NULL || PlayerGet(iLoop1)->IsDead())
					continue;

				// calculate the displacement
				oVector.x = PlayerGet(iID)->GetX() - PlayerGet(iLoop1)->GetX();
				oVector.y = PlayerGet(iID)->GetY() - PlayerGet(iLoop1)->GetY();
				oDistance = oVector.Length() * 0.5;

				if (oDistance < oShortestDistance)
				{
					oShortestDistance = oDistance;
					oClosestPoint.x = PlayerGet(iLoop1)->GetX();
					oClosestPoint.y = PlayerGet(iLoop1)->GetY();
					iWhichCont = iWhichVert = -1;
				}
			}*/

			// Closest to a wall
			if (iWhichCont != -1 && false /* don't use this for now, the extra work is not needed */)
			{
				//Vector2		oVector;
				//Segment2	oSegment;

				// Check if resolved point lies on the same side of wall
				// TODO: This can be optimized by using IsLeft() of the closest wall, instead of the expensive IsInside()
				//if (ColHandIsLeft

				oVector.x = oState.fX - oClosestPoint.x;
				oVector.y = oState.fY - oClosestPoint.y;
				oState.fX += (float)(oVector.x * PLAYER_HALF_WIDTH / oShortestDistance - oVector.x);
				oState.fY += (float)(oVector.y * PLAYER_HALF_WIDTH / oShortestDistance - oVector.y);

				if (ColHandIsPointInside(static_cast<int>(oState.fX), static_cast<int>(oState.fY)))
				{
					// Outside, move back
					oVector.Unitize();
					oState.fX -= (float)(oVector.x * PLAYER_WIDTH);
					oState.fY -= (float)(oVector.y * PLAYER_WIDTH);
				}
			}
			// Closest to another player
			else
			{
				oVector.x = oState.fX - oClosestPoint.x;
				oVector.y = oState.fY - oClosestPoint.y;
				//oState.fX = fX - (oVector.x / (oShortestDistance / PLAYER_HALF_WIDTH) - oVector.x);
				//oState.fY = fY - (oVector.y / (oShortestDistance / PLAYER_HALF_WIDTH) - oVector.y);
				oState.fX += (float)(oVector.x * PLAYER_HALF_WIDTH / oShortestDistance - oVector.x);
				oState.fY += (float)(oVector.y * PLAYER_HALF_WIDTH / oShortestDistance - oVector.y);
			}
		}
		else
			break;
	}
	if (nTries <= -1) {
		oState = oOriginalStateTEST;
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
					oState.fX += oVector.x * PLAYER_WIDTH / oShortestDistance - oVector.x;
					oState.fY += oVector.y * PLAYER_WIDTH / oShortestDistance - oVector.y;

					// check for collision
					if (!ColHandCheckPlayerPos(&oState.fX, &oState.fY))
					{
						//oState.fX = fOldX;
						//oState.fY = fOldY;
						CalcColResp();
					}
				}
			}
		}
	}*/

	// Update the velocity (based on collisions with things)
	oState.fVelX = oState.fX - m_fOldX;
	oState.fVelY = oState.fY - m_fOldY;
}

void CPlayer::Position(SequencedState_st oSequencedState)
{
	printf("Positioning player %d at %f, %f, %f [state=%d].\n", iID, oSequencedState.oState.fX, oSequencedState.oState.fY, oSequencedState.oState.fZ, oSequencedState.cSequenceNumber);

	eX0_assert(GlobalStateSequenceNumberTEST == oSequencedState.cSequenceNumber || nullptr == m_pController, "CPlayer::Position(): GlobalStateSequenceNumberTEST == oSequencedState.cSequenceNumber");

	oSequencedState.oState.fVelX = 0.0f;
	oSequencedState.oState.fVelY = 0.0f;

	SetZ(oSequencedState.oState.fZ);

	oLatestAuthStateTEST = oSequencedState;

	// TODO: Merge this with StateHistory? Maybe?
	// Reset unconfirmed commands
	oUnconfirmedMoves.clear();

	// Reset state history
	oStateHistory.clear();
	AuthState_st oAuthState;
	oAuthState.oState = oLatestAuthStateTEST;
	oAuthState.bAuthed = true;
	oStateHistory.push_front(oAuthState);
}

void CPlayer::Position(float fX, float fY, float fZ, u_char cLastCommandSequenceNumber)
{
	SequencedState_st oSequencedState;
	oSequencedState.oState.fX = fX;
	oSequencedState.oState.fY = fY;
	oSequencedState.oState.fZ = fZ;
	oSequencedState.cSequenceNumber = cLastCommandSequenceNumber;

	Position(oSequencedState);
}

float CPlayer::GetVelocity()
{
	// is the player dead?
	if (fHealth <= 0.0f) return 0.0f;

	auto State = GetStateInPast(0, &g_pGameSession->LogicTimer());
	auto State0 = GetStateInPast(1.0 / g_cCommandRate, &g_pGameSession->LogicTimer());
	Vector2 Velocity(State.fX - State0.fX, State.fY - State0.fY);
	return Velocity.Length();
}

void CPlayer::Rotate(float fAmount)
{
	SetZ(fZ + fAmount);
}

void CPlayer::SetZ(float fValue)
{
	fZ = fValue;
	while (fZ >= Math::TWO_PI) fZ -= Math::TWO_PI;
	while (fZ < 0.0) fZ += Math::TWO_PI;
}

float CPlayer::GetZ()
{
	//return fZ + Math::Sin(g_pGameSession->LogicTimer().GetTime() * 10.0f) * GetVelocity() * GetVelocity() / 3.5f * 0.0075f;
	return fZ;
}

//void CPlayer::SetLastLatency(u_short nLastLatency) { eX0_assert(pConnection == NULL || dynamic_cast<RemoteClientConnection *>(pConnection), "called CPlayer::SetLastLatency(u_short) when pConnection != NULL"); m_nLastLatency = nLastLatency; }
//u_short CPlayer::GetLastLatency() const { return (pConnection == NULL || dynamic_cast<RemoteClientConnection *>(pConnection) ? m_nLastLatency : pConnection->GetLastLatency()); }

void CPlayer::Render()
{
#ifdef EX0_CLIENT
	// Player Shadow
#define SHADOW_TYPE 0		// Black Shadows
//#define SHADOW_TYPE 1		// Coloured Shadows
	if (this == pLocalPlayer)
		RenderOffsetCamera(true);
	else {
		RenderOffsetCamera(false);
		State_st oRenderState = GetRenderState();
		glTranslatef(oRenderState.fX, oRenderState.fY, 0);
	}
	glEnable(GL_BLEND);
	glShadeModel(GL_SMOOTH);
	glBegin(GL_TRIANGLE_FAN);
#if SHADOW_TYPE == 0
		glColor4d(0, 0, 0, 0.3);
#elif SHADOW_TYPE == 1
		glColor4d(!IsDead() && m_nTeam == 0 ? 1 : 0, 0, !IsDead() && m_nTeam == 1 ? 1 : 0, 0.3);
#endif
		glVertex2d(0, 0);
#if SHADOW_TYPE == 0
		glColor4d(0, 0, 0, 0);
#elif SHADOW_TYPE == 1
		glColor4d(!IsDead() && m_nTeam == 0 ? 1 : 0, 0, !IsDead() && m_nTeam == 1 ? 1 : 0, 0);
#endif
		const int nSlices = 16;
		const double dShadowRadius = PLAYER_HALF_WIDTH * 1.75;
		for (int nSlice = 0; nSlice <= nSlices; ++nSlice)
		{
			glVertex2d(Math::Cos(Math::TWO_PI * nSlice / nSlices) * dShadowRadius, Math::Sin(Math::TWO_PI * nSlice / nSlices) * dShadowRadius);
		}
	glEnd();
	glShadeModel(GL_FLAT);
	glDisable(GL_BLEND);

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
	if (this == pLocalPlayer)
	// local player
	{
		OglUtilsSetMaskingMode(NO_MASKING_MODE);
		RenderOffsetCamera(true);
		gluPartialDisk(oQuadricObj, 6, 8, 10, 1, 30.0, 300.0);

		// Render the weapon
		oWeapons.Render();
		OglUtilsSetMaskingMode(WITH_MASKING_MODE);

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
			State_st oRenderState = GetRenderState();
			glTranslatef(oRenderState.fX, oRenderState.fY, 0);
			glRotatef(oRenderState.fZ * Math::RAD_TO_DEG, 0, 0, -1);
			gluPartialDisk(oQuadricObj, 6, 8, 10, 1, 30.0, 300.0);

			// Render the weapon
			oWeapons.Render();

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

void CPlayer::RenderInsideMask()
{
	if (bStencilOperations && !IsDead() && pLocalPlayer != NULL && pLocalPlayer->GetTeam() != 2) {
		OglUtilsSetMaskingMode(RENDER_TO_MASK_MODE);
		OglUtilsSetMaskValue(0);
		gluDisk(oQuadricObj, 0, 8, 12, 1);
		OglUtilsSetMaskingMode(WITH_MASKING_MODE);
	}
}

void CPlayer::PushStateHistory(AuthState_st oAuthState)
{
	//printf("PushStateHistory(): %d (%c)\n", oAuthState.oState.cSequenceNumber, oAuthState.bAuthed ? 'a' : 'p');

	// TODO: Any way to fix/improve the if cast<char>() > 0 or <= 0) thing?
	if (static_cast<char>(oAuthState.oState.cSequenceNumber - oStateHistory.front().oState.cSequenceNumber) > 0)
	{
		if (oStateHistory.size() >= 1000) oStateHistory.pop_back();
		oStateHistory.push_front(oAuthState);
	}
	else
	{
		std::list<AuthState_st>::iterator it1 = oStateHistory.begin();
		for (; it1 != oStateHistory.end(); ++it1)
		{
			if (it1->bAuthed)
				break;
			else if (it1->oState.cSequenceNumber == oAuthState.oState.cSequenceNumber)
			{
				/// Ensured: false == it1->bAuthed

				if (true == oAuthState.bAuthed)
				{
					// All older unconfirmed commands are now superseded by this update, remove them
					while (oUnconfirmedMoves.begin() != oAuthState.oState.cSequenceNumber)
						oUnconfirmedMoves.pop_front();

					// Check if our prediction was close, if not, we'll need to redo all predictions since then
					bool bPredictionWasClose = it1->oState.oState.CloseTo(oAuthState.oState.oState);

					// Update the old non-authed state with the new authed one (except keep the locally-predicted velocity)
					oAuthState.oState.oState.fVelX = it1->oState.oState.fVelX;
					oAuthState.oState.oState.fVelY = it1->oState.oState.fVelY;
					*it1 = oAuthState;

					if (false == bPredictionWasClose)
					// Redo all predictions since the new latest auth update
					{
						if (oUnconfirmedMoves.empty()) printf("Snapping-A to server's position (no unconfirmed commands left).\n");
						else printf("Snapping-B to server's position (need to do rewind&replay for %d cmds).\n", oUnconfirmedMoves.size());

						// Redo each remaining unconfirmed command
						for (IndexedCircularBuffer<Command_st, u_char>::iterator it2 = oUnconfirmedMoves.begin(); it2 != oUnconfirmedMoves.end(); ++it2)
						{
							SequencedState_st oStartingState = it1->oState;
							SequencedCommand_st oCommand;
							oCommand.oCommand = oUnconfirmedMoves[it2];
							oCommand.cSequenceNumber = it2;

							--it1;
							it1->oState = PhysicsTickTEST(oStartingState, oCommand);
						}

						eX0_assert(it1 == oStateHistory.begin(), "after re-doing the predictions, we ended up at the right place");
					}
				}
				else
				{
					eX0_assert(false, "pushing duplicate non-authed state sn #" + itos(oAuthState.oState.cSequenceNumber));
				}

				break;
			}
		}
		eX0_assert(it1 != oStateHistory.end(), "PushStateHistory(): couldn't find an old state in oStateHistory");
	}
}

#if 0
State_st CPlayer::GetStateInPastX(float fTimeAgo)
{
	if (IsDead()) {
		return m_oDeadState;
	}

	// TODO: This needs to be worked on... need a better separation between rendering and logic timeframe/player location/etc.
	float fTicks = (pLocalPlayer != NULL) ? (pLocalPlayer->fTicks) : (float)(g_pGameSession->MainTimer().GetTime() - (g_dNextTickTime - 1.0 / g_cCommandRate));
	if (pLocalPlayer == NULL) {
		g_pGameSession->cRenderCurrentCommandSequenceNumberTEST = g_cCurrentCommandSequenceNumber;
	}

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
			State_st oToState = oStateHistory[cHistoryPoint];
			--cHistoryPoint;
			State_st oFromState = oStateHistory[cHistoryPoint];

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
		//float	fCurrentTimepoint = (u_char)(g_cCurrentCommandSequenceNumber - 1) + fTicks / fTickTime;
		float	fCurrentTimepoint = (u_char)(g_pGameSession->cRenderCurrentCommandSequenceNumberTEST - 1) + fTicks / fTickTime;
		if (fCurrentTimepoint >= 256) fCurrentTimepoint -= 256;
		float	fHistoryPoint = fCurrentTimepoint - fTimeAgo / fTickTime;
		if (fHistoryPoint < 0) fHistoryPoint += 256;
		int		nTicksAgo = (int)floorf(fTimeAgo / fTickTime - fTicks / fTickTime) + 1 + (u_char)(g_cCurrentCommandSequenceNumber - g_pGameSession->cRenderCurrentCommandSequenceNumberTEST);
		char	cLastKnownTickAgo = (char)(g_cCurrentCommandSequenceNumber - oStateHistory.front().cSequenceNumber);
		std::list<SequencedState_st>::iterator oHistoryTo = oStateHistory.begin();
		std::list<SequencedState_st>::iterator oHistoryFrom = oStateHistory.begin(); ++oHistoryFrom;
		//if ((char)((u_char)floorf(fHistoryPoint) - oStateHistory.front().cSequenceNumber) >= 5)
		if (cLastKnownTickAgo > nTicksAgo)
		// Extrapolate into the future
		{
			//printf("future %d %d\n", cLastKnownTickAgo, nTicksAgo);
			/*fHistoryX = GetIntX();
			fHistoryY = GetIntY();
			fHistoryZ = GetZ();*/

			State_st oStateTo = oHistoryTo->oState;
			State_st oStateFrom = oHistoryFrom->oState;

			float dHistoryTicks = fHistoryPoint - oHistoryFrom->oState.cSequenceNumber;
			if (dHistoryTicks < 0) dHistoryTicks += 256;
			if (dHistoryTicks > ((u_char)(oHistoryTo->oState.cSequenceNumber - oHistoryFrom->oState.cSequenceNumber) + fTicks + kfMaxExtrapolate) / fTickTime) {
				// Max forward extrapolation time
				dHistoryTicks = ((u_char)(oHistoryTo->oState.cSequenceNumber - oHistoryFrom->oState.cSequenceNumber) + fTicks + kfMaxExtrapolate) / fTickTime;

				printf("Exceeding max EXTERP time (player %d).\n", iID);
				printf("cur state = %d; last known state = %d\n", g_cCurrentCommandSequenceNumber, oStateHistory.front().cSequenceNumber);
				printf("cLastKnownTickAgo %d > nTicksAgo %d\n", cLastKnownTickAgo, nTicksAgo);
			}// else
			//	printf("Exterping into the future (player %d).\n", iID);

			u_char cHistoryTickTime = (u_char)(oHistoryTo->oState.cSequenceNumber - oHistoryFrom->oState.cSequenceNumber);
			fHistoryX = oStateFrom.fX + (oStateTo.fX - oStateFrom.fX) * dHistoryTicks / cHistoryTickTime;
			fHistoryY = oStateFrom.fY + (oStateTo.fY - oStateFrom.fY) * dHistoryTicks / cHistoryTickTime;

			float fDiffZ = oStateTo.fZ - oStateFrom.fZ;
			if (fDiffZ >= Math::PI) fDiffZ -= Math::TWO_PI;
			if (fDiffZ < -Math::PI) fDiffZ += Math::TWO_PI;
			if (dHistoryTicks > 1.0f) dHistoryTicks = sqrtf(dHistoryTicks);
			fHistoryZ = oStateFrom.fZ + fDiffZ * dHistoryTicks / cHistoryTickTime;
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
				//(char)((u_char)floorf(fHistoryPoint) - oHistoryFrom->oState.cSequenceNumber) < 0) {
				nTicksAgo >= (cPeriodLength = (u_char)(oHistoryTo->oState.cSequenceNumber - oHistoryFrom->oState.cSequenceNumber))) {
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
				eX0_assert(oHistoryFrom->oState.cSequenceNumber != oHistoryTo->oState.cSequenceNumber, "dup!");
				if (oHistoryFrom->oState.cSequenceNumber < oHistoryTo->oState.cSequenceNumber)
				{
					/*eX0_assert(oHistoryFrom->oState.cSequenceNumber <= fHistoryPoint
						&& fHistoryPoint <= oHistoryTo->oState.cSequenceNumber,
						"not in seq");*/
					if (!(oHistoryFrom->oState.cSequenceNumber <= fHistoryPoint
						&& fHistoryPoint <= oHistoryTo->oState.cSequenceNumber))
					printf("%d <= %f <= %d\n", oHistoryFrom->oState.cSequenceNumber,
											   fHistoryPoint,
											   oHistoryTo->oState.cSequenceNumber);
					//else
					//	printf("%d <= %f <= %d ALL OK\n", oHistoryFrom->oState.cSequenceNumber,
					//						   fHistoryPoint,
					//						   oHistoryTo->oState.cSequenceNumber);
				}

				State_st oStateTo = oHistoryTo->oState;
				State_st oStateFrom = oHistoryFrom->oState;

				//float dHistoryTicks = fHistoryPoint - oHistoryFrom->oState.cSequenceNumber;
				float dHistoryTicks = fHistoryPoint - oHistoryFrom->oState.cSequenceNumber;
				if (dHistoryTicks < 0) dHistoryTicks += 256;
				u_char cHistoryTickTime = (u_char)(oHistoryTo->oState.cSequenceNumber - oHistoryFrom->oState.cSequenceNumber);
				//eX0_assert(dHistoryTicks < cHistoryTickTime, "dHistoryTicks not in range");
				if (!(dHistoryTicks <= cHistoryTickTime))
					printf("dHistoryTicks not in range: %f\n", dHistoryTicks / cHistoryTickTime);
				//else
				//	printf("dHistoryTicks IS OK: %f\n", dHistoryTicks / cHistoryTickTime);
				fHistoryX = oStateFrom.fX + (oStateTo.fX - oStateFrom.fX) * dHistoryTicks / cHistoryTickTime;
				fHistoryY = oStateFrom.fY + (oStateTo.fY - oStateFrom.fY) * dHistoryTicks / cHistoryTickTime;

				float fDiffZ = oStateTo.fZ - oStateFrom.fZ;
				if (fDiffZ >= Math::PI) fDiffZ -= Math::TWO_PI;
				if (fDiffZ < -Math::PI) fDiffZ += Math::TWO_PI;
				fHistoryZ = oStateFrom.fZ + fDiffZ * dHistoryTicks / cHistoryTickTime;
			}
		}
	}

	State_st oState;
	oState.fX = fHistoryX;
	oState.fY = fHistoryY;
	oState.fZ = fHistoryZ;
	return oState;
}
#endif // 0

void CPlayer::UpdateRenderState()
{
	if (GetTeam() == 2) return;

	if (!IsDead()) {
		//m_oRenderState = GetStateInPast(0);
		if (nullptr != dynamic_cast<LocalController *>(m_pController))
			m_oRenderState = GetStateInPast(0);
		else
			m_oRenderState = GetStateInPast(kfInterpolate);
	} else
		m_oRenderState = m_oDeadState;
}

const State_st CPlayer::GetRenderState()
{
	if (nullptr != dynamic_cast<LocalController *>(m_pController))
	{
		m_oRenderState.fZ = GetZ();
	}

	return m_oRenderState;
}

// TODO: Rewrite this func...
State_st CPlayer::GetStateAtTime(double dTime)
{
	eX0_assert(dTime >= 0, "dTime can only be >= 0");

	if (IsDead()) {
		return m_oDeadState;
	}

	State_st oStateInPast;

	if (oStateHistory.size() == 0) {
		oStateInPast.fX = 0;
		oStateInPast.fY = 0;
		oStateInPast.fZ = 0;
		eX0_assert(false, "something's wrong, cuz oStateHistory.size() is == 0!");
	} else if (oStateHistory.size() == 1) {
		oStateInPast = oStateHistory.front().oState.oState;
	} else {
		double dCurrentTimepoint = dTime / (256.0 / g_cCommandRate);
		dCurrentTimepoint -= static_cast<uint32>(dCurrentTimepoint);
		dCurrentTimepoint *= 256;
eX0_assert(dCurrentTimepoint >= 0, "dCurrentTimepoint >= 0");
eX0_assert(dCurrentTimepoint < 256, "dCurrentTimepoint < 256");
		uint8 cCurrentTimepoint = static_cast<uint8>(dCurrentTimepoint);
		//printf("dCurrentTimepoint = %f, cCurrentTimepoint = %d\n", dCurrentTimepoint, cCurrentTimepoint);
		/*double dTicks = dCurrentTimepoint - cCurrentTimepoint;
eX0_assert(dTicks >= 0, "dTicks >= 0");
eX0_assert(dTicks < 1, "dTicks < 1");*/
		double dTickTime = 1.0 / g_cCommandRate;
		//int		nTicksAgo = (int)floor(dTimeAgo * g_cCommandRate - fTicks * g_cCommandRate) + 1 + (u_char)(g_cCurrentCommandSequenceNumber - g_pGameSession->cRenderCurrentCommandSequenceNumberTEST);
		//int		nTicksAgo = static_cast<int>(dTimeAgo * g_cCommandRate - dTicks + 1);
/*if(dTimeAgo * g_c >>> dTicks)
then nTicksAgo = 123;
if(dTimeAgo * g_c > dTicks)
then nTicksAgo = 1;
if(dTimeAgo * g_c == dTicks)
then nTicksAgo = 1;
else if(dTimeAgo * g_c < dTicks)
then nTicksAgo = 0;*/
		//char	cLastKnownTickAgo = (char)(cCurrentTimepoint + 1 - oStateHistory.front().oState.cSequenceNumber);
if (glfwGetKey('I') == GLFW_PRESS){
DumpStateHistory(oStateHistory);
}
//static int c = 30;
//if(GetTeam() == 0 && c-- > 0)DumpStateHistory(oStateHistory);
		std::list<AuthState_st>::iterator oHistoryTo = oStateHistory.begin();
		std::list<AuthState_st>::iterator oHistoryFrom = oStateHistory.begin(); ++oHistoryFrom;
		//if ((char)((u_char)floorf(fHistoryPoint) - oStateHistory.front().cSequenceNumber) >= 5)
		//if (cLastKnownTickAgo > nTicksAgo)
		// TODO: Any way to fix/improve the if cast<char>() > 0 or <= 0) thing?
		if (static_cast<char>(cCurrentTimepoint - oStateHistory.front().oState.cSequenceNumber) >= 0)
		// Extrapolate into the future
		{
			if (static_cast<char>(cCurrentTimepoint - oStateHistory.front().oState.cSequenceNumber) > 1)
				printf("future %f %d\n", dCurrentTimepoint, oStateHistory.front().oState.cSequenceNumber);

			State_st oStateTo = oHistoryTo->oState.oState;
			State_st oStateFrom = oHistoryFrom->oState.oState;
			uint8 cHistoryTickTime = static_cast<uint8>(oHistoryTo->oState.cSequenceNumber - oHistoryFrom->oState.cSequenceNumber);

			double dHistoryTicks = dCurrentTimepoint - oHistoryFrom->oState.cSequenceNumber;
			if (dHistoryTicks < 0) dHistoryTicks += 256;
			if (dHistoryTicks > (cHistoryTickTime + kfMaxExtrapolate / dTickTime)) {
				// Max forward extrapolation time
				dHistoryTicks = cHistoryTickTime + kfMaxExtrapolate / dTickTime;

				printf("Exceeding max EXTERP time (player %d).\n", iID);
				printf("cur state = %d; last known state = %d\n", g_pGameSession->GlobalStateSequenceNumberTEST, oStateHistory.front().oState.cSequenceNumber);
			}

			oStateInPast.fX = oStateFrom.fX + (oStateTo.fX - oStateFrom.fX) * (float)dHistoryTicks / cHistoryTickTime;
			oStateInPast.fY = oStateFrom.fY + (oStateTo.fY - oStateFrom.fY) * (float)dHistoryTicks / cHistoryTickTime;

			float fDiffZ = oStateTo.fZ - oStateFrom.fZ;
			if (fDiffZ >= Math::PI) fDiffZ -= Math::TWO_PI;
			if (fDiffZ < -Math::PI) fDiffZ += Math::TWO_PI;
			if (dHistoryTicks > 1.0) dHistoryTicks = std::sqrt(dHistoryTicks);
			oStateInPast.fZ = oStateFrom.fZ + fDiffZ * (float)dHistoryTicks / cHistoryTickTime;
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
			while (oHistoryFrom != oStateHistory.end() &&
				//(char)((u_char)floorf(fHistoryPoint) - oHistoryFrom->oState.cSequenceNumber) < 0) {
				//nTicksAgo >= (cPeriodLength = (u_char)(oHistoryTo->oState.cSequenceNumber - oHistoryFrom->oState.cSequenceNumber)))
				static_cast<char>(oHistoryFrom->oState.cSequenceNumber - cCurrentTimepoint) > 0)
			{
				++oHistoryTo;
				++oHistoryFrom;
			}

			if (oHistoryFrom == oStateHistory.end()) {
				oStateInPast.fX = oStateHistory.back().oState.oState.fX;
				oStateInPast.fY = oStateHistory.back().oState.oState.fY;
				oStateInPast.fZ = oStateHistory.back().oState.oState.fZ;
			} else {
				eX0_assert(oHistoryFrom->oState.cSequenceNumber != oHistoryTo->oState.cSequenceNumber, "dup!");
				if (oHistoryFrom->oState.cSequenceNumber < oHistoryTo->oState.cSequenceNumber)
				{
					/*eX0_assert(oHistoryFrom->oState.cSequenceNumber <= fHistoryPoint
						&& fHistoryPoint <= oHistoryTo->oState.cSequenceNumber,
						"not in seq");*/
					if (!(oHistoryFrom->oState.cSequenceNumber <= dCurrentTimepoint
						&& dCurrentTimepoint <= oHistoryTo->oState.cSequenceNumber))
					printf("%d <= %f <= %d\n", oHistoryFrom->oState.cSequenceNumber,
											   dCurrentTimepoint,
											   oHistoryTo->oState.cSequenceNumber);
				}

				State_st oStateTo = oHistoryTo->oState.oState;
				State_st oStateFrom = oHistoryFrom->oState.oState;
				uint8 cHistoryTickTime = static_cast<uint8>(oHistoryTo->oState.cSequenceNumber - oHistoryFrom->oState.cSequenceNumber);

				double dHistoryTicks = dCurrentTimepoint - oHistoryFrom->oState.cSequenceNumber;
				if (dHistoryTicks < 0) dHistoryTicks += 256;
				if (!(dHistoryTicks <= cHistoryTickTime))
					printf("dHistoryTicks not in range: %f\n", dHistoryTicks / cHistoryTickTime);

				oStateInPast.fX = oStateFrom.fX + (oStateTo.fX - oStateFrom.fX) * (float)dHistoryTicks / cHistoryTickTime;
				oStateInPast.fY = oStateFrom.fY + (oStateTo.fY - oStateFrom.fY) * (float)dHistoryTicks / cHistoryTickTime;

				float fDiffZ = oStateTo.fZ - oStateFrom.fZ;
				if (fDiffZ >= Math::PI) fDiffZ -= Math::TWO_PI;
				if (fDiffZ < -Math::PI) fDiffZ += Math::TWO_PI;
				oStateInPast.fZ = oStateFrom.fZ + fDiffZ * (float)dHistoryTicks / cHistoryTickTime;
			}
		}
	}

	return oStateInPast;
}

#if 0
State_st CPlayer::GetStateInPast(double dTimeAgo, GameTimer * pTimer)
{
if (dTimeAgo < -0.001) printf("dTimeAgo=%f is negative, weird? ", dTimeAgo);
	double dCurrentTime;
	if (nullptr == pTimer) dCurrentTime = g_pGameSession->MainTimer().GetTime();
	else dCurrentTime = pTimer->GetTime();

	return GetStateAtTime(dCurrentTime - dTimeAgo);
}
#else
State_st CPlayer::GetStateInPast(double dTimeAgo, GameTimer * pTimer)
{
if (dTimeAgo < -0.001) printf("dTimeAgo=%f is negative, weird? ", dTimeAgo);
	double dCurrentTime;
	if (nullptr == pTimer) dCurrentTime = g_pGameSession->MainTimer().GetTime();
	else dCurrentTime = pTimer->GetTime();
	
	double dTime = dCurrentTime - dTimeAgo;
	
	eX0_assert(dTime >= 0, "dTime can only be >= 0");

	if (IsDead()) {
		return m_oDeadState;
	}

	State_st oStateInPast;

	if (oStateHistory.size() == 0) {
		oStateInPast.fX = 0;
		oStateInPast.fY = 0;
		oStateInPast.fZ = 0;
		eX0_assert(false, "something's wrong, cuz oStateHistory.size() is == 0!");
	} else if (oStateHistory.size() == 1) {
		oStateInPast = oStateHistory.front().oState.oState;
	} else {
		double dCurrentTimepoint = dTime / (256.0 / g_cCommandRate);
		dCurrentTimepoint -= static_cast<uint32>(dCurrentTimepoint);
		dCurrentTimepoint *= 256;
eX0_assert(dCurrentTimepoint >= 0, "dCurrentTimepoint >= 0");
eX0_assert(dCurrentTimepoint < 256, "dCurrentTimepoint < 256");
		uint8 cCurrentTimepoint = static_cast<uint8>(dCurrentTimepoint);
		//printf("dCurrentTimepoint = %f, cCurrentTimepoint = %d\n", dCurrentTimepoint, cCurrentTimepoint);
		/*double dTicks = dCurrentTimepoint - cCurrentTimepoint;
eX0_assert(dTicks >= 0, "dTicks >= 0");
eX0_assert(dTicks < 1, "dTicks < 1");*/
		double dTickTime = 1.0 / g_cCommandRate;
		//int		nTicksAgo = (int)floor(dTimeAgo * g_cCommandRate - fTicks * g_cCommandRate) + 1 + (u_char)(g_cCurrentCommandSequenceNumber - g_pGameSession->cRenderCurrentCommandSequenceNumberTEST);
		//int		nTicksAgo = static_cast<int>(dTimeAgo * g_cCommandRate - dTicks + 1);
/*if(dTimeAgo * g_c >>> dTicks)
then nTicksAgo = 123;
if(dTimeAgo * g_c > dTicks)
then nTicksAgo = 1;
if(dTimeAgo * g_c == dTicks)
then nTicksAgo = 1;
else if(dTimeAgo * g_c < dTicks)
then nTicksAgo = 0;*/
		//char	cLastKnownTickAgo = (char)(cCurrentTimepoint + 1 - oStateHistory.front().oState.cSequenceNumber);
if (glfwGetKey('I') == GLFW_PRESS){
DumpStateHistory(oStateHistory);
}
//static int c = 30;
//if(GetTeam() == 0 && c-- > 0)DumpStateHistory(oStateHistory);
		std::list<AuthState_st>::iterator oHistoryTo = oStateHistory.begin();
		std::list<AuthState_st>::iterator oHistoryFrom = oStateHistory.begin(); ++oHistoryFrom;
		//if ((char)((u_char)floorf(fHistoryPoint) - oStateHistory.front().cSequenceNumber) >= 5)
		//if (cLastKnownTickAgo > nTicksAgo)
		// TODO: Any way to fix/improve the if cast<char>() > 0 or <= 0) thing?
		if (static_cast<char>(cCurrentTimepoint - oStateHistory.front().oState.cSequenceNumber) >= 0)
		// Extrapolate into the future
		{
			if (static_cast<char>(cCurrentTimepoint - oStateHistory.front().oState.cSequenceNumber) > 1)
				printf("future %f %d\n", dCurrentTimepoint, oStateHistory.front().oState.cSequenceNumber);

			State_st oStateTo = oHistoryTo->oState.oState;
			State_st oStateFrom = oHistoryFrom->oState.oState;
			uint8 cHistoryTickTime = static_cast<uint8>(oHistoryTo->oState.cSequenceNumber - oHistoryFrom->oState.cSequenceNumber);

			double dHistoryTicks = dCurrentTimepoint - oHistoryFrom->oState.cSequenceNumber;
			if (dHistoryTicks < 0) dHistoryTicks += 256;
			if (dHistoryTicks > (cHistoryTickTime + kfMaxExtrapolate / dTickTime)) {
				// Max forward extrapolation time
				dHistoryTicks = cHistoryTickTime + kfMaxExtrapolate / dTickTime;

				printf("Exceeding max EXTERP time (player %d).\n", iID);
				printf("cur state = %d; last known state = %d\n", g_pGameSession->GlobalStateSequenceNumberTEST, oStateHistory.front().oState.cSequenceNumber);
			}

			oStateInPast.fX = oStateFrom.fX + (oStateTo.fX - oStateFrom.fX) * (float)dHistoryTicks / cHistoryTickTime;
			oStateInPast.fY = oStateFrom.fY + (oStateTo.fY - oStateFrom.fY) * (float)dHistoryTicks / cHistoryTickTime;

			float fDiffZ = oStateTo.fZ - oStateFrom.fZ;
			if (fDiffZ >= Math::PI) fDiffZ -= Math::TWO_PI;
			if (fDiffZ < -Math::PI) fDiffZ += Math::TWO_PI;
			if (dHistoryTicks > 1.0) dHistoryTicks = std::sqrt(dHistoryTicks);
			oStateInPast.fZ = oStateFrom.fZ + fDiffZ * (float)dHistoryTicks / cHistoryTickTime;
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
			while (oHistoryFrom != oStateHistory.end() &&
				//(char)((u_char)floorf(fHistoryPoint) - oHistoryFrom->oState.cSequenceNumber) < 0) {
				//nTicksAgo >= (cPeriodLength = (u_char)(oHistoryTo->oState.cSequenceNumber - oHistoryFrom->oState.cSequenceNumber)))
				static_cast<char>(oHistoryFrom->oState.cSequenceNumber - cCurrentTimepoint) > 0)
			{
				++oHistoryTo;
				++oHistoryFrom;
			}

			if (oHistoryFrom == oStateHistory.end()) {
				oStateInPast.fX = oStateHistory.back().oState.oState.fX;
				oStateInPast.fY = oStateHistory.back().oState.oState.fY;
				oStateInPast.fZ = oStateHistory.back().oState.oState.fZ;
			} else {
				eX0_assert(oHistoryFrom->oState.cSequenceNumber != oHistoryTo->oState.cSequenceNumber, "dup!");
				if (oHistoryFrom->oState.cSequenceNumber < oHistoryTo->oState.cSequenceNumber)
				{
					/*eX0_assert(oHistoryFrom->oState.cSequenceNumber <= fHistoryPoint
						&& fHistoryPoint <= oHistoryTo->oState.cSequenceNumber,
						"not in seq");*/
					if (!(oHistoryFrom->oState.cSequenceNumber <= dCurrentTimepoint
						&& dCurrentTimepoint <= oHistoryTo->oState.cSequenceNumber))
					printf("%d <= %f <= %d\n", oHistoryFrom->oState.cSequenceNumber,
											   dCurrentTimepoint,
											   oHistoryTo->oState.cSequenceNumber);
				}

				State_st oStateTo = oHistoryTo->oState.oState;
				State_st oStateFrom = oHistoryFrom->oState.oState;
				uint8 cHistoryTickTime = static_cast<uint8>(oHistoryTo->oState.cSequenceNumber - oHistoryFrom->oState.cSequenceNumber);

				double dHistoryTicks = dCurrentTimepoint - oHistoryFrom->oState.cSequenceNumber;
				if (dHistoryTicks < 0) dHistoryTicks += 256;
				if (!(dHistoryTicks <= cHistoryTickTime))
					printf("dHistoryTicks not in range: %f\n", dHistoryTicks / cHistoryTickTime);

				oStateInPast.fX = oStateFrom.fX + (oStateTo.fX - oStateFrom.fX) * (float)dHistoryTicks / cHistoryTickTime;
				oStateInPast.fY = oStateFrom.fY + (oStateTo.fY - oStateFrom.fY) * (float)dHistoryTicks / cHistoryTickTime;

				float fDiffZ = oStateTo.fZ - oStateFrom.fZ;
				if (fDiffZ >= Math::PI) fDiffZ -= Math::TWO_PI;
				if (fDiffZ < -Math::PI) fDiffZ += Math::TWO_PI;
				oStateInPast.fZ = oStateFrom.fZ + fDiffZ * (float)dHistoryTicks / cHistoryTickTime;
			}
		}
	}

	return oStateInPast;
}
#endif

void CPlayer::RenderInPast(double dTimeAgo)
{
	//eX0_assert(false, "i shouldn't be using RenderInPast anymore..");

	State_st oState = GetStateInPast(dTimeAgo);

	// select player color
	if (IsDead())
		glColor3f(0.1f, 0.1f, 0.1f);
	else if (m_nTeam == 0)
		glColor3f(1, 0, 0);
	else if (m_nTeam == 1)
		glColor3f(0, 0, 1);
	else
		glColor3f(0, 1, 0);

	if (this == pLocalPlayer)
		glColor3f(0.5f, 0.5f, 0.5f);

	if (bWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPushMatrix();
	if (this == pLocalPlayer) OglUtilsSetMaskingMode(NO_MASKING_MODE);
	RenderOffsetCamera(false);
	glTranslatef(oState.fX, oState.fY, 0);
	glRotatef(oState.fZ * Math::RAD_TO_DEG, 0, 0, -1);
	gluPartialDisk(oQuadricObj, 6, 8, 10, 1, 30.0, 300.0);
	glBegin(GL_QUADS);
		glVertex2i(-1, 11);
		glVertex2i(-1, 3);
		glVertex2i(1, 3);
		glVertex2i(1, 11);
	glEnd();
	if (this == pLocalPlayer) OglUtilsSetMaskingMode(WITH_MASKING_MODE);
	glPopMatrix();
	if (bWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void CPlayer::InitWeapons()
{
	oWeapons.Reset();
	oWeapons.AddWeapon(1);
	oWeapons.AddWeapon(2);
}

string & CPlayer::GetName(void) { return sName; }
void CPlayer::SetName(string & sNewName) {
	if (sNewName.length() > 0) sName = sNewName.substr(0, 32);
	else sName = "Unnamed Player";
}

void CPlayer::Add(CPlayer * pPlayer)
{
	/*printf("Before Add():"); for (std::vector<CPlayer *>::const_iterator cit1 = m_oPlayers.begin(); cit1 < m_oPlayers.end(); ++cit1) {
		printf(" %p", *cit1);
	} printf("\n");*/
	m_oPlayers.at(pPlayer->iID = NextFreePlayerId()) = pPlayer;
	/*printf("After Add():"); for (std::vector<CPlayer *>::const_iterator cit1 = m_oPlayers.begin(); cit1 < m_oPlayers.end(); ++cit1) {
		printf(" %p", *cit1);
	} printf("\n");*/
}

void CPlayer::Add(CPlayer * pPlayer, uint8 nPlayerId)
{
	/*printf("Before Add(int):"); for (std::vector<CPlayer *>::const_iterator cit1 = m_oPlayers.begin(); cit1 < m_oPlayers.end(); ++cit1) {
		printf(" %p", *cit1);
	} printf("\n");*/
	if (nPlayerId >= m_oPlayers.size())
		m_oPlayers.resize(nPlayerId + 1);
	else if (m_oPlayers.at(nPlayerId) != NULL) throw 1;
	m_oPlayers.at(pPlayer->iID = nPlayerId) = pPlayer;
	/*printf("After Add(int):"); for (std::vector<CPlayer *>::const_iterator cit1 = m_oPlayers.begin(); cit1 < m_oPlayers.end(); ++cit1) {
		printf(" %p", *cit1);
	} printf("\n");*/
}

uint8 CPlayer::NextFreePlayerId()
{
	// Look for a free slot
	for (std::vector<CPlayer *>::const_iterator cit1 = m_oPlayers.begin(); cit1 < m_oPlayers.end(); ++cit1) {
		if (*cit1 == NULL)
			return static_cast<uint8>(cit1 - m_oPlayers.begin());
	}

	// No free slots in the current array, so extend it by 1
	m_oPlayers.push_back(NULL);
	return static_cast<uint8>(m_oPlayers.size() - 1);
}

u_int CPlayer::GetPlayerCount()
{
	return m_nPlayerCount;
}

void CPlayer::Remove(CPlayer * pPlayer)
{
	for (std::vector<CPlayer *>::iterator it1 = m_oPlayers.begin(); it1 < m_oPlayers.end(); ++it1) {
		if (*it1 == pPlayer) {
			*it1 = NULL;
			return;
		}
	}
}

void CPlayer::DeleteAll()
{
	for (std::vector<CPlayer *>::iterator it1 = m_oPlayers.begin(); it1 < m_oPlayers.end(); ++it1) {
		if (*it1 != NULL) {
printf("ERROR: Deleting player %p id=%d name='%s' in CPlayer::DeleteAll()...\n", (*it1), (*it1)->iID, (*it1)->GetName().c_str());
			delete *it1;
			*it1 = NULL;
		}
	}
	m_oPlayers.clear();
}

// Returns a player
CPlayer * PlayerGet(uint8 nPlayerId)
{
	if (nPlayerId >= CPlayer::m_oPlayers.size()) return NULL;
	else return CPlayer::m_oPlayers.at(nPlayerId);
}

void PlayerTick()
{
#ifdef EX0_CLIENT
	for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
	{
		if (PlayerGet(nPlayer) != NULL) {
			//PlayerGet(nPlayer)->Tick();
			PlayerGet(nPlayer)->ProcessUpdates();
		}
	}
#endif // EX0_CLIENT
}
