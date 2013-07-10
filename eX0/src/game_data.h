struct TextureIDs_t
{
	int iFonts[2];
	int	iFloor;
	int nM4A1MuzzleFlash;
};

bool GameDataLoad(void);

void GameDataUnload(void);

bool GameDataLoadTextures(void);

// load a level in memory
bool GameDataOpenLevel(const char *chFileName);

// close currently opened level, free memory, reset vars
void GameDataEndLevel(void);
