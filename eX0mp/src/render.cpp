#include "globals.h"

int		iCameraType = 0;

CHudMessageQueue	*pChatMessages = NULL;

// render the static scene
void RenderStaticScene()
{
	u_int nLoop1, nLoop2;

	OglUtilsSwitchMatrix(WORLD_SPACE_MATRIX);
	RenderOffsetCamera(false);

	// DEBUG: Render the ground using one of the two triangulations (gpc or PolyBoolean)
	if (bUseDefaultTriangulation)
	{
		// fill in the ground
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, oTextureIDs.iFloor);
		glColor3f(0.9f, 0.9f, 0.9f);
		for (nLoop1 = 0; nLoop1 < (u_int)oTristripLevel.num_strips; nLoop1++)
		{
			if (bWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			//glColor3f(1.0, 0.0, 0.0);
			glLineWidth(1.0f);
			//glShadeModel(GL_SMOOTH);
			glBegin(GL_TRIANGLE_STRIP);
			//glBegin(GL_LINE_STRIP);
				for (nLoop2 = 0; nLoop2 < (u_int)oTristripLevel.strip[nLoop1].num_vertices; nLoop2++)
				{
					//glColor3f(1.0 - (nLoop2 % 2 * 1), (nLoop2 % 2 * 1), 0.0);
					glTexCoord2f((float)oTristripLevel.strip[nLoop1].vertex[nLoop2].x / 256.0f, (float)oTristripLevel.strip[nLoop1].vertex[nLoop2].y / -256.0f);
					glVertex2d(oTristripLevel.strip[nLoop1].vertex[nLoop2].x, oTristripLevel.strip[nLoop1].vertex[nLoop2].y);
				}
			glEnd();
			if (bWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		glDisable(GL_TEXTURE_2D);
	} else
	{
		// Fill in the ground using the PolyBoolean triangulation
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, oTextureIDs.iFloor);
		glColor3f(0.9f, 0.9f, 0.9f);
		for (nLoop1 = 0; nLoop1 < pPolyBooleanLevel->tnum; ++nLoop1)
		{
			if (bWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(1.0f);
			glBegin(GL_TRIANGLES);
				glTexCoord2f((float)pPolyBooleanLevel->tria[nLoop1].v0->g.x / 256.0f, (float)pPolyBooleanLevel->tria[nLoop1].v0->g.y / -256.0f);
				glVertex2i(pPolyBooleanLevel->tria[nLoop1].v0->g.x, pPolyBooleanLevel->tria[nLoop1].v0->g.y);

				glTexCoord2f((float)pPolyBooleanLevel->tria[nLoop1].v1->g.x / 256.0f, (float)pPolyBooleanLevel->tria[nLoop1].v1->g.y / -256.0f);
				glVertex2i(pPolyBooleanLevel->tria[nLoop1].v1->g.x, pPolyBooleanLevel->tria[nLoop1].v1->g.y);

				glTexCoord2f((float)pPolyBooleanLevel->tria[nLoop1].v2->g.x / 256.0f, (float)pPolyBooleanLevel->tria[nLoop1].v2->g.y / -256.0f);
				glVertex2i(pPolyBooleanLevel->tria[nLoop1].v2->g.x, pPolyBooleanLevel->tria[nLoop1].v2->g.y);
			glEnd();
			if (bWireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		glDisable(GL_TEXTURE_2D);
	}

	// draw the outline
	glColor3d(0.9, 0.9, 0.9);
	glLineWidth(1.5);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	for (nLoop1 = 0; nLoop1 < (u_int)oPolyLevel.num_contours; nLoop1++)
	{
		glBegin(GL_LINE_LOOP);
			for (nLoop2 = 0; nLoop2 < (u_int)oPolyLevel.contour[nLoop1].num_vertices; nLoop2++)
			{
				glVertex2d(oPolyLevel.contour[nLoop1].vertex[nLoop2].x, oPolyLevel.contour[nLoop1].vertex[nLoop2].y);
			}
		glEnd();
	}
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(2);

	// vertices
	/*glColor3f(1.0, 0.0, 0.0);
	for (nLoop1 = 0; nLoop1 < oPolyLevel.num_contours; nLoop1++)
	{
		glBegin(GL_POINTS);
			for (nLoop2 = 0; nLoop2 < oPolyLevel.contour[nLoop1].num_vertices; nLoop2++)
			{
				glVertex2i(oPolyLevel.contour[nLoop1].vertex[nLoop2].x, oPolyLevel.contour[nLoop1].vertex[nLoop2].y);
			}
		glEnd();
	}*/
}

// render the HUD
void RenderHUD()
{
	OglUtilsSwitchMatrix(SCREEN_SPACE_MATRIX);
	glLoadIdentity();
	glColor3f(1, 1, 1);
	//OglUtilsPrint(320, 0, 0, true, (char *)sFpsString.c_str());

	// Render FPS Counters
	u_int nOffset = 0;
	std::list<FpsCounter *> oFpsCounters = FpsCounter::GetCounters();
	for (std::list<FpsCounter *>::iterator it1 = oFpsCounters.begin(); it1 != oFpsCounters.end(); ++it1)
	{
		string sFpsCounterString = (*it1)->GetFpsString();
		glColor3f(0, 0, 0); OglUtilsPrint(1, nOffset * 8 + 1, 1, false, (char *)sFpsCounterString.c_str());
		glColor3f(1, 1, 1); OglUtilsPrint(0, nOffset++ * 8, 1, false, (char *)sFpsCounterString.c_str());
	}

	if (pLocalPlayer != NULL && pLocalPlayer->GetTeam() != 2 && bSelectTeamReady)
	{
		if (pLocalPlayer->IsReloading() && !pLocalPlayer->IsDead())
		{
			OglUtilsPrint(0, 408, 0, false, "reloading!");
		}
		if (pLocalPlayer->GetHealth() <= 40.0)
			glColor3d(0.9, 0.1, 0.1);
		else if (pLocalPlayer->GetHealth() <= 75.0)
			glColor3d(0.9, 0.5, 0.1);
		else
			glColor3d(1, 1, 1);
		sTempString = (std::string)"health: " + itos((int)ceil(pLocalPlayer->GetHealth()));
		OglUtilsPrint(0, 426, 0, false, (char *)sTempString.c_str());
		glColor3f(1, 1, 1);
		sTempString = (string)"ammo: " + itos(pLocalPlayer->GetSelClipAmmo());
		OglUtilsPrint(0, 444, 0, false, (char *)sTempString.c_str());
		sTempString = (string)"clips: " + itos(pLocalPlayer->GetSelClips());
		OglUtilsPrint(0, 462, 0, false, (char *)sTempString.c_str());
	}

	// Print the chat string
	if (nChatMode) {
		sTempString = (string)"Say: " + sChatString;
		if ((long)(g_pGameSession->RenderTimer().GetTime() * 4) % 2) sTempString += "_";
		OglUtilsPrint(0, 18, 0, false, (char *)sTempString.c_str());
	}

	// Print chat messages
	pChatMessages->Render();

	// Render the Select Team display
	if (bSelectTeamDisplay && bSelectTeamReady)
	{
		glColor3d(0.2, 0.95, 0.2);
		OglUtilsPrint(0, 250 + 15 * 0, 0, false, "Choose your team:");
		glColor3d(0.2, pLocalPlayer->GetTeam() != 0 ? 0.95 : 0.3, 0.2);
		OglUtilsPrint(0, 250 + 15 * 1, 0, false, "1. Red");
		glColor3d(0.2, pLocalPlayer->GetTeam() != 1 ? 0.95 : 0.3, 0.2);
		OglUtilsPrint(0, 250 + 15 * 2, 0, false, "2. Blue");
		glColor3d(0.2, pLocalPlayer->GetTeam() != 2 ? 0.95 : 0.3, 0.2);
		OglUtilsPrint(0, 250 + 15 * 3, 0, false, "3. Spectator");
		glColor3d(0.2, 0.95, 0.2);
		OglUtilsPrint(0, 250 + 15 * 4, 0, false, "0. Cancel");
	} else if (bSelectTeamDisplay && !bSelectTeamReady)
	{
		glColor3d(0.2, 0.95, 0.2);
		OglUtilsPrint(0, 250 + 15 * 0, 0, false, "Choose your team: Wait...");
	}

	/*sTempString = ftos(oPlayers[1]->GetIntX());
	glLoadIdentity();
	OglUtilsPrint(0, 30, 0, false, (char *)sTempString.c_str());*/

	// mouse cursor
	/*glBegin(GL_LINES);
		glVertex2i(iCursorX - 5, iCursorY);
		glVertex2i(iCursorX + 5, iCursorY);
		glVertex2i(iCursorX, iCursorY - 5);
		glVertex2i(iCursorX, iCursorY + 5);
	glEnd();*/

	// DEBUG some debug info
	if (1 &&
#ifdef EX0_DEBUG
		!glfwGetKey(GLFW_KEY_TAB))
#else
		glfwGetKey(GLFW_KEY_TAB))
#endif
	{
		//OglUtilsSwitchMatrix(SCREEN_SPACE_MATRIX);
		//OglUtilsSetMaskingMode(NO_MASKING_MODE);
		glColor3f(1, 1, 1);

		if (pLocalPlayer != NULL && pLocalPlayer->GetTeam() != 2) {
			sTempString = "x: " + ftos(pLocalPlayer->GetIntX());
			glLoadIdentity();
			OglUtilsPrint(0, 35, 1, false, (char *)sTempString.c_str());
			sTempString = "y: " + ftos(pLocalPlayer->GetIntY());
			glLoadIdentity();
			OglUtilsPrint(0, 35+7, 1, false, (char *)sTempString.c_str());
			sTempString = "z: " + ftos(pLocalPlayer->GetZ());
			glLoadIdentity();
			OglUtilsPrint(0, 35+14, 1, false, (char *)sTempString.c_str());
			sTempString = "vel: " + ftos(pLocalPlayer->GetVelocity());
			glLoadIdentity();
			OglUtilsPrint(80, 35, 1, false, (char *)sTempString.c_str());
		}

		for (u_int iLoop1 = 0; iLoop1 < nPlayerCount && iLoop1 < 10; ++iLoop1)
		{
			if (PlayerGet(iLoop1) != NULL) {
				glLoadIdentity();
if (PlayerGet(iLoop1)->pConnection == NULL) {
	printf("  OMG OMG 3: render pConnection == nuLL for PlayerGet(%d)\n", iLoop1);
	//printf("\n");
	continue;
}
				sTempString = (string)"pl#" + itos(iLoop1) + " name: '" + PlayerGet(iLoop1)->GetName()
					+ "' hp: " + itos((int)PlayerGet(iLoop1)->GetHealth())
					+ " lat: " + ftos(PlayerGet(iLoop1)->pConnection->GetLastLatency() * 0.1f)
					+ " lacsn: " + itos(PlayerGet(iLoop1)->cLatestAuthStateSequenceNumber);
				OglUtilsPrint(0, 60 + iLoop1 * 8, 1, false, (char *)sTempString.c_str());
			}
		}

		sTempString = "max oLocallyPredictedInputs.size(): " + itos(iTempInt);
		glLoadIdentity();
		OglUtilsPrint(0, 145, 1, false, (char *)sTempString.c_str());
		/*sTempString = "fTempFloat: " + ftos(fTempFloat);
		glLoadIdentity();
		OglUtilsPrint(0, 180, 0, false, (char *)sTempString.c_str());*/

		// Networking info
		sTempString = "g_cCurrentCommandSequenceNumber = " + itos(g_cCurrentCommandSequenceNumber);
		glLoadIdentity();
		OglUtilsPrint(0, 145+7, 1, false, (char *)sTempString.c_str());
		if (pServer != NULL) {
			sTempString = "cLastUpdateSequenceNumber = " + itos(pServer->cLastUpdateSequenceNumber);
			glLoadIdentity();
			OglUtilsPrint(0, 145+14, 1, false, (char *)sTempString.c_str());
		}
		sTempString = "oUnconfirmedMoves.size() = " + itos(oUnconfirmedMoves.size());
		glLoadIdentity();
		OglUtilsPrint(0, 145+21, 1, false, (char *)sTempString.c_str());

		//OglUtilsSwitchMatrix(WORLD_SPACE_MATRIX);
		//RenderOffsetCamera(false);
		//OglUtilsSetMaskingMode(WITH_MASKING_MODE);
	}
}

// render all players
void RenderPlayers()
{
	// Render other players
	/*for (u_int iLoop1 = 0; iLoop1 < nPlayerCount; ++iLoop1)
	{
		if (iLoop1 != iLocalPlayerID && PlayerGet(iLoop1)->bConnected
			&& PlayerGet(iLoop1)->GetTeam() != 2) {
			PlayerGet(iLoop1)->RenderInPast(kfInterpolate);
			PlayerGet(iLoop1)->RenderInPast(0);
		}
	}*/
	if (pLocalPlayer != NULL) {
		/*pLocalPlayer->fTicks = (float)(glfwGetTime() - (g_dNextTickTime - 1.0 / g_cCommandRate));
		if (pLocalPlayer->GetTeam() != 2) {
			if (!pLocalPlayer->IsDead()) {
				pLocalPlayer->UpdateInterpolatedPos();
			}
		}*/

		for (std::vector<CPlayer *>::iterator it1 = CPlayer::m_oPlayers.begin(); it1 < CPlayer::m_oPlayers.end(); ++it1) {
			if (*it1 != NULL && *it1 != pLocalPlayer && (*it1)->GetTeam() != 2) {
				if ((*it1)->pConnection->IsLocal())
					(*it1)->RenderInPast(0);
				else
					(*it1)->RenderInPast(kfInterpolate);
			}
		}

		// Render the local player
		if (pLocalPlayer->GetTeam() != 2) {
			for (int i = 1; i <= 1; ++i)
				pLocalPlayer->RenderInPast(kfInterpolate * i);
			pLocalPlayer->Render();
		}
	} else {
		for (std::vector<CPlayer *>::iterator it1 = CPlayer::m_oPlayers.begin(); it1 < CPlayer::m_oPlayers.end(); ++it1) {
			if (*it1 != NULL && (*it1)->GetTeam() != 2) {
				if ((*it1)->pConnection->IsLocal())
					(*it1)->RenderInPast(0);
				else
					(*it1)->RenderInPast(kfInterpolate);
			}
		}
	}
}

// render all particles
void RenderParticles()
{
	// render all particles
	oParticleEngine.Render();
}

// renders the interactive scene
void RenderInteractiveScene()
{
}

void RenderOffsetCamera(bool bLocalPlayerReferenceFrame)
{
	glLoadIdentity();

	if (pLocalPlayer == NULL || pLocalPlayer->GetTeam() == 2) {
		glTranslatef(0, 0, -680);
	} else
	{
		// Camera view
		if (iCameraType == 0) {
			glTranslatef(-345, 0, -680);
			glRotatef(-90, 0, 0, 1);
		} else if (iCameraType == 1) {
			//glTranslatef(-200, -250, -680);
			//glRotatef(-40, 0, 0, 1);
			glTranslatef(345, -185, -680);
			glRotatef(57, 0, 0, 1);
		} else if (iCameraType == 2) {
			glTranslatef(0, -250, -680);
			//glTranslatef(0, glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? -250 - pLocalPlayer->fAimingDistance : -250,
			//				glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? -680 : -680);
			//glTranslatef(0, -500, -1360);
			//glTranslatef(0, -750, -2040);
		} else if (iCameraType == 3) {
			glTranslatef(0, 0, -450);
			glRotatef(-40, 1, 0, 0);
			glTranslatef(0, -160, 0);
			/*glTranslatef(0, 0, -200);
			glRotatef(-40, 1, 0, 0);
			glTranslatef(0, -40, 0);*/
		}

		// Translate to a reference frame (either global, or local to the local player)
		if (!bLocalPlayerReferenceFrame) {
			glRotatef((pLocalPlayer->GetZ() * Math::RAD_TO_DEG), 0, 0, 1);
			glTranslatef(-(pLocalPlayer->GetIntX()), -(pLocalPlayer->GetIntY()), 0);
			/*State_t oState = pLocalPlayer->GetStateInPast(kfInterpolate);
			//glRotatef(oState.fZ * Math::RAD_TO_DEG, 0, 0, 1);
			glRotatef((pLocalPlayer->GetZ() * Math::RAD_TO_DEG), 0, 0, 1);
			glTranslatef(-oState.fX, -oState.fY, 0);*/
		}
	}
}

// renders the fov zone
void RenderFOV()
{
	if (pLocalPlayer == NULL || pLocalPlayer->GetTeam() == 2)
		return;

	// Create the FOV mask
	RenderCreateFOVMask();

	// Render the smoke grenade masks
	oParticleEngine.RenderFOVMask();

	// Highlight the Field of View
	OglUtilsSetMaskingMode(WITH_MASKING_MODE);
	//OglUtilsSwitchMatrix(WORLD_SPACE_MATRIX);
	RenderOffsetCamera(true);

	// Highlight what's visible
	glEnable(GL_BLEND);
	glShadeModel(GL_SMOOTH);
	glBegin(GL_QUADS);
		glColor4d(1, 1, 1, 0.075);
		glVertex2i(-250, -15);
		glVertex2i(250, -15);
		glColor4d(1, 1, 1, 0.025);
		glVertex2i(750, 1000);
		glVertex2i(-750, 1000);
	glEnd();
	glShadeModel(GL_FLAT);

	// Darken what's not
	glStencilFunc(GL_NOTEQUAL, 1, 1);
	glBegin(GL_QUADS);
		glColor4d(0, 0, 0, 0.6);
		glVertex2i(-750, -250);
		glVertex2i(750, -250);
		glVertex2i(750, 1000);
		glVertex2i(-750, 1000);
	glEnd();
	glStencilFunc(GL_EQUAL, 1, 1);
	glDisable(GL_BLEND);
}

// Creates the FOV mask
void RenderCreateFOVMask()
{
	int iLoop1, iLoop2;
	Vector2 oVector;
	Ray2 oRay;

	// Start rendering to the mask the fully unobstructed Field of View
	OglUtilsSetMaskingMode(RENDER_TO_MASK_MODE);
	OglUtilsSetMaskValue(1);
	glClear(GL_STENCIL_BUFFER_BIT);		// Always needed to be sure player can't see through walls, even due to glitches (i.e. drag the window outside the desktop viewable area)

	//OglUtilsSwitchMatrix(WORLD_SPACE_MATRIX);
	RenderOffsetCamera(true);		// Reset the matrix, local reference frame

	glBegin(GL_TRIANGLES);
		glVertex2i(-1250, 1000);
		glVertex2i(0, 0);
		glVertex2i(1250, 1000);
	glEnd();
	gluPartialDisk(oQuadricObj, 0, 8, 12, 1, 30.0, 300.0);

	// Now clip away from the full view what's blocked by the walls
	OglUtilsSetMaskValue(0);

	RenderOffsetCamera(false);		// Reset the matrix, global reference frame

	glBegin(GL_QUADS);
		for (iLoop1 = 0; iLoop1 < oPolyLevel.num_contours; iLoop1++)
		{
			for (iLoop2 = 1; iLoop2 < oPolyLevel.contour[iLoop1].num_vertices; iLoop2++)
			{
				glVertex2d(oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x, oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y);

				oRay.Origin().x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
				oRay.Origin().y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
				oRay.Direction().x = (float)oRay.Origin().x - pLocalPlayer->GetIntX();
				oRay.Direction().y = (float)oRay.Origin().y - pLocalPlayer->GetIntY();
				oVector = MathProjectRay(oRay, 500);
				glVertex2f(oVector.x, oVector.y);

				oRay.Origin().x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].x;
				oRay.Origin().y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].y;
				oRay.Direction().x = (float)oRay.Origin().x - pLocalPlayer->GetIntX();
				oRay.Direction().y = (float)oRay.Origin().y - pLocalPlayer->GetIntY();
				oVector = MathProjectRay(oRay, 500);
				glVertex2f(oVector.x, oVector.y);

				glVertex2d(oPolyLevel.contour[iLoop1].vertex[iLoop2].x, oPolyLevel.contour[iLoop1].vertex[iLoop2].y);
			}

			// last vertex
			glVertex2d(oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x, oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y);

			oRay.Origin().x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
			oRay.Origin().y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
			oRay.Direction().x = (float)oRay.Origin().x - pLocalPlayer->GetIntX();
			oRay.Direction().y = (float)oRay.Origin().y - pLocalPlayer->GetIntY();
			oVector = MathProjectRay(oRay, 500);
			glVertex2f(oVector.x, oVector.y);

			oRay.Origin().x = (float)oPolyLevel.contour[iLoop1].vertex[0].x;
			oRay.Origin().y = (float)oPolyLevel.contour[iLoop1].vertex[0].y;
			oRay.Direction().x = (float)oRay.Origin().x - pLocalPlayer->GetIntX();
			oRay.Direction().y = (float)oRay.Origin().y - pLocalPlayer->GetIntY();
			oVector = MathProjectRay(oRay, 500);
			glVertex2f(oVector.x, oVector.y);

			glVertex2d(oPolyLevel.contour[iLoop1].vertex[0].x, oPolyLevel.contour[iLoop1].vertex[0].y);
		}
	glEnd();
}

void RenderSmokeFOVMask(Mgc::Vector2 oSmokePosition, float fSmokeRadius)
{
	const float		fSmokeMaskLength = 750;
	const int		nSubdivisions = 4;

	Mgc::Vector2 oPlayerPosition(pLocalPlayer->GetIntX(), pLocalPlayer->GetIntY());
	Mgc::Vector2 oDirection = (oSmokePosition - oPlayerPosition);
	float fDistance = oDirection.Unitize();
	float fAngleToPlayer = Math::ATan2(-oDirection.y, -oDirection.x);

	// Check if the player located outside of the smoke
	if (fDistance > fSmokeRadius) {
		// Circular part
		glPushMatrix();
		glTranslatef(oSmokePosition.x, oSmokePosition.y, 0);
		gluDisk(oQuadricObj, 0, fSmokeRadius * 0.75, 16, 1);
		glPopMatrix();

		// Trapezoid part
		float fAngle = Mgc::Math::ACos(fSmokeRadius / fDistance);
		Mgc::Vector2 oEndPoint = oSmokePosition + oDirection * (fSmokeMaskLength - fDistance);
		Mgc::Vector2 oCrossDirection = oDirection.UnitCross();
		Mgc::Vector2 oEndPointR = oEndPoint + oCrossDirection * (Mgc::Math::Tan(Mgc::Math::HALF_PI - fAngle) * fSmokeMaskLength);
		Mgc::Vector2 oEndPointL = oEndPoint - oCrossDirection * (Mgc::Math::Tan(Mgc::Math::HALF_PI - fAngle) * fSmokeMaskLength);

		glEnable(GL_POLYGON_STIPPLE);
		glBegin(GL_TRIANGLE_FAN);
			glVertex2f(oSmokePosition.x, oSmokePosition.y);
			//glVertex2f(oEndPoint.x, oEndPoint.y);
			glVertex2f(oEndPointR.x, oEndPointR.y);

			// Just before step
			glVertex2f(oSmokePosition.x + fSmokeRadius * Mgc::Math::Cos(fAngleToPlayer + fAngle),
					   oSmokePosition.y + fSmokeRadius * Mgc::Math::Sin(fAngleToPlayer + fAngle));

			// Intermediate steps
			for (int nStep = nSubdivisions - 1; nStep > -nSubdivisions; --nStep) {
				float fTempAngle = (float)nStep / nSubdivisions * Mgc::Math::HALF_PI;
				if (Mgc::Math::FAbs(fTempAngle) >= fAngle) continue;

				glVertex2f(oSmokePosition.x + fSmokeRadius * Mgc::Math::Cos(fAngleToPlayer + fTempAngle),
						   oSmokePosition.y + fSmokeRadius * Mgc::Math::Sin(fAngleToPlayer + fTempAngle));
			}

			// Middle step
			//glVertex2f((oSmokePosition - oDirection * fSmokeRadius).x, (oSmokePosition - oDirection * fSmokeRadius).y);

			// Just after step
			glVertex2f(oSmokePosition.x + fSmokeRadius * Mgc::Math::Cos(fAngleToPlayer - fAngle),
					   oSmokePosition.y + fSmokeRadius * Mgc::Math::Sin(fAngleToPlayer - fAngle));

			glVertex2f(oEndPointL.x, oEndPointL.y);
			glVertex2f(oEndPointR.x, oEndPointR.y);
		glEnd();
		glDisable(GL_POLYGON_STIPPLE);
	}
	else
	{
		// Circular part
		glPushMatrix();
		glTranslatef(oSmokePosition.x, oSmokePosition.y, 0);
		gluDisk(oQuadricObj, 0, fSmokeRadius * 0.75, 16, 1);
		glPopMatrix();

		// The rest
		glPushMatrix();
		RenderOffsetCamera(true);
		glEnable(GL_POLYGON_STIPPLE);
		glBegin(GL_QUADS);
			glColor4d(0, 0, 0, 0.6);
			glVertex2i(-750, -250);
			glVertex2i(750, -250);
			glVertex2i(750, 1000);
			glVertex2i(-750, 1000);
		glEnd();
		glDisable(GL_POLYGON_STIPPLE);
		glPopMatrix();
	}
}
