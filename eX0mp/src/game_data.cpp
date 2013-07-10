#include "globals.h"

gpc_polygon		oPolyLevel;
gpc_tristrip	oTristripLevel;

// DEBUG: PolyBoolean level
PAREA			*pPolyBooleanLevel = NULL;

TextureIDs_t oTextureIDs;

void GameDataLoad()
{
	// load all textures
	GameDataLoadTextures();

	// DEBUG - temporarily just open a level right away
	//GameDataOpenLevel("levels/test3.wwl");
	//GameDataOpenLevel("levels/test_orientation.wwl");
}

void GameDataUnload()
{
	// close the level and free all memory
	GameDataEndLevel();

	// unload all textures
	DESTROY_TEXMANAGER
}

// load all textures
bool GameDataLoadTextures()
{
	// load 1st font texture
	oTextureIDs.iFonts[0] = TEXMANAGER.LoadTexture("data/fonts/font1.tga");
	oTextureIDs.iFonts[1] = TEXMANAGER.LoadTexture("data/fonts/font2.tga");
	oTextureIDs.iFloor = TEXMANAGER.LoadTexture("data/textures/floor.tga");
	oTextureIDs.nM4A1MuzzleFlash = TEXMANAGER.LoadTexture("data/textures/m4a1_muzzle_flashes.tga");
	// ...

	// no error checking just yet
	return true;
}

// load a level in memory
void GameDataOpenLevel(const char *chFileName)
{
	FILE	*pFile;

	// close previous level
	GameDataEndLevel();

	if ((pFile = fopen(chFileName, "r")) != NULL)
	{
		gpc_read_polygon(pFile, 0, &oPolyLevel);
		gpc_polygon_to_tristrip(&oPolyLevel, &oTristripLevel);
		fclose(pFile);
	}
	else
	{
		// level not found
		string sMessage = (string)"an eX0 level file \'" + chFileName + "\' could not be opened.";
#ifdef WIN32
		MessageBox(NULL, sMessage.c_str(), "eX0 error", MB_ICONERROR);
#else
		printf("%s\n", sMessage.c_str());
#endif
		Terminate(1);
	}

	// DEBUG: Open the same level with PolyBoolean
	LoadParea2(chFileName, &pPolyBooleanLevel);
	if (pPolyBooleanLevel == NULL)
		{ printf("Failed to load the PolyBoolean level.\n"); Terminate(2); }
	if (PAREA::Triangulate(pPolyBooleanLevel) != 0)
		{ printf("PolyBoolean level triangulation failed.\n"); Terminate(3); }

	// DEBUG: Print info about the level, namely the tristrip vertex count
	printf("opened level %s\n", chFileName);
	int nTotalTriangles = 0, nTotalVertices = 0;
	for (int nLoop1 = 0; nLoop1 < oTristripLevel.num_strips; ++nLoop1) {
		nTotalVertices += oTristripLevel.strip[nLoop1].num_vertices;
		nTotalTriangles += oTristripLevel.strip[nLoop1].num_vertices - 2;
	}
	printf("gpc tri: triangle count = %d; vertex count = %d; tristrips = %d\n", nTotalTriangles, nTotalVertices, oTristripLevel.num_strips);
	printf("PB tri: triangle count = %d; vertex count = %d\n", pPolyBooleanLevel->tnum, 3 * pPolyBooleanLevel->tnum);
}

// close currently opened level, free memory, reset vars
void GameDataEndLevel()
{
	gpc_free_polygon(&oPolyLevel);
	gpc_free_tristrip(&oTristripLevel);

	// DEBUG: Close the PolyBoolean level
	PAREA::Del(&pPolyBooleanLevel);
}
