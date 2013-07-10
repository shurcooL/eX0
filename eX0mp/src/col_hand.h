// returns whether a point is inside the land or not
bool ColHandIsPointInside(int iX, int iY);

// is a point to the left or the right of a line? >0 - left; =0 - on the line; <0 - right
inline int ColHandIsLeft(Vector2 oPoint0, Vector2 oPoint1, Vector2 oPoint2);

// checks to see if player pos is ok, or inside a wall
bool ColHandCheckPlayerPos(float *fX, float *fY, Real *oShortestDistance, Vector2 *oClosestPoint, int *iWhichCont, int *iWhichVert);
bool ColHandCheckPlayerPos(float *fX, float *fY);

// traces and returns true if the segment intersecs any world line
bool ColHandSegmentIntersects(float fStartX, float fStartY, float fEndX, float fEndY);
bool ColHandSegmentIntersects(Segment2 oSegment);

// does a trace and returns where it intersects with world line, if doesn't returns origin
Vector2 ColHandTrace(Ray2 oRay);

// returns whether a point is inside a bbox or not
inline bool ColHandIsPointInsideBBox(float fX, float fY, float fStartX, float fStartY, float fEndX, float fEndY);

// returns whether a segment is close to a point
inline bool ColHandIsSegmentCloseToPoint(float fX, float fY, float fStartX, float fStartY, float fEndX, float fEndY);
inline bool ColHandIsSegmentCloseToPoint(float fX, float fY, Segment2 oSegment);

// returns whether a segment is close to a circle
inline bool ColHandIsSegmentCloseToCircle(float fX, float fY, float fRadius, float fStartX, float fStartY, float fEndX, float fEndY);
inline bool ColHandIsSegmentCloseToCircle(float fX, float fY, float fRadius, Segment2 oSegment);

// returns whether two segments could intersect
inline bool ColHandIsSegmentCloseToSegment(Segment2 oSegment1, Segment2 oSegment2);