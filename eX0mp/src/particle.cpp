#include "globals.h"

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

void CParticle::Render()
{
	Vector2		fIntPos;

	for (int iLoop1 = 0; iLoop1 < iNumParticles; iLoop1++)
	{
		if (oParticles[iLoop1].iWhatType)
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
					glColor4f(1.0, 0.65, 0.05, 0.0);
					glVertex2f(fIntPos.x - oParticles[iLoop1].oVelocity.x * 0.2, fIntPos.y - oParticles[iLoop1].oVelocity.y * 0.2);
					glColor4f(0.95, 0.95, 0.1, 0.9);
					glVertex2f(fIntPos.x + oParticles[iLoop1].oVelocity.x * 0.1, fIntPos.y + oParticles[iLoop1].oVelocity.y * 0.1);
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
				OglUtilsSetMaskingMode(NO_MASKING_MODE);
				glBegin(GL_QUADS);
					glColor4f(0.95, 0.75, 0.1, 0.0);
					glVertex2i(fIntPos.x - oParticles[iLoop1].oVelocity.UnitCross().UnitCross().x * 5, fIntPos.y - oParticles[iLoop1].oVelocity.UnitCross().UnitCross().y * 5);
					glColor4f(0.95, 0.75, 0.1, 0.6);
					glVertex2i(fIntPos.x - oParticles[iLoop1].oVelocity.UnitCross().x * 5, fIntPos.y - oParticles[iLoop1].oVelocity.UnitCross().y * 5);
					glColor4f(0.95, 0.1, 0.1, 0.9);
					glVertex2i(fIntPos.x + oParticles[iLoop1].oVelocity.UnitCross().UnitCross().x * 5, fIntPos.y + oParticles[iLoop1].oVelocity.UnitCross().UnitCross().y * 5);
					glColor4f(0.95, 0.75, 0.1, 0.6);
					glVertex2i(fIntPos.x + oParticles[iLoop1].oVelocity.UnitCross().x * 5, fIntPos.y + oParticles[iLoop1].oVelocity.UnitCross().y * 5);
				glEnd();
				glBegin(GL_POINTS);
					glColor3f(1, 0, 0);
					glVertex2i(oParticles[iLoop1].oPosition.x, oParticles[iLoop1].oPosition.y);
					glVertex2i((oParticles[iLoop1].oPosition + oParticles[iLoop1].oVelocity).x,
							   (oParticles[iLoop1].oPosition + oParticles[iLoop1].oVelocity).y);
					glVertex2i((oParticles[iLoop1].oPosition + 2 * oParticles[iLoop1].oVelocity).x,
							   (oParticles[iLoop1].oPosition + 2 * oParticles[iLoop1].oVelocity).y);
				glEnd();
				OglUtilsSetMaskingMode(WITH_MASKING_MODE);
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
			default:
				break;
			}
		}
	}
}

void CParticle::CollisionHandling(int iParticle)
{
	Vector2		oVector, oNormal;
	Segment2	oSegment1;
	Segment2	oSegment2;
	int			iLoop1, iLoop2;
	int			iQuantity;
	Real		oParams[2];

	switch(oParticles[iParticle].iWhatType)
	{
	// bullet
	case 1:
		// bullet-wall collision handling
		oSegment1.Origin() = oParticles[iParticle].oPosition;
		oSegment1.Direction() = oParticles[iParticle].oVelocity;

		for (iLoop1 = 0; iLoop1 < oPolyLevel.num_contours; iLoop1++)
		{
			for (iLoop2 = 1; iLoop2 < oPolyLevel.contour[iLoop1].num_vertices; iLoop2++)
			{
				oVector.x = oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
				oVector.y = oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
				oSegment2.Origin() = oVector;
				oVector.x = oPolyLevel.contour[iLoop1].vertex[iLoop2].x - oVector.x;
				oVector.y = oPolyLevel.contour[iLoop1].vertex[iLoop2].y - oVector.y;
				oSegment2.Direction() = oVector;

				// make sure lines could interesect
				//if (!ColHandIsSegmentCloseToSegment(oSegment1, oSegment2))
				//	continue;

				if (FindIntersection(oSegment1, oSegment2, iQuantity, oParams))
				{
					if (oParams[0] < oParticles[iParticle].fDieAt) {
						oParticles[iParticle].iWillHit = -1;
						oParticles[iParticle].fDieAt = oParams[0];
					}
				}
			}

			// last segment
			oVector.x = oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
			oVector.y = oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
			oSegment2.Origin() = oVector;
			oVector.x = oPolyLevel.contour[iLoop1].vertex[0].x - oVector.x;
			oVector.y = oPolyLevel.contour[iLoop1].vertex[0].y - oVector.y;
			oSegment2.Direction() = oVector;

			// make sure lines could interesect
			//if (!ColHandIsSegmentCloseToSegment(oSegment1, oSegment2))
			//	continue;

			if (FindIntersection(oSegment1, oSegment2, iQuantity, oParams))
			{
				if (oParams[0] < oParticles[iParticle].fDieAt) {
					oParticles[iParticle].iWillHit = -1;
					oParticles[iParticle].fDieAt = oParams[0];
				}
			}
		}

		// bullet-player collision handling
		oSegment1.Origin() = oParticles[iParticle].oPosition;
		oSegment1.Direction() = oParticles[iParticle].oVelocity;

		for (iLoop1 = 0; iLoop1 < nPlayerCount; iLoop1++)
		{
			// a bullet can't hit his owner
			if (!PlayerGet(iLoop1)->bConnected || iLoop1 == oParticles[iParticle].iOwnerID
			  || PlayerGet(iLoop1)->IsDead())
				continue;

			oVector.x = PlayerGet(iLoop1)->GetIntX();
			oVector.y = PlayerGet(iLoop1)->GetIntY();

			oParams[1] = Distance(oVector, oSegment1, &oParams[0]);

			if ((float)oParams[1] < PLAYER_HALF_WIDTH)
			// the bullet will hit this player
			{
				if (oParams[0] < oParticles[iParticle].fDieAt)
				{
					oParticles[iParticle].iWillHit = iLoop1;
					oParticles[iParticle].fMaxDamage *= Math::FastSin0(0.5 + (PLAYER_HALF_WIDTH - oParams[1]) * Math::HALF_PI / PLAYER_HALF_WIDTH * 0.5);
					oParticles[iParticle].fDieAt = oParams[0];
				}
			}
		}
		break;
	// bouncy bullet
	case 2:
		oNormal = Vector2::ZERO;

		// bullet-wall collision handling
		oSegment1.Origin() = oParticles[iParticle].oPosition;
		oSegment1.Direction() = oParticles[iParticle].oVelocity;

		for (iLoop1 = 0; iLoop1 < oPolyLevel.num_contours; iLoop1++)
		{
			for (iLoop2 = 1; iLoop2 < oPolyLevel.contour[iLoop1].num_vertices; iLoop2++)
			{
				oVector.x = oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
				oVector.y = oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
				oSegment2.Origin() = oVector;
				oVector.x = oPolyLevel.contour[iLoop1].vertex[iLoop2].x - oVector.x;
				oVector.y = oPolyLevel.contour[iLoop1].vertex[iLoop2].y - oVector.y;
				oSegment2.Direction() = oVector;

				// make sure lines could interesect
				//if (!ColHandIsSegmentCloseToSegment(oSegment1, oSegment2))
				//	continue;

				if (FindIntersection(oSegment1, oSegment2, iQuantity, oParams))
				{
					if (oParams[0] < oParticles[iParticle].fDieAt) {
						oParticles[iParticle].iWillHit = -1;
						oParticles[iParticle].fDieAt = oParams[0];

						oNormal = oSegment2.Direction().UnitCross();		// the normal
					}
				}
			}

			// last segment
			oVector.x = oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
			oVector.y = oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
			oSegment2.Origin() = oVector;
			oVector.x = oPolyLevel.contour[iLoop1].vertex[0].x - oVector.x;
			oVector.y = oPolyLevel.contour[iLoop1].vertex[0].y - oVector.y;
			oSegment2.Direction() = oVector;

			// make sure lines could interesect
			//if (!ColHandIsSegmentCloseToSegment(oSegment1, oSegment2))
			//	continue;

			if (FindIntersection(oSegment1, oSegment2, iQuantity, oParams))
			{
				if (oParams[0] < oParticles[iParticle].fDieAt) {
					oParticles[iParticle].iWillHit = -1;
					oParticles[iParticle].fDieAt = oParams[0];

					oNormal = oSegment2.Direction().UnitCross();		// the normal
				}
			}
		}

		oParticles[iParticle].fDieAt = 10000;		// This particle type doesn't die from walls

		// Bounce the bullet off a wall
		if (oNormal != Vector2::ZERO) {
			oParticles[iParticle].oVelocity += oNormal * -2.0f * oNormal.Dot(oParticles[iParticle].oVelocity);
			//oParticles[iParticle].oVelocity *= 0.75;
			oParticles[iParticle].oVelocity *= oParticles[iParticle].oVelocity.Unitize() * 0.9
				- fabs(oParticles[iParticle].oVelocity.Dot(oNormal)) * 0.25;
		}

		// bullet-player collision handling
		oSegment1.Origin() = oParticles[iParticle].oPosition;
		oSegment1.Direction() = oParticles[iParticle].oVelocity;

		for (iLoop1 = 0; iLoop1 < nPlayerCount; iLoop1++)
		{
			if (!PlayerGet(iLoop1)->bConnected || iLoop1 == oParticles[iParticle].iOwnerID
			  || oPlayers[iLoop1]->IsDead())
				continue;

			oVector.x = oPlayers[iLoop1]->GetIntX();
			oVector.y = oPlayers[iLoop1]->GetIntY();

			oParams[1] = Distance(oVector, oSegment1, &oParams[0]);

			if ((float)oParams[1] < PLAYER_HALF_WIDTH)
			// the bullet will hit this player
			{
				if (oParams[0] < oParticles[iParticle].fDieAt)
				{
					// Don't hit players for now
					/*oParticles[iParticle].iWillHit = iLoop1;
					oParticles[iParticle].fMaxDamage *= Math::FastSin0(0.5 + (PLAYER_HALF_WIDTH - oParams[1]) * Math::HALF_PI / PLAYER_HALF_WIDTH * 0.5);
					oParticles[iParticle].fDieAt = oParams[0];
					*/
				}
			}
		}
		break;
	default:
		break;
	}
}

void CParticle::GetInterpolatedPos(Vector2 *fIntPos, int iParticle)
{
	*fIntPos = oParticles[iParticle].oPosition + (oParticles[iParticle].fTicks / PARTICLE_TICK_TIME) * oParticles[iParticle].oVelocity;
}

void CParticle::Tick()
{
	for (int iLoop1 = 0; iLoop1 < iNumParticles; iLoop1++)
	{
		// is it an in-use particle?
		if (oParticles[iLoop1].iWhatType)
		{
			oParticles[iLoop1].fTicks += dTimePassed;
			oParticles[iLoop1].fLife -= dTimePassed;

			// Kill the particle if it's supposed to die by hitting something
			// and its life wasn't over at the time of impact
			if ( (oParticles[iLoop1].fTicks / PARTICLE_TICK_TIME) >= oParticles[iLoop1].fDieAt
				&& (oParticles[iLoop1].fLife > 0 ||													// Either still alive now
					oParticles[iLoop1].fDieAt < (1 + oParticles[iLoop1].fLife / dTimePassed)) )		// Hit something before life ended
			{
				// give damage to whoever
				if (oParticles[iLoop1].iWillHit != -1)
					{oPlayers[oParticles[iLoop1].iWillHit]->GiveHealth(-oParticles[iLoop1].fMaxDamage);
				printf("%i hit %i for %f dmg\n", oParticles[iLoop1].iOwnerID, oParticles[iLoop1].iWillHit, oParticles[iLoop1].fMaxDamage);
				}
				//else printf("%i missed\n", oParticles[iLoop1].iOwnerID);

				// kill it and continue w/ the next particle
				oParticles[iLoop1].iWhatType = 0;
				continue;
			}

			// Kill it if its life is over
			if (oParticles[iLoop1].fLife <= 0)
			{
				// Splash dmg
				if (oParticles[iLoop1].iWhatType == 2) {
					for (int iLoop2 = 0; iLoop2 < nPlayerCount; iLoop2++)
					{
						// a bullet can't hit his owner
						// but a bouncy one can
						if (!PlayerGet(iLoop2)->bConnected/* || iLoop2 == oParticles[iLoop1].iOwnerID*/
						  || oPlayers[iLoop2]->IsDead())
							continue;

						Vector2 oVector;
						oVector.x = oPlayers[iLoop2]->GetIntX();
						oVector.y = oPlayers[iLoop2]->GetIntY();

						oVector -= oParticles[iLoop1].oPosition + (1 + oParticles[iLoop1].fLife / dTimePassed) * oParticles[iLoop1].oVelocity;

						float fDistance = oVector.Length();

						if (fDistance <= PLAYER_WIDTH * 10)
						// the bullet will hit this player
						{
							float fDamage = 1.0f - (fDistance / (PLAYER_WIDTH * 10));
							fDamage *= fDamage * 300.0f;		// Max 300 dmg at 0 distance, 0 dmg at max distance, 1/d^2 attenuation

							oPlayers[iLoop2]->GiveHealth(-fDamage);
							printf("%i hit %i for %f dmg [splash dmg from %lf away]\n", oParticles[iLoop1].iOwnerID, iLoop2, fDamage, fDistance);
						}
					}
				}

				// Create a smoke grenade cloud
				if (oParticles[iLoop1].iWhatType == BOUNCY_BULLET) {
					AddParticle(oParticles[iLoop1].oPosition.x, oParticles[iLoop1].oPosition.y,
					  0, 0, SMOKE_CLOUD, 60, 30, oParticles[iLoop1].iOwnerID);
				}

				// Kill it and continue w/ the next particle
				oParticles[iLoop1].iWhatType = 0;
				continue;
			}

			// DEBUG - This while loops needs to include checking for collisions above
			while (oParticles[iLoop1].fTicks >= PARTICLE_TICK_TIME)
			{
				oParticles[iLoop1].fTicks -= PARTICLE_TICK_TIME;
				if (oParticles[iLoop1].fLife > 1.25)
					oParticles[iLoop1].oVelocity *= 0.990;
				else {
					float len = oParticles[iLoop1].oVelocity.Unitize();
					oParticles[iLoop1].oVelocity *= __max(0.01f, len - 0.2f);
				}
				oParticles[iLoop1].oPosition += oParticles[iLoop1].oVelocity;
				CollisionHandling(iLoop1);
			}
		}
	}
}

void CParticle::RenderFOVMask()
{
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
}

void CParticle::SetArraySize(int iSize)
{
	int iLoop1 = iNumParticles;

	iNumParticles = iSize;

	oParticles = (struct Particle_t*)realloc(oParticles, iNumParticles * sizeof(struct Particle_t));
	for (; iLoop1 < iNumParticles; iLoop1++)
	{
		oParticles[iLoop1].iWhatType = 0;
	}

	// DEBUG - just keeping track of the particle array size
	//iTempInt = iSize;
}

// Resets the particle array, deleting all active particles and going back to the default array size
void CParticle::Reset()
{
	iNumParticles = 0;
	SetArraySize(PARTICLE_INITIAL_SIZE);
}

void CParticle::AddParticle(float fX, float fY, float fVelX, float fVelY, int iWhatType, float fMaxDamage, float fLife, int iOwnerID)
{
	int iNextAvailParticle = NextAvailParticle();

	oParticles[iNextAvailParticle].oPosition.x = fX;
	oParticles[iNextAvailParticle].oPosition.y = fY;
	oParticles[iNextAvailParticle].oVelocity.x = fVelX;
	oParticles[iNextAvailParticle].oVelocity.y = fVelY;
	oParticles[iNextAvailParticle].fMaxDamage = fMaxDamage;
	oParticles[iNextAvailParticle].fTicks = 0;
	oParticles[iNextAvailParticle].fLife = fLife;
	oParticles[iNextAvailParticle].fDieAt = 10000;
	oParticles[iNextAvailParticle].iWillHit = -1;
	oParticles[iNextAvailParticle].iOwnerID = iOwnerID;
	oParticles[iNextAvailParticle].iWhatType = iWhatType;

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
