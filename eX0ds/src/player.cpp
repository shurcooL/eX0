#include "globals.h"

int		nPlayerCount;
CPlayer	*oPlayers[32];

//float	fPlayerTickTime;// = 0.025f;
//float	fPlayerTickTime = 0.050f;
//float	fPlayerTickTime = 0.100f;
//float	fPlayerTickTime = 1.0f;

// implementation of the player class
CPlayer::CPlayer()
{
	// init vars
	fX = 0;
	fY = 0;
	fOldX = 0;
	fOldY = 0;
	fVelX = 0;
	fVelY = 0;
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
	pClient = NULL;
}

CPlayer::~CPlayer()
{
	// nothing to do here yet
}

void CPlayer::Tick()
{
	// is the player dead?
	if (IsDead()) return;

	//oWeapons[iSelWeapon].Tick();

	//fTicks += fTimePassed;
	while (fTicks >= fTickTime)
	{
		fTicks -= fTickTime;

		// calculate player trajectory
		CalcTrajs();

		// do collision response for player
		CalcColResp();
	}

	UpdateInterpolatedPos();
}

// returns number of clips left in the selected weapon
int CPlayer::GetSelClips()
{
	//return oWeapons[iSelWeapon].GetClips();
	return 0;
}
int CPlayer::GetSelClipAmmo()
{
	//return oWeapons[iSelWeapon].GetClipAmmo();
	return 0;
}

// fire selected weapon
void CPlayer::Fire()
{
	// is the player dead?
	if (IsDead()) return;

	// fire the selected weapon
	//oWeapons[iSelWeapon].Fire();
}

// reload selected weapon
void CPlayer::Reload()
{
	// is the player dead?
	if (IsDead()) return;

	//oWeapons[iSelWeapon].Reload();
}

bool CPlayer::IsReloading()
{
	//return oWeapons[iSelWeapon].IsReloading();
	return false;
}

// buy a clip for selected weapon
void CPlayer::BuyClip()
{
	// is the player dead?
	if (IsDead()) return;

	// TODO: some kind of money system?
	// just give it to whoever
	//oWeapons[iSelWeapon].GiveClip();
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

	int			iWhichCont, iWhichVert, iCounter = 0;
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
			/*for (int iLoop1 = 0; iLoop1 < nPlayerCount; iLoop1++)
			{
				// dont check for collision with yourself
				if (iLoop1 == iID)
					continue;

				// calculate the displacement
				oVector.x = oPlayers[iID]->GetX() - oPlayers[iLoop1]->GetX();
				oVector.y = oPlayers[iID]->GetY() - oPlayers[iLoop1]->GetY();
				oDistance = oVector.Length();

				if (oDistance < oShortestDistance)
				{
					oShortestDistance = oDistance;
					oClosestPoint.x = oPlayers[iLoop1]->GetX();
					oClosestPoint.y = oPlayers[iLoop1]->GetY();
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
		for (int iLoop1 = 0; iLoop1 < nPlayerCount; iLoop1++)
		{
			// dont check for collision with yourself
			if (iLoop1 == iID)
				continue;

			// calculate the displacement
			oVector.x = oPlayers[iID]->GetX() - oPlayers[iLoop1]->GetX();
			oVector.y = oPlayers[iID]->GetY() - oPlayers[iLoop1]->GetY();
			oShortestDistance = oVector.Length();

			if (oPlayers[iLoop1]->GetVelX() || oPlayers[iLoop1]->GetVelY())
			// the other player is moving
			{
				if (oShortestDistance < PLAYER_WIDTH - PLAYER_COL_DET_TOLERANCE)
				// we're colliding with the other player
				{
					fVelXPercentage = Math::FAbs(fVelX / (Math::FAbs(fVelX) + Math::FAbs(oPlayers[iLoop1]->GetVelX())));
					fVelYPercentage = Math::FAbs(fVelY / (Math::FAbs(fVelY) + Math::FAbs(oPlayers[iLoop1]->GetVelY())));

					// move us back slightly
					oPlayers[iID]->SetX(oPlayers[iID]->GetX() + (oVector.x * PLAYER_WIDTH / oShortestDistance - oVector.x) * fVelXPercentage);
					oPlayers[iID]->SetY(oPlayers[iID]->GetY() + (oVector.y * PLAYER_WIDTH / oShortestDistance - oVector.y) * fVelYPercentage);

					// move the other player away slightly
					oPlayers[iLoop1]->SetX(oPlayers[iLoop1]->GetX() - (oVector.x * PLAYER_WIDTH / oShortestDistance - oVector.x) * (1.0 - fVelXPercentage));
					oPlayers[iLoop1]->SetY(oPlayers[iLoop1]->GetY() - (oVector.y * PLAYER_WIDTH / oShortestDistance - oVector.y) * (1.0 - fVelYPercentage));

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
}

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
	//return fZ + Math::Sin((float)glfwGetTime() * 7.5f) * GetVelocity() * 0.005f;
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

void CPlayer::Render()
{
	// select player color
	/*if (iID != iLocalPlayerID && !Trace(oPlayers[iLocalPlayerID]->GetIntX(), oPlayers[iLocalPlayerID]->GetIntY(), fIntX, fIntY))
		glColor3f(0.2, 0.5, 0.2);
	else*/ /*if (fHealth <= 0)
		glColor3f(0.1, 0.1, 0.1);
	else if (m_nTeam == 0)
		glColor3f(1, 0, 0);
	else if (m_nTeam == 1)
		glColor3f(0, 0, 1);

	if (bWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (iID == iLocalPlayerID)
	// local player
	{
		OglUtilsSetMaskingMode(NO_MASKING_MODE);
		glLoadIdentity();
		RenderOffsetCamera(true);*/
		/*glBegin(GL_LINES);
			glVertex2i(0, 11);
			glVertex2i(0, 3);
		glEnd();*/
		/*glBegin(GL_QUADS);
			glVertex2i(-1, 11);
			glVertex2i(-1, 3);
			glVertex2i(1, 3);
			glVertex2i(1, 11);
		glEnd();
		gluPartialDisk(oQuadricObj, 6, 8, 12, 1, 30.0, 300.0);

		// Draw the aiming-guide
		OglUtilsSetMaskingMode(WITH_MASKING_MODE);
		glLineWidth(1.0);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glShadeModel(GL_SMOOTH);
		glBegin(GL_LINES);
			glColor4f(0.9, 0.2, 0.1, 0.5);
			glVertex2i(0, 11);
			glColor4f(0.9, 0.2, 0.1, 0.0);
			glVertex2i(0, 600);
		glEnd();
		glShadeModel(GL_FLAT);
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
		glLineWidth(2.0);*/

		// Draw the cross section of the aiming-guide
		/*glBegin(GL_LINES);
			glColor3f(0.9, 0.2, 0.1);
			glVertex2i(-5, fAimingDistance);
			glVertex2i(5, fAimingDistance);
		glEnd();*/

		/*RenderOffsetCamera(false);
	}
	else
	// not local player
	{
		//if (Trace(oPlayers[iLocalPlayerID]->GetIntX(), oPlayers[iLocalPlayerID]->GetIntY(), fIntX, fIntY))
		//{
			glPushMatrix();
			glTranslatef(fIntX, fIntY, 0);
			glRotatef(this->GetZ() * Math::RAD_TO_DEG, 0, 0, -1);
			glBegin(GL_QUADS);
				glVertex2i(-1, 11);
				glVertex2i(-1, 3);
				glVertex2i(1, 3);
				glVertex2i(1, 11);
			glEnd();
			gluPartialDisk(oQuadricObj, 6, 8, 12, 1, 30.0, 300.0);*/

		// Draw the aiming-guide
		/*glLineWidth(1.0);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glShadeModel(GL_SMOOTH);
		glRotatef(this->GetZ() * Math::RAD_TO_DEG, 0, 0, -1);
		glBegin(GL_LINES);
			glColor4f(0.1, 0.2, 0.9, 0.5);
			glVertex2i(0, 11);
			glColor4f(0.1, 0.2, 0.9, 0.0);
			glVertex2i(0, 400);
		glEnd();
		glRotatef(this->GetZ() * Math::RAD_TO_DEG, 0, 0, 1);
		glShadeModel(GL_FLAT);
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
		glLineWidth(2.0);*/

			/*glPopMatrix();
		//}
	}
	if (bWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glBegin(GL_POINTS);
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

	// DEBUG some debug info
	/*if (1 && glfwGetKey(GLFW_KEY_TAB) && iID == iLocalPlayerID)
	{
		OglUtilsSwitchMatrix(SCREEN_SPACE_MATRIX);
		OglUtilsSetMaskingMode(NO_MASKING_MODE);
		glColor3f(1, 1, 1);

		sTempString = "x: " + ftos(fIntX);
		glLoadIdentity();
		OglUtilsPrint(0, 20, 0, false, (char *)sTempString.c_str());
		sTempString = "y: " + ftos(fIntY);
		glLoadIdentity();
		OglUtilsPrint(0, 30, 0, false, (char *)sTempString.c_str());
		sTempString = "z: " + ftos(fZ);
		glLoadIdentity();
		OglUtilsPrint(0, 40, 0, false, (char *)sTempString.c_str());
		sTempString = "velocity: " + ftos(GetVelocity());
		glLoadIdentity();
		OglUtilsPrint(150, 20, 0, false, (char *)sTempString.c_str());

		for (int iLoop1 = 0; iLoop1 < nPlayerCount; iLoop1++)
		{
			glLoadIdentity();
			sTempString = (string)"player #" + itos(iLoop1) + " health: " + ftos(oPlayers[iLoop1]->GetHealth())
				+ " vel: " + ftos(oPlayers[iLoop1]->GetVelocity());
			OglUtilsPrint(0, 50 + iLoop1 * 10, 0, false, (char *)sTempString.c_str());
		}

		sTempString = "particle array size: " + itos(iTempInt);
		glLoadIdentity();
		OglUtilsPrint(0, 50 + nPlayerCount * 10, 0, false, (char*)sTempString.c_str());

		OglUtilsSwitchMatrix(WORLD_SPACE_MATRIX);
		RenderOffsetCamera(false);
		OglUtilsSetMaskingMode(WITH_MASKING_MODE);
	}*/
}

void CPlayer::InitWeapons()
{
	/*oWeapons[0].Init(iID, 0);
	oWeapons[1].Init(iID, 0);
	oWeapons[2].Init(iID, 1);
	oWeapons[3].Init(iID, 2);*/
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
void CPlayer::SetName(string &sNewName) {
	if (sNewName.length() > 0) sName = sNewName.substr(0, 32);
	else sName = "Unnamed Player";
}

// allocate memory for all the players
void PlayerInit()
{
	for (int iLoop1 = 0; iLoop1 < 32; iLoop1++)
	{
		if (oPlayers[iLoop1] != NULL) delete oPlayers[iLoop1];

		if (iLoop1 < nPlayerCount)
		{
			oPlayers[iLoop1] = new CPlayer();
			oPlayers[iLoop1]->iID = iLoop1;
			oPlayers[iLoop1]->InitWeapons();
			oPlayers[iLoop1]->UpdateInterpolatedPos();
		}
	}
}

// Returns a player
CPlayer * PlayerGet(int nPlayerID)
{
	if (nPlayerID >= 0 &&nPlayerID < nPlayerCount) return oPlayers[nPlayerID];
	else return NULL;
}

void PlayerTick()
{
	for (int iLoop1 = 0; iLoop1 < nPlayerCount; iLoop1++)
	{
		oPlayers[iLoop1]->Tick();
	}

	// update local player interpolated pos
	//oPlayers[iLocalPlayerID]->UpdateInterpolatedPos();
}

int PlayerGetFreePlayerID()
{
	for (int iLoop1 = 0; iLoop1 < nPlayerCount; ++iLoop1)
	{
		// Check if we've found a free player ID
		if (oPlayers[iLoop1]->pClient == NULL)
		{
			return iLoop1;
		}
	}

	return -1;		// No free player IDs; server is full
}
