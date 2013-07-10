// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

// computation of angle (in rads)
float MathCoordToRad(int iX1, int iY1, int iX2, int iY2)
{
	float fAngle = 0;

	if (iX1 != iX2)
	{
		fAngle = Math::ATan(fabs((Mgc::Real)(iY2 - iY1) / (iX2 - iX1)));
		if (iX1 > iX2) fAngle = Math::PI - fAngle;
		if (iY1 < iY2) fAngle = Math::PI * 2 - fAngle;
	}
	else
	{
		if (iY2 != iY1)
		{
			fAngle = Math::PI / 2;
			if (iY1 < iY2) fAngle += Math::PI;
		}
		else
		{
			fAngle = 0.0;
		}
	}
	while (fAngle < 0 || fAngle >= Math::TWO_PI)
	{
		if (fAngle < 0) fAngle += Math::TWO_PI;
		else fAngle -= Math::TWO_PI;
	}

	return fAngle;
}

// DEBUG - Not sure if this is entirely memory-usage correct
// convert int to string
std::string itos(const int nInteger)
{
	std::ostringstream oStringStream;
	oStringStream << nInteger;
	return oStringStream.str();
}

// DEBUG - Not sure if this is entirely memory-usage correct
// convert float to string
std::string ftos(const float fFloat)
{
	std::ostringstream oStringStream;
	oStringStream << fFloat;
	return oStringStream.str();
}

// DEBUG - Not sure if this is entirely memory-usage correct
// convert double to string
std::string dtos(const double dDouble)
{
	std::ostringstream oStringStream;
	oStringStream << dDouble;
	return oStringStream.str();
}

// projects a ray and returns a point at least so much units away from origin
Vector2 MathProjectRay(Ray2 oRay, int iDistance)
{
	Vector2 oVector = oRay.Origin();

	while(fabs(oVector.x - oRay.Origin().x) < iDistance
	   && fabs(oVector.y - oRay.Origin().y) < iDistance)
	{
		// A (hopefully) temporary fix
		oVector += oRay.Direction() * 100.0;
	}

	return oVector;
}
