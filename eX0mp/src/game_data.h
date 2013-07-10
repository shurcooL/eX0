struct TextureIDs_t
{
	int iFonts[1];
	int	iFloor;
	int nM4A1MuzzleFlash;
};

void GameDataLoad(void);

void GameDataUnload(void);

bool GameDataLoadTextures();

// load a level in memory
void GameDataOpenLevel(const char *chFileName);

// close currently opened level, free memory, reset vars
void GameDataEndLevel();