// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

// returns whether a point is inside the land or not
bool ColHandIsPointInside(int iX, int iY)
{
	gpc_vertex	oVertex1, oVertex2;
	bool	bInside = true;		// initial test condition
	bool	bFlag1, bFlag2;

	// loop through all contours
	for (int iLoop1 = 0; iLoop1 < oPolyLevel.num_contours; iLoop1++)
	{
		oVertex1 = oPolyLevel.contour[iLoop1].vertex[0];
		bFlag1 = (iY >= oVertex1.y);

		// loop through all vertices in the contour
		for (int iLoop2 = 0; iLoop2 < oPolyLevel.contour[iLoop1].num_vertices; iLoop2++)
		{
			if (iLoop2 == oPolyLevel.contour[iLoop1].num_vertices - 1)
			{
				// if it the last line, point to 1st vertex
				oVertex2 = oPolyLevel.contour[iLoop1].vertex[0];
			}
			else
			{
				// otherwise point to next vertex
				oVertex2 = oPolyLevel.contour[iLoop1].vertex[iLoop2 + 1];
			}
			bFlag2 = (iY >= oVertex2.y);	// is it over or under

			if (bFlag1 != bFlag2)		// make sure the horisontal line on the point would cross our testing line
			{
				// calculate that the oVertex actually goes beyond the line - therefore crosses it
				// a trick from graphic gems IV to get rid of the x-intercept divide
				if (
				  ((oVertex2.y - iY) * (oVertex1.x - oVertex2.x) >=
				  (oVertex2.x - iX) * (oVertex1.y - oVertex2.y)) == bFlag2 )
				{
					bInside = !bInside;	// if it crossed, toggle the inside bFlag
				}
			}
			oVertex1 = oVertex2;	// point to next oVertex
			bFlag1 = bFlag2;		// update the bFlag
		}
	}

	return bInside;
}

// is a point to the left or the right of a line? >0 - left; =0 - on the line; <0 - right
inline float ColHandIsLeft(Vector2 oPoint0, Vector2 oPoint1, Vector2 oPoint2)
{
    return (oPoint1.x - oPoint0.x) * (oPoint2.y - oPoint0.y)
			- (oPoint2.x - oPoint0.x) * (oPoint1.y - oPoint0.y);
}

// checks to see if player pos is ok, or inside a wall
bool ColHandCheckPlayerPos(float *fX, float *fY, Real *oShortestDistance, Vector2 *oClosestPoint, int *iWhichCont, int *iWhichVert)
{
	int iLoop1, iLoop2;
	Vector2		oVector;
	Segment2	oSegment;
	Real		oParam;
	Real		oDistance;

	*oShortestDistance = 1000;

	// loop through all contours
	for (iLoop1 = 0; iLoop1 < oPolyLevel.num_contours; iLoop1++)
	{
		// loop through all vertices in the contour
		for (iLoop2 = 1; iLoop2 < oPolyLevel.contour[iLoop1].num_vertices; iLoop2++)
		{
			oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
			oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
			oSegment.Origin() = oVector;
			oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].x - oVector.x;
			oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].y - oVector.y;
			oSegment.Direction() = oVector;
			oVector.x = *fX;
			oVector.y = *fY;

			// make sure the distance we're looking for is possible
			if (!ColHandIsSegmentCloseToCircle(oVector.x, oVector.y, PLAYER_HALF_WIDTH + PLAYER_COL_DET_TOLERANCE, oSegment))
				continue;

			// calclulate the distance
			oDistance = Distance(oVector, oSegment, &oParam);

			if ((float)oDistance < PLAYER_HALF_WIDTH - PLAYER_COL_DET_TOLERANCE && oDistance < *oShortestDistance)
			{
				*oShortestDistance = oDistance;
				*oClosestPoint = oSegment.Origin() + oParam * oSegment.Direction();
				*iWhichCont = iLoop1;
				*iWhichVert = iLoop2;
			}
		}

		// do the last segment
		oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
		oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
		oSegment.Origin() = oVector;
		oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[0].x - oVector.x;
		oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[0].y - oVector.y;
		oSegment.Direction() = oVector;
		oVector.x = *fX;
		oVector.y = *fY;

		// make sure the distance we're looking for is possible
		if (!ColHandIsSegmentCloseToCircle(oVector.x, oVector.y, PLAYER_HALF_WIDTH + PLAYER_COL_DET_TOLERANCE, oSegment))
			continue;

		// calclulate the distance
		oDistance = Distance(oVector, oSegment, &oParam);

		if ((float)oDistance < PLAYER_HALF_WIDTH - PLAYER_COL_DET_TOLERANCE && oDistance < *oShortestDistance)
		{
			*oShortestDistance = oDistance;
			*oClosestPoint = oSegment.Origin() + oParam * oSegment.Direction();
			*iWhichCont = iLoop1;
			*iWhichVert = iLoop2;
		}
	}

	if ((float)*oShortestDistance < PLAYER_HALF_WIDTH - PLAYER_COL_DET_TOLERANCE)
		return false;
	else
		return true;
}
bool ColHandCheckPlayerPos(float *fX, float *fY)
{
	int iLoop1, iLoop2;
	Vector2		oVector;
	Segment2	oSegment;
	Real		oDistance;

	// loop through all contours
	for (iLoop1 = 0; iLoop1 < oPolyLevel.num_contours; iLoop1++)
	{
		// loop through all vertices in the contour
		for (iLoop2 = 1; iLoop2 < oPolyLevel.contour[iLoop1].num_vertices; iLoop2++)
		{
			oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
			oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
			oSegment.Origin() = oVector;
			oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].x - oVector.x;
			oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].y - oVector.y;
			oSegment.Direction() = oVector;
			oVector.x = *fX;
			oVector.y = *fY;

			// make sure the distance we're looking for is possible
			if (!ColHandIsSegmentCloseToCircle(oVector.x, oVector.y, PLAYER_HALF_WIDTH + PLAYER_COL_DET_TOLERANCE, oSegment))
				continue;

			// calclulate the distance
			oDistance = Distance(oVector, oSegment, NULL);

			if ((float)oDistance < PLAYER_HALF_WIDTH - PLAYER_COL_DET_TOLERANCE)
			{
				return false;
			}
		}

		// do the last segment
		oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
		oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
		oSegment.Origin() = oVector;
		oVector.x = (float)oPolyLevel.contour[iLoop1].vertex[0].x - oVector.x;
		oVector.y = (float)oPolyLevel.contour[iLoop1].vertex[0].y - oVector.y;
		oSegment.Direction() = oVector;
		oVector.x = *fX;
		oVector.y = *fY;

		// make sure the distance we're looking for is possible
		if (!ColHandIsSegmentCloseToCircle(oVector.x, oVector.y, PLAYER_HALF_WIDTH + PLAYER_COL_DET_TOLERANCE, oSegment))
			continue;

		// calclulate the distance
		oDistance = Distance(oVector, oSegment, NULL);

		if ((float)oDistance < PLAYER_HALF_WIDTH - PLAYER_COL_DET_TOLERANCE)
		{
			return false;
		}
	}

	return true;
}

// traces and returns true if the segment intersecs any world line
bool ColHandSegmentIntersects(float fStartX, float fStartY, float fEndX, float fEndY)
{
	int iLoop1, iLoop2;
	Segment2	oSegment1;
	Segment2	oSegment2;
	int			iQuantity;
	Real		oParams[2];

	oSegment1.Origin().x = fStartX;
	oSegment1.Origin().y = fStartY;
	oSegment1.Direction().x = fEndX - fStartX;
	oSegment1.Direction().y = fEndY - fStartY;

	// loop through all contours
	for (iLoop1 = 0; iLoop1 < oPolyLevel.num_contours; iLoop1++)
	{
		// loop through all vertices in the contour
		for (iLoop2 = 1; iLoop2 < oPolyLevel.contour[iLoop1].num_vertices; iLoop2++)
		{
			oSegment2.Origin().x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
			oSegment2.Origin().y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
			oSegment2.Direction().x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].x - oSegment2.Origin().x;
			oSegment2.Direction().y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].y - oSegment2.Origin().y;

			// make sure lines could interesect
			if (!ColHandIsSegmentCloseToSegment(oSegment1, oSegment2))
				continue;

			// check for intersection
			if (FindIntersection(oSegment1, oSegment2, iQuantity, oParams))
				return true;
		}

		// do the last segment
		oSegment2.Origin().x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
		oSegment2.Origin().y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
		oSegment2.Direction().x = (float)oPolyLevel.contour[iLoop1].vertex[0].x - oSegment2.Origin().x;
		oSegment2.Direction().y = (float)oPolyLevel.contour[iLoop1].vertex[0].y - oSegment2.Origin().y;

		// make sure lines could interesect
		if (!ColHandIsSegmentCloseToSegment(oSegment1, oSegment2))
			continue;

		// check for intersection
		if (FindIntersection(oSegment1, oSegment2, iQuantity, oParams))
			return true;
	}

	// no intersection
	return false;
}
bool ColHandSegmentIntersects(Segment2 oSegment)
{
	return ColHandSegmentIntersects(oSegment.Origin().x, oSegment.Origin().y, (oSegment.Origin() + oSegment.Direction()).x, (oSegment.Origin() + oSegment.Direction()).y);
}

// does a trace and returns where it intersects with world line, if doesn't returns origin
Vector2 ColHandTrace(Ray2 oRay)
{
	int iLoop1, iLoop2;
	Segment2	oSegment;
	int			iQuantity;
	Real		oParams[2], oClosest = 1000;

	// loop through all contours
	for (iLoop1 = 0; iLoop1 < oPolyLevel.num_contours; iLoop1++)
	{
		// loop through all vertices in the contour
		for (iLoop2 = 1; iLoop2 < oPolyLevel.contour[iLoop1].num_vertices; iLoop2++)
		{
			oSegment.Origin().x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
			oSegment.Origin().y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
			oSegment.Direction().x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].x - oSegment.Origin().x;
			oSegment.Direction().y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2].y - oSegment.Origin().y;

			// check for intersection
			if (FindIntersection(oRay, oSegment, iQuantity, oParams))
			{
				if (oParams[0] < oClosest)
					oClosest = oParams[0];
			}
		}

		// do the last segment
		oSegment.Origin().x = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].x;
		oSegment.Origin().y = (float)oPolyLevel.contour[iLoop1].vertex[iLoop2 - 1].y;
		oSegment.Direction().x = (float)oPolyLevel.contour[iLoop1].vertex[0].x - oSegment.Origin().x;
		oSegment.Direction().y = (float)oPolyLevel.contour[iLoop1].vertex[0].y - oSegment.Origin().y;

		// check for intersection
		if (FindIntersection(oRay, oSegment, iQuantity, oParams))
		{
			if (oParams[0] < oClosest)
				oClosest = oParams[0];
		}
	}

	return oRay.Origin() + oClosest * oRay.Direction();
}

// returns whether a point is inside a bbox or not
inline bool ColHandIsPointInsideBBox(float fX, float fY, float fStartX, float fStartY, float fEndX, float fEndY)
{
	return !(fX < __min(fStartX, fEndX) || fX > __max(fStartX, fEndX)
		 || fY < __min(fStartY, fEndY) || fY > __max(fStartY, fEndY));
}

// returns whether a segment is close to a point
inline bool ColHandIsSegmentCloseToPoint(float fX, float fY, float fStartX, float fStartY, float fEndX, float fEndY)
{
	return !((fStartX < fX && fEndX < fX) || (fStartX > fX && fEndX > fX)
		|| (fStartY < fY && fEndY < fY) || (fStartY > fY && fEndY > fY));
}
inline bool ColHandIsSegmentCloseToPoint(float fX, float fY, Segment2 oSegment)
{
	return ColHandIsSegmentCloseToPoint(fX, fY, oSegment.Origin().x, oSegment.Origin().y, (oSegment.Origin() + oSegment.Direction()).x, (oSegment.Origin() + oSegment.Direction()).y);
}

// returns whether a segment is close to a circle
inline bool ColHandIsSegmentCloseToCircle(float fX, float fY, float fRadius, float fStartX, float fStartY, float fEndX, float fEndY)
{
	return !((fStartX < fX - fRadius && fEndX < fX - fRadius)
		 || (fStartX > fX + fRadius && fEndX > fX + fRadius)
		 || (fStartY < fY - fRadius && fEndY < fY - fRadius)
		 || (fStartY > fY + fRadius && fEndY > fY + fRadius));
}
inline bool ColHandIsSegmentCloseToCircle(float fX, float fY, float fRadius, Segment2 oSegment)
{
	return ColHandIsSegmentCloseToCircle(fX, fY, fRadius, oSegment.Origin().x, oSegment.Origin().y, (oSegment.Origin() + oSegment.Direction()).x, (oSegment.Origin() + oSegment.Direction()).y);
}

// returns whether two segments could intersect
inline bool ColHandIsSegmentCloseToSegment(Segment2 oSegment1, Segment2 oSegment2)
{
	return (ColHandIsPointInsideBBox(oSegment1.Origin().x, oSegment1.Origin().y, oSegment2.Origin().x, oSegment2.Origin().y, (oSegment1.Origin() + oSegment1.Direction()).x, (oSegment1.Origin() + oSegment1.Direction()).y)
		 || ColHandIsPointInsideBBox((oSegment1.Origin() + oSegment1.Direction()).x, (oSegment1.Origin() + oSegment1.Direction()).y, oSegment2.Origin().x, oSegment2.Origin().y, (oSegment1.Origin() + oSegment1.Direction()).x, (oSegment1.Origin() + oSegment1.Direction()).y));
}
