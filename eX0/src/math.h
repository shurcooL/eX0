float MathCoordToRad(int x1, int y1, int x2, int y2);

std::string itos(const int iInteger);

std::string ftos(const float fFloat);

std::string dtos(const double dDouble);

template <typename T>
std::string to_string(const T & type, int nPrecision = 0)
{
	std::ostringstream os;
	if (0 != nPrecision) os.precision(nPrecision);
	os << type;
	return os.str();
}

// projects a ray and returns a point at least so much units away from origin
Vector2 MathProjectRay(Ray2 oRay, int iDistance);
