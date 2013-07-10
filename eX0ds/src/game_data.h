struct TextureIDs_t
{
	int iFonts[1];
	int	iFloor;
};

bool GameDataLoad(void);

void GameDataUnload(void);

bool GameDataLoadTextures(void);

// load a level in memory
bool GameDataOpenLevel(const char *chFileName);

// close currently opened level, free memory, reset vars
void GameDataEndLevel();
