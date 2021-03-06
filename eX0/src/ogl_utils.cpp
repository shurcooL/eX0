#include "globals.h"

GLUquadricObj	*oQuadricObj;

// local vars
GLuint	oFontBase[2];

// init all gl stuff
bool OglUtilsInitGL()
{
	// init the font
	OglUtilsInitFont();

	// Create a Quadric object
	oQuadricObj = gluNewQuadric();

	// reset the identity
	glLoadIdentity();

	// set the correct viewport
	glViewport(0, 0, 640, 480);
	//glViewport(0, 0, 2560, 1600);

	// init matrices
	OglUtilsSwitchMatrix(WORLD_SPACE_MATRIX);

	// set up some OpenGL properties
	glClearColor(0, 0, 0, 1);
	glClearStencil(0);

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);		// perspective correction hint
	glShadeModel(GL_FLAT);
	glPointSize(2);

	GLubyte cSmokeShade[128] = {
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
		0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55 };
	glPolygonStipple(cSmokeShade);

	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	return true;
}

// deinit all gl stuff
void OglUtilsDeinitGL()
{
	// kill the font
	OglUtilsKillFont();

	// Delete the Quadric object
	gluDeleteQuadric(oQuadricObj);
}

// print text on screen
void OglUtilsPrint(int iX, int iY, int iFont, PrintAlignment nAlignment, const char *chText)
{
	if (chText == NULL)
		return;

	//char *chAsciiCodes = (char *)malloc(sizeof(char) * (strlen(chText) + 1));

	//glLoadIdentity();
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, oTextureIDs.iFonts[iFont]);

	//vsprintf(chAsciiCodes, chText, NULL);

	if (LEFT == nAlignment)
		glTranslatef((GLfloat)iX, (GLfloat)iY, 0.0f);
	else if (CENTER == nAlignment)
		glTranslatef((GLfloat)(iX - strlen(chText) * (iFont == 0 ? 5 : 3)), (GLfloat)iY, 0.0f);
	else if (RIGHT == nAlignment)
		glTranslatef((GLfloat)(iX - strlen(chText) * 2 * (iFont == 0 ? 5 : 3)), (GLfloat)iY, 0.0f);

	glListBase(oFontBase[iFont]);
	//glCallLists(strlen(chAsciiCodes), GL_UNSIGNED_BYTE, chAsciiCodes);
	glCallLists((GLsizei)strlen(chText), GL_UNSIGNED_BYTE, chText);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glLoadIdentity();

	//free(chAsciiCodes);
}

// init the font
void OglUtilsInitFont()
{
	float fCharX;
	float fCharY;

	// Font 1
	oFontBase[0] = glGenLists(256);

	for (int iLoop1 = 0; iLoop1 < 256; iLoop1++)
	{
		fCharX = float(iLoop1 % 16) / 16.0f;
		fCharY = float(iLoop1 / 16) / 16.0f;

		glNewList(oFontBase[0] + iLoop1, GL_COMPILE);
			glBegin(GL_QUADS);
				glTexCoord2f(fCharX, 1 - fCharY - 0.0625f);
				glVertex2i(0, 16);
				glTexCoord2f(fCharX + 0.0625f, 1 - fCharY - 0.0625f);
				glVertex2i(16, 16);
				glTexCoord2f(fCharX + 0.0625f, 1 - fCharY);
				glVertex2i(16, 0);
				glTexCoord2f(fCharX, 1 - fCharY);
				glVertex2i(0, 0);
			glEnd();
			glTranslatef(10.0, 0.0, 0.0);
		glEndList();
	}

	// Font 2
	oFontBase[1] = glGenLists(256);

	for (int iLoop1 = 0; iLoop1 < 256; iLoop1++)
	{
		fCharX = float(iLoop1 % 16) / 16.0f;
		fCharY = float(iLoop1 / 16) / 16.0f;

		glNewList(oFontBase[1] + iLoop1, GL_COMPILE);
			glBegin(GL_QUADS);
				glTexCoord2f(fCharX, 1 - fCharY - 0.0625f);
				glVertex2i(0, 8);
				glTexCoord2f(fCharX + 0.0625f, 1 - fCharY - 0.0625f);
				glVertex2i(8, 8);
				glTexCoord2f(fCharX + 0.0625f, 1 - fCharY);
				glVertex2i(8, 0);
				glTexCoord2f(fCharX, 1 - fCharY);
				glVertex2i(0, 0);
			glEnd();
			glTranslatef(6.0, 0.0, 0.0);
		glEndList();
	}
}

// kill the font
void OglUtilsKillFont()
{
	glDeleteLists(oFontBase[0], 256);
	glDeleteLists(oFontBase[1], 256);
}

void OglUtilsSwitchMatrix(int iWhichMatrix)
{
	switch (iWhichMatrix)
	{
	// 2d ortho matrix
	case SCREEN_SPACE_MATRIX:
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		gluOrtho2D(0, 640, 480, 0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;
	// 3d perspective matrix
	case WORLD_SPACE_MATRIX:
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		gluPerspective(45.0, 640.0 / 480.0, 1.0, 5000.0);
		//gluPerspective(45.0, 1.6, 1.0, 5000.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;
	}
}

// Select the masking mode
void OglUtilsSetMaskingMode(int nMaskingMode)
{
	// DEBUG: A hack to temporary disable stencil operations (for a performance increase)
	if (!bStencilOperations)
	{
		switch (nMaskingMode)
		{
		case NO_MASKING_MODE:
		case WITH_MASKING_MODE:
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			break;

		case RENDER_TO_MASK_MODE:
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			break;
		}

		return;
	}

	switch (nMaskingMode)
	{
	case NO_MASKING_MODE:
		// Enable writing to the colour buffer and
		// disable writing to the stencil buffer
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilMask(0);

		// No stencil testing
		glDisable(GL_STENCIL_TEST);

		break;

	case WITH_MASKING_MODE:
		// Enable writing to the colour buffer and
		// disable writing to the stencil buffer
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilMask(0);

		// Enable stencil testing for masked writing to the colour buffer
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		break;

	case RENDER_TO_MASK_MODE:
		// Disable writing to the colour buffer and
		// enable writing to the stencil buffer
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glStencilMask(1);

		// Enable stencil testing for writing to the stencil buffer
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		OglUtilsSetMaskValue(1);		// Use value of 1 by default

		break;
	}
}

// Set the value to be written into the mask
void OglUtilsSetMaskValue(int nValue)
{
	glStencilFunc(GL_ALWAYS, nValue, 1);
}
