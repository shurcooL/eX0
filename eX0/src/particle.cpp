// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

CParticle	oParticleEngine;

CParticle::CParticle()
{
	//printf("CParticle ctor\n");
	oParticles = NULL;
	iNumParticles = 0;
	SetArraySize(PARTICLE_INITIAL_SIZE);
}

CParticle::~CParticle()
{
	SetArraySize(0);
	//printf("oParticles points to %d (should be NULL if realloc(0) worked)\n", (int)oParticles);
	//free(oParticles);
	//printf("CParticle ~dtor\n");
}

void CParticle::GetInterpolatedPos(Vector2 *fIntPos, int iParticle)
{
	*fIntPos = oParticles[iParticle].oPosition + (float)((g_pGameSession->MainTimer().GetTime() - oParticles[iParticle].dTime + (double)oParticles[iParticle].fRenderTimeOffset) / PARTICLE_TICK_TIME) * oParticles[iParticle].oVelocity;
}

void CParticle::Render()
{
#ifdef EX0_CLIENT
	Vector2		fIntPos;

	RenderOffsetCamera(false);

	for (int iLoop1 = 0; iLoop1 < iNumParticles; ++iLoop1)
	{
		if (0 != oParticles[iLoop1].iWhatType)
		{
			switch(oParticles[iLoop1].iWhatType)
			{
			// bullet
			case BULLET:
				GetInterpolatedPos(&fIntPos, iLoop1);
				//glColor4f(0.95, 0.95, 0.1, 0.8);
				glLineWidth(2);
				glShadeModel(GL_SMOOTH);
				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glBegin(GL_LINES);
					//glColor4d(1.0, 0.65, 0.05, 0.1);
					glColor4d(1.0, 0.65, 0.05, 0.075);
					glVertex2d(fIntPos.x - oParticles[iLoop1].oVelocity.x * 0.2, fIntPos.y - oParticles[iLoop1].oVelocity.y * 0.2);
					//glColor4d(0.95, 0.95, 0.1, 0.9);
					glColor4d(0.95, 0.95, 0.1, 0.2);
					glVertex2d(fIntPos.x + oParticles[iLoop1].oVelocity.x * 0.1, fIntPos.y + oParticles[iLoop1].oVelocity.y * 0.1);
				glEnd();
				glDisable(GL_BLEND);
				glDisable(GL_LINE_SMOOTH);
				glShadeModel(GL_FLAT);
				glLineWidth(2);
				/*glBegin(GL_POINTS);
					glVertex2f(fIntPos.x, fIntPos.y);
				glEnd();*/
				break;
			// bouncy bullet
			case BOUNCY_BULLET:
				GetInterpolatedPos(&fIntPos, iLoop1);
				//fIntPos = oParticles[iLoop1].oPosition;
				glShadeModel(GL_SMOOTH);
				glEnable(GL_BLEND);
				//OglUtilsSetMaskingMode(NO_MASKING_MODE);
				glBegin(GL_QUADS);
					glColor4d(0.95, 0.75, 0.1, 0.0);
					glVertex2d(fIntPos.x - oParticles[iLoop1].oVelocity.UnitCross().UnitCross().x * 5, fIntPos.y - oParticles[iLoop1].oVelocity.UnitCross().UnitCross().y * 5);
					glColor4d(0.95, 0.75, 0.1, 0.6);
					glVertex2d(fIntPos.x - oParticles[iLoop1].oVelocity.UnitCross().x * 5, fIntPos.y - oParticles[iLoop1].oVelocity.UnitCross().y * 5);
					glColor4d(0.95, 0.1, 0.1, 0.9);
					glVertex2d(fIntPos.x + oParticles[iLoop1].oVelocity.UnitCross().UnitCross().x * 5, fIntPos.y + oParticles[iLoop1].oVelocity.UnitCross().UnitCross().y * 5);
					glColor4d(0.95, 0.75, 0.1, 0.6);
					glVertex2d(fIntPos.x + oParticles[iLoop1].oVelocity.UnitCross().x * 5, fIntPos.y + oParticles[iLoop1].oVelocity.UnitCross().y * 5);
				glEnd();
				glBegin(GL_POINTS);
					glColor3f(1, 0, 0);
					glVertex2f(oParticles[iLoop1].oPosition.x, oParticles[iLoop1].oPosition.y);
					glVertex2f((oParticles[iLoop1].oPosition + oParticles[iLoop1].oVelocity).x,
							   (oParticles[iLoop1].oPosition + oParticles[iLoop1].oVelocity).y);
					glVertex2f((oParticles[iLoop1].oPosition + 2 * oParticles[iLoop1].oVelocity).x,
							   (oParticles[iLoop1].oPosition + 2 * oParticles[iLoop1].oVelocity).y);
				glEnd();
				//OglUtilsSetMaskingMode(WITH_MASKING_MODE);
				glDisable(GL_BLEND);
				glShadeModel(GL_FLAT);
				break;
			// DEBUG: Render the smoke cloud FOV mask in wireframe
			case SMOKE_CLOUD:
				/*OglUtilsSetMaskingMode(NO_MASKING_MODE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glLineWidth(1);
				glColor3f(1, 0, 0);
				RenderSmokeFOVMask(oParticles[iLoop1].oPosition, oParticles[iLoop1].fMaxDamage);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				OglUtilsSetMaskingMode(WITH_MASKING_MODE);*/
				break;
			case WALL_HIT_SPARK:
				GetInterpolatedPos(&fIntPos, iLoop1);
				//glColor4f(0.95, 0.95, 0.1, 0.8);
				glLineWidth(1);
				glShadeModel(GL_SMOOTH);
				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glBegin(GL_LINES);
					glColor4d(1.0, 0.65, 0.05, 0.1);
					glVertex2d(fIntPos.x - oParticles[iLoop1].oVelocity.x * 3, fIntPos.y - oParticles[iLoop1].oVelocity.y * 3);
					glColor4d(0.95, 0.95, 0.1, 0.9);
					glVertex2d(fIntPos.x + oParticles[iLoop1].oVelocity.x * 0, fIntPos.y + oParticles[iLoop1].oVelocity.y * 0);
				glEnd();
				glDisable(GL_BLEND);
				glDisable(GL_LINE_SMOOTH);
				glShadeModel(GL_FLAT);
				glLineWidth(2);
				break;
			case BLOOD_SPLATTER:
				if (PlayerGet(oParticles[iLoop1].iOwnerID) == pLocalPlayer) OglUtilsSetMaskingMode(NO_MASKING_MODE);
				GetInterpolatedPos(&fIntPos, iLoop1);
				//glColor4f(0.95, 0.95, 0.1, 0.8);
				glLineWidth(3);
				glShadeModel(GL_SMOOTH);
				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glBegin(GL_LINES);
					glColor4d(0.5, 0.1, 0.1, 0.1);
					glVertex2d(fIntPos.x - oParticles[iLoop1].oVelocity.x * 2, fIntPos.y - oParticles[iLoop1].oVelocity.y * 2);
					glColor4d(0.990, 0.05, 0.05, 0.9);
					glVertex2d(fIntPos.x + oParticles[iLoop1].oVelocity.x * 0, fIntPos.y + oParticles[iLoop1].oVelocity.y * 0);
				glEnd();
				glDisable(GL_BLEND);
				glDisable(GL_LINE_SMOOTH);
				glShadeModel(GL_FLAT);
				glLineWidth(2);
				if (PlayerGet(oParticles[iLoop1].iOwnerID) == pLocalPlayer) OglUtilsSetMaskingMode(WITH_MASKING_MODE);
				break;
			default:
				break;
			}
		}
	}
#endif // EX0_CLIENT
}

void CParticle::CollisionHandling(int iParticle)
{
	Vector2		oVector;
	Segment2	oSegment1;
	Segment2	oSegment2;
	int			iQuantity;
	Real		oParams[2];

	switch(oParticles[iParticle].iWhatType)
	{
	// bullet
	case BULLET:
		// bullet-wall collision handling
		oSegment1.Origin() = oParticles[iParticle].oPosition;
		oSegment1.Direction() = oParticles[iParticle].oVelocity;

		for (int iLoop1 = 0; iLoop1 < oPolyLevel.num_contours; iLoop1++)
		{
			for (int iLoop2 = 1; iLoop2 < oPolyLevel.contour[iLoop1].num_vertices; iLoop2++)
			{
				oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
				oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
				oSegment2.Origin() = oVector;
				oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].x - oVector.x;
				oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].y - oVector.y;
				oSegment2.Direction() = oVector;

				// make sure lines could interesect
				//if (!ColHandIsSegmentCloseToSegment(oSegment1, oSegment2))
				//	continue;

				if (FindIntersection(oSegment1, oSegment2, iQuantity, oParams))
				{
					if (oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME < oParticles[iParticle].dDieAt) {
						oParticles[iParticle].iWillHit = NO_PLAYER;
						oParticles[iParticle].dDieAt = oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME;

						oParticles[iParticle].oHitNormal = oSegment2.Direction().UnitCross();		// the normal

						if (oParticles[iParticle].oHitNormal.Dot(oParticles[iParticle].oVelocity) > 0)
							oParticles[iParticle].oHitNormal *= -1;
					}
				}
			}

			// last segment
			oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[oPolyLevel.contour[iLoop1].num_vertices - 1].x;
			oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[oPolyLevel.contour[iLoop1].num_vertices - 1].y;
			oSegment2.Origin() = oVector;
			oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[0].x - oVector.x;
			oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[0].y - oVector.y;
			oSegment2.Direction() = oVector;

			// make sure lines could interesect
			//if (!ColHandIsSegmentCloseToSegment(oSegment1, oSegment2))
			//	continue;

			if (FindIntersection(oSegment1, oSegment2, iQuantity, oParams))
			{
				if (oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME < oParticles[iParticle].dDieAt) {
					oParticles[iParticle].iWillHit = NO_PLAYER;
					oParticles[iParticle].dDieAt = oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME;

					oParticles[iParticle].oHitNormal = oSegment2.Direction().UnitCross();		// the normal

					if (oParticles[iParticle].oHitNormal.Dot(oParticles[iParticle].oVelocity) > 0)
							oParticles[iParticle].oHitNormal *= -1;
				}
			}
		}

		// bullet-player collision handling
		oSegment1.Origin() = oParticles[iParticle].oPosition;
		oSegment1.Direction() = oParticles[iParticle].oVelocity;

		for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
		{
			// a bullet can't hit his owner
#ifdef EX0_CLIENT
			if (PlayerGet(nPlayer) == NULL|| nPlayer == oParticles[iParticle].iOwnerID
#else
			if (PlayerGet(nPlayer)->pConnection == NULL || nPlayer == oParticles[iParticle].iOwnerID
#endif
				|| PlayerGet(nPlayer)->IsDead() || PlayerGet(nPlayer)->GetTeam() == 2)
				continue;

			State_st oPlayerState = PlayerGet(nPlayer)->GetStateAtTime(oParticles[iParticle].dTime + PARTICLE_TICK_TIME*0.5 - ((PlayerGet(nPlayer)->pConnection == PlayerGet(oParticles[iParticle].iOwnerID)->pConnection) ? 0 : kfInterpolate));
			oVector.x = oPlayerState.fX;
			oVector.y = oPlayerState.fY;

			oParams[1] = Distance(oVector, oSegment1, oParams);

			if ((float)oParams[1] < PLAYER_HALF_WIDTH)
			// the bullet will hit this player
			{
				if (oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME < oParticles[iParticle].dDieAt)
				{
					oParticles[iParticle].iWillHit = nPlayer;
					oParticles[iParticle].fMaxDamage *= Math::FastSin0(0.5f + (PLAYER_HALF_WIDTH - oParams[1]) * Math::HALF_PI / PLAYER_HALF_WIDTH * 0.5f);
					oParticles[iParticle].dDieAt = oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME;

					//printf("hit player at %f, %f\n", oPlayerState.fX, oPlayerState.fY);
				}
			}
		}
		break;
	// bouncy bullet
	case BOUNCY_BULLET:
	//case BLOOD_SPLATTER:
		// bullet-wall collision handling
		oSegment1.Origin() = oParticles[iParticle].oPosition;
		oSegment1.Direction() = oParticles[iParticle].oVelocity;

		for (int iLoop1 = 0; iLoop1 < oPolyLevel.num_contours; iLoop1++)
		{
			for (int iLoop2 = 1; iLoop2 < oPolyLevel.contour[iLoop1].num_vertices; iLoop2++)
			{
				oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
				oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
				oSegment2.Origin() = oVector;
				oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].x - oVector.x;
				oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].y - oVector.y;
				oSegment2.Direction() = oVector;

				// make sure lines could interesect
				//if (!ColHandIsSegmentCloseToSegment(oSegment1, oSegment2))
				//	continue;

				if (FindIntersection(oSegment1, oSegment2, iQuantity, oParams))
				{
					if (oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME < oParticles[iParticle].dDieAt) {
						// This particle type doesn't die from walls
						//oParticles[iParticle].iWillHit = NO_PLAYER;
						//oParticles[iParticle].dDieAt = oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME;

						oParticles[iParticle].oHitNormal = oSegment2.Direction().UnitCross();		// the normal
					}
				}
			}

			// last segment
			oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[oPolyLevel.contour[iLoop1].num_vertices - 1].x;
			oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[oPolyLevel.contour[iLoop1].num_vertices - 1].y;
			oSegment2.Origin() = oVector;
			oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[0].x - oVector.x;
			oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[0].y - oVector.y;
			oSegment2.Direction() = oVector;

			// make sure lines could interesect
			//if (!ColHandIsSegmentCloseToSegment(oSegment1, oSegment2))
			//	continue;

			if (FindIntersection(oSegment1, oSegment2, iQuantity, oParams))
			{
				if (oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME < oParticles[iParticle].dDieAt) {
					// This particle type doesn't die from walls
					//oParticles[iParticle].iWillHit = NO_PLAYER;
					//oParticles[iParticle].dDieAt = oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME;

					oParticles[iParticle].oHitNormal = oSegment2.Direction().UnitCross();		// the normal
				}
			}
		}

		// Bounce the bullet off a wall
		if (oParticles[iParticle].oHitNormal != Vector2::ZERO) {
			oParticles[iParticle].oVelocity += oParticles[iParticle].oHitNormal * -2.0f * oParticles[iParticle].oHitNormal.Dot(oParticles[iParticle].oVelocity);
			//oParticles[iParticle].oVelocity *= 0.75;
			oParticles[iParticle].oVelocity *= oParticles[iParticle].oVelocity.Unitize() * 0.9f
				- fabs(oParticles[iParticle].oVelocity.Dot(oParticles[iParticle].oHitNormal)) * 0.25f;

			oParticles[iParticle].oHitNormal = Vector2::ZERO;
		}

		// bullet-player collision handling
		oSegment1.Origin() = oParticles[iParticle].oPosition;
		oSegment1.Direction() = oParticles[iParticle].oVelocity;

		for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
		{
#ifdef EX0_CLIENT
			if (PlayerGet(nPlayer) == NULL|| nPlayer == oParticles[iParticle].iOwnerID
#else
			if (PlayerGet(nPlayer)->pConnection == NULL || nPlayer == oParticles[iParticle].iOwnerID
#endif
				|| PlayerGet(nPlayer)->IsDead() || PlayerGet(nPlayer)->GetTeam() == 2)
				continue;

			State_st oPlayerState = PlayerGet(nPlayer)->GetStateAtTime(oParticles[iParticle].dTime + PARTICLE_TICK_TIME*0.5 - ((PlayerGet(nPlayer)->pConnection == PlayerGet(oParticles[iParticle].iOwnerID)->pConnection) ? 0 : kfInterpolate));
			oVector.x = oPlayerState.fX;
			oVector.y = oPlayerState.fY;

			oParams[1] = Distance(oVector, oSegment1, oParams);

			if ((float)oParams[1] < PLAYER_HALF_WIDTH)
			// the bullet will hit this player
			{
				if (oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME < oParticles[iParticle].dDieAt)
				{
					// Don't hit players for now
					/*oParticles[iParticle].iWillHit = nPlayer;
					oParticles[iParticle].fMaxDamage *= Math::FastSin0(0.5 + (PLAYER_HALF_WIDTH - oParams[1]) * Math::HALF_PI / PLAYER_HALF_WIDTH * 0.5);
					oParticles[iParticle].dDieAt = oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME;
					*/
				}
			}
		}
		break;
	case WALL_HIT_SPARK:
		// bullet-wall collision handling
		oSegment1.Origin() = oParticles[iParticle].oPosition;
		oSegment1.Direction() = oParticles[iParticle].oVelocity;

		for (int iLoop1 = 0; iLoop1 < oPolyLevel.num_contours; iLoop1++)
		{
			for (int iLoop2 = 1; iLoop2 < oPolyLevel.contour[iLoop1].num_vertices; iLoop2++)
			{
				oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
				oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
				oSegment2.Origin() = oVector;
				oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].x - oVector.x;
				oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].y - oVector.y;
				oSegment2.Direction() = oVector;

				// make sure lines could interesect
				//if (!ColHandIsSegmentCloseToSegment(oSegment1, oSegment2))
				//	continue;

				if (FindIntersection(oSegment1, oSegment2, iQuantity, oParams))
				{
					if (oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME < oParticles[iParticle].dDieAt) {
						oParticles[iParticle].iWillHit = NO_PLAYER;
						oParticles[iParticle].dDieAt = oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME;
					}
				}
			}

			// last segment
			oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[oPolyLevel.contour[iLoop1].num_vertices - 1].x;
			oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[oPolyLevel.contour[iLoop1].num_vertices - 1].y;
			oSegment2.Origin() = oVector;
			oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[0].x - oVector.x;
			oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[0].y - oVector.y;
			oSegment2.Direction() = oVector;

			// make sure lines could interesect
			//if (!ColHandIsSegmentCloseToSegment(oSegment1, oSegment2))
			//	continue;

			if (FindIntersection(oSegment1, oSegment2, iQuantity, oParams))
			{
				if (oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME < oParticles[iParticle].dDieAt) {
					oParticles[iParticle].iWillHit = NO_PLAYER;
					oParticles[iParticle].dDieAt = oParticles[iParticle].dTime + oParams[0] * PARTICLE_TICK_TIME;
				}
			}
		}
		break;
	default:
		break;
	}
}

void CParticle::HitCheck(int nParticle)
{
	double dCurrentTime = g_pGameSession->LogicTimer().GetTime();

	// Kill the particle if it's supposed to die by hitting something
	// and its life wasn't over at the time of impact
	if ( (dCurrentTime >= oParticles[nParticle].dDieAt) )
		//&& (oParticles[nParticle].dLife > 0 ||													// Either still alive now
		//	oParticles[nParticle].fDieAt < (1 + oParticles[nParticle].dLife / g_pGameSession->LogicTimer().GetTimePassed())) )		// Hit something before life ended
	{
		// give damage to whoever
		if (oParticles[nParticle].iWillHit != NO_PLAYER) {
			if (nullptr != pGameServer)
				PlayerGet(oParticles[nParticle].iWillHit)->GiveHealth(-oParticles[nParticle].fMaxDamage, oParticles[nParticle].iOwnerID);
//if (oParticles[nParticle].fMaxDamage < 100) bPaused = true;
			printf("%i hit %i for %f dmg\n", oParticles[nParticle].iOwnerID, oParticles[nParticle].iWillHit, oParticles[nParticle].fMaxDamage);

			// TEST: Spawn another particle
			for (int n = 0; n < 5; ++n)
			{
				// Angle(Normal) + (HALF_PI * 10% * (-100%~+100%))
				double dRandomAngle = MathCoordToRad(0, 0, (int)(oParticles[nParticle].oVelocity.x * 1000), (int)(oParticles[nParticle].oVelocity.y * 1000)) + Math::HALF_PI + (((rand() % 1001)*0.001) * 2 - 1) * Math::HALF_PI * 0.1;
				double dVelocity = 20 * (0.75 + (rand() % 1001)*0.001*0.5);		// 5.0 * (75%~125%)
				AddParticle(oParticles[nParticle].dDieAt - 1.75 * PARTICLE_TICK_TIME,
					oParticles[nParticle].oPosition.x + (float)(oParticles[nParticle].dDieAt - oParticles[nParticle].dTime) / (float)PARTICLE_TICK_TIME * 0.999f * oParticles[nParticle].oVelocity.x,
					oParticles[nParticle].oPosition.y + (float)(oParticles[nParticle].dDieAt - oParticles[nParticle].dTime) / (float)PARTICLE_TICK_TIME * 0.999f * oParticles[nParticle].oVelocity.y,
					// DEBUG: I will need to re-add player's velocity to the equation eventually...
					Math::Sin((float)dRandomAngle) * (float)dVelocity,
					Math::Cos((float)dRandomAngle) * (float)dVelocity,
					BLOOD_SPLATTER, 0,
					(float)dVelocity / 150.0f + 0.1f, oParticles[nParticle].iWillHit);
			}
		}
		else
		{
			// Splash dmg
			if (oParticles[nParticle].iWhatType == 2) {
				printf("BOOM at x=%f, y=%f and LogicTime=%.20f\n", oParticles[nParticle].oPosition.x, oParticles[nParticle].oPosition.y, oParticles[nParticle].dDieAt);
				for (uint8 nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
				{
					// a bullet can't hit his owner
					// but a bouncy one can
#ifdef EX0_CLIENT
					if (PlayerGet(nPlayer) == NULL/* || nPlayer == oParticles[nParticle].iOwnerID*/
#else
					if (PlayerGet(nPlayer)->pConnection == NULL/* || nPlayer == oParticles[nParticle].iOwnerID*/
#endif
						|| PlayerGet(nPlayer)->IsDead() || PlayerGet(nPlayer)->GetTeam() == 2)
						continue;

					State_st oPlayerState = PlayerGet(nPlayer)->GetStateAtTime(oParticles[nParticle].dDieAt - ((PlayerGet(nPlayer)->pConnection == PlayerGet(oParticles[nParticle].iOwnerID)->pConnection) ? 0 : kfInterpolate));
					//printf("player was at x=%.18f, y=%.18f when p-d\n", oPlayerState.fX, oPlayerState.fY);
					Vector2 oPlayerPosition;
					oPlayerPosition.x = oPlayerState.fX;
					oPlayerPosition.y = oPlayerState.fY;

					Vector2 oExplosionPosition = oParticles[nParticle].oPosition + (float)((oParticles[nParticle].dTime - oParticles[nParticle].dDieAt) / PARTICLE_TICK_TIME) * oParticles[nParticle].oVelocity;

					float fDistance = (oPlayerPosition - oExplosionPosition).Length();

					if (fDistance <= PLAYER_WIDTH * 10)
					// the bullet will hit this player
					{
						float fDamage = 1.0f - (fDistance / (PLAYER_WIDTH * 10));
						fDamage *= fDamage * 300.0f;		// Max 300 dmg at 0 distance, 0 dmg at max distance, 1/d^2 attenuation

						if (nullptr != pGameServer)
							PlayerGet(nPlayer)->GiveHealth(-fDamage);
						printf("%i hit %i for %f dmg [splash dmg from %lf away]\n", oParticles[nParticle].iOwnerID, nPlayer, fDamage, fDistance);
					}
				}
			}

			// Create a smoke grenade cloud
			if (oParticles[nParticle].iWhatType == BOUNCY_BULLET) {
				AddParticle(oParticles[nParticle].dDieAt, oParticles[nParticle].oPosition.x, oParticles[nParticle].oPosition.y,
				  0, 0, SMOKE_CLOUD, 60, 30, oParticles[nParticle].iOwnerID);
			}
			// TEST: Spawn another particle
			else if (oParticles[nParticle].iWhatType == BULLET)
			{
				eX0_assert(oParticles[nParticle].oHitNormal != Vector2::ZERO);

				for (int n = 0; n < 5; ++n)
				{
					// Angle(Normal) + (HALF_PI * 50% * (-100%~+100%))
					double dRandomAngle = MathCoordToRad(0, 0, (int)(oParticles[nParticle].oHitNormal.x * 1000), (int)(oParticles[nParticle].oHitNormal.y * 1000)) + Math::HALF_PI + (((rand() % 1001)*0.001) * 2 - 1) * Math::HALF_PI * 0.5;
					double dVelocity = 5.0 * (0.5 + (rand() % 1001)*0.001);		// 5.0 * (50%~150%)
					AddParticle(oParticles[nParticle].dDieAt,
						oParticles[nParticle].oPosition.x + (float)(oParticles[nParticle].dDieAt - oParticles[nParticle].dTime) / (float)PARTICLE_TICK_TIME * 0.999f * oParticles[nParticle].oVelocity.x,
						oParticles[nParticle].oPosition.y + (float)(oParticles[nParticle].dDieAt - oParticles[nParticle].dTime) / (float)PARTICLE_TICK_TIME * 0.999f * oParticles[nParticle].oVelocity.y,
						// DEBUG: I will need to re-add player's velocity to the equation eventually...
						Math::Sin((float)dRandomAngle) * (float)dVelocity,
						Math::Cos((float)dRandomAngle) * (float)dVelocity,
						WALL_HIT_SPARK, 0,
						dVelocity / 15.0 + 0.1, oParticles[nParticle].iOwnerID);
				}
			}
		}

		// kill it
		oParticles[nParticle].iWhatType = 0;
	}
}

void CParticle::Tick()
{
	double dCurrentTime = g_pGameSession->LogicTimer().GetTime();

	for (int iLoop1 = 0; iLoop1 < iNumParticles; ++iLoop1)
	{
		// is it an in-use particle?
		if (0 != oParticles[iLoop1].iWhatType)
		{
			HitCheck(iLoop1);
			if (0 == oParticles[iLoop1].iWhatType)
				continue;

			// DEBUG - This while loops needs to include checking for collisions above
			while (dCurrentTime >= oParticles[iLoop1].dTime + PARTICLE_TICK_TIME)
			{
				oParticles[iLoop1].dTime += PARTICLE_TICK_TIME;

				oParticles[iLoop1].oPosition += oParticles[iLoop1].oVelocity;
				if (BOUNCY_BULLET == oParticles[iLoop1].iWhatType) {
					// DEBUG: This is actually bouncy-bullet-specific stuff (i.e. slow down significantly during its last 1.25 secs of life
					if (oParticles[iLoop1].dDieAt - dCurrentTime > 1.25)
						oParticles[iLoop1].oVelocity *= 0.990f;
					else {
						float len = oParticles[iLoop1].oVelocity.Unitize();
						oParticles[iLoop1].oVelocity *= std::max<float>(0.01f, len - 0.2f);
					}
				} else {
					float fVelocity = oParticles[iLoop1].oVelocity.Unitize();
					//if (fVelocity <= 0.0125f) { oParticles[iLoop1].iWhatType = 0; break; }
					if (fVelocity + oParticles[iLoop1].fAcceleration > 0.01f) fVelocity += oParticles[iLoop1].fAcceleration; else fVelocity = 0.01f;
					oParticles[iLoop1].oVelocity *= fVelocity;
				}

				CollisionHandling(iLoop1);
				HitCheck(iLoop1);
				if (0 == oParticles[iLoop1].iWhatType)
					break;
			}
		}
	}
}

void CParticle::RenderFOVMask()
{
#ifdef EX0_CLIENT
	// Setup the rendering mode
	//OglUtilsSetMaskingMode(RENDER_TO_MASK_MODE);
	//OglUtilsSetMaskValue(0);

	for (int iLoop1 = 0; iLoop1 < iNumParticles; iLoop1++)
	{
		switch(oParticles[iLoop1].iWhatType)
		{
		// smoke
		case SMOKE_CLOUD:
			RenderSmokeFOVMask(oParticles[iLoop1].oPosition, oParticles[iLoop1].fMaxDamage);
			break;
		default:
			break;
		}
	}
#endif // EX0_CLIENT
}

void CParticle::SetArraySize(int iSize)
{
	int iLoop1 = iNumParticles;

	iNumParticles = iSize;

	oParticles = (struct Particle_st*)realloc(oParticles, iNumParticles * sizeof(struct Particle_st));
	for (; iLoop1 < iNumParticles; iLoop1++)
	{
		oParticles[iLoop1].iWhatType = 0;
	}
}

// Resets the particle array, deleting all active particles and going back to the default array size
void CParticle::Reset()
{
	iNumParticles = 0;
	SetArraySize(PARTICLE_INITIAL_SIZE);
}

void CParticle::AddParticle(double dTime, float fX, float fY, float fVelX, float fVelY, int iWhatType, float fMaxDamage, double dLife, uint8 iOwnerID)
{
	int iNextAvailParticle = NextAvailParticle();

	oParticles[iNextAvailParticle].oPosition.x = fX;
	oParticles[iNextAvailParticle].oPosition.y = fY;
	oParticles[iNextAvailParticle].oVelocity.x = fVelX;
	oParticles[iNextAvailParticle].oVelocity.y = fVelY;
	oParticles[iNextAvailParticle].fAcceleration = 0.0f;
	if (iWhatType == WALL_HIT_SPARK) oParticles[iNextAvailParticle].fAcceleration = -15.0f * PARTICLE_TICK_TIME;
	if (iWhatType == BLOOD_SPLATTER) oParticles[iNextAvailParticle].fAcceleration = -30.0f * PARTICLE_TICK_TIME;
	oParticles[iNextAvailParticle].fMaxDamage = fMaxDamage;
	oParticles[iNextAvailParticle].dTime = dTime;
	oParticles[iNextAvailParticle].dDieAt = dTime + dLife;
	oParticles[iNextAvailParticle].iWillHit = NO_PLAYER;
	oParticles[iNextAvailParticle].oHitNormal = Vector2::ZERO;
	oParticles[iNextAvailParticle].iOwnerID = iOwnerID;
	//oParticles[iNextAvailParticle].fRenderTimeOffset = (rand() % 1001)*0.001f * 1.0f/60;	// DEBUG: Assuming 60 fps, if higher fps, this value range is too high
	oParticles[iNextAvailParticle].fRenderTimeOffset = 0;

	oParticles[iNextAvailParticle].iWhatType = iWhatType;		// Assign iWhatType last to make this function more thread safe

	CollisionHandling(iNextAvailParticle);
}

int CParticle::NextAvailParticle()
{
	int iLoop1;

	for (iLoop1 = 0; iLoop1 < iNumParticles; iLoop1++)
	{
		if (oParticles[iLoop1].iWhatType == 0)		// Check if we've found a free particle
			return iLoop1;
	}

	// Otherwise extend the array and return the next free particle
	iLoop1 = iNumParticles;
	SetArraySize(iNumParticles + PARTICLE_INCREMENT_SIZE);

	return iLoop1;
}
