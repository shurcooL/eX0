// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

//std::string			sLevelName = "test_orientation";
std::string			sLevelName = "test3";

gpc_polygon		oPolyLevel;
#ifdef EX0_CLIENT
gpc_tristrip	oTristripLevel;

// DEBUG: PolyBoolean level
PAREA			*pPolyBooleanLevel = NULL;

TextureIDs_t oTextureIDs;
#endif // EX0_CLIENT

bool GameDataLoad()
{
#ifdef EX0_CLIENT
	if (bWindowModeDEBUG) {
		// load all textures
		GameDataLoadTextures();
	}
#endif // EX0_CLIENT

	// DEBUG - temporarily just open a level right away
	printf("Loading level '%s'.\n", sLevelName.c_str());
	string sLevelPath1 = "./levels/" + sLevelName + ".wwl";
	string sLevelPath2 = "../eX0/levels/" + sLevelName + ".wwl";
	if (!GameDataOpenLevel(sLevelPath1.c_str()) &&
		!GameDataOpenLevel(sLevelPath2.c_str()))
		return false;

	return true;
}

void GameDataUnload()
{
	// close the level and free all memory
	GameDataEndLevel();

#ifdef EX0_CLIENT
	// unload all textures
	DESTROY_TEXMANAGER
#endif // EX0_CLIENT
}

#ifdef EX0_CLIENT
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
#endif // EX0_CLIENT

// load a level in memory
bool GameDataOpenLevel(const char *chFileName)
{
	FILE	*pFile;

	// close previous level
	GameDataEndLevel();

	if ((pFile = fopen(chFileName, "r")) != NULL)
	{
		gpc_read_polygon(pFile, 0, &oPolyLevel);
#ifdef EX0_CLIENT
		gpc_polygon_to_tristrip(&oPolyLevel, &oTristripLevel);
#endif // EX0_CLIENT
		fclose(pFile);
	}
	else
	{
		// level not found
		string sMessage = (string)"an eX0 level file \'" + chFileName + "\' could not be opened.";
#ifdef WIN32
		// TODO: Refactor, put error messages in the right place (outside?)
		//MessageBox(NULL, sMessage.c_str(), "eX0 error", MB_ICONERROR);
#else
		//printf("%s\n", sMessage.c_str());
#endif
		printf("%s (errno=%d)\n", sMessage.c_str(), errno);
		return false;
	}

#ifdef EX0_CLIENT
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
#endif // EX0_CLIENT

	return true;
}

// close currently opened level, free memory, reset vars
void GameDataEndLevel()
{
	gpc_free_polygon(&oPolyLevel);
#ifdef EX0_CLIENT
	gpc_free_tristrip(&oTristripLevel);

	// DEBUG: Close the PolyBoolean level
	PAREA::Del(&pPolyBooleanLevel);
#endif // EX0_CLIENT
}
