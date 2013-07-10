float MathCoordToRad(int x1, int y1, int x2, int y2);

std::string itos(const int &iInteger);

std::string ftos(const float &fFloat);

template <typename T>
std::string toString(const T &type, int nPrecision = 0)
{
	std::ostringstream os;
	if (nPrecision != 0) os.precision(nPrecision);
	os << type;
	return os.str();
}

// projects a ray and returns a point at least so much units away from origin
Vector2 MathProjectRay(Ray2 oRay, int iDistance);
