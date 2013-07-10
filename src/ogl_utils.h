#define SCREEN_SPACE_MATRIX		0
#define WORLD_SPACE_MATRIX		1

#define NO_MASKING_MODE			0
#define	WITH_MASKING_MODE		1
#define RENDER_TO_MASK_MODE		2

bool OglUtilsInitGL();

void OglUtilsDeinitGL();

void OglUtilsInitFont();

void OglUtilsKillFont();

void OglUtilsPrint(int iX, int iY, int iFont, bool bCentered, char *chText);
//void OglUtilsPrint(int iX, int iY, int iFont, bool bCentered, const char *chFmt, ...);

void OglUtilsSwitchMatrix(int iWhichMatrix);

void OglUtilsSetMaskingMode(int nMaskingMode);

void OglUtilsSetMaskValue(int nValue);
