struct TextureIDs_t
{
	int iFonts[1];
	int	iFloor;
};

void GameDataLoad(void);

void GameDataUnload(void);

bool GameDataLoadTextures();

// load a level in memory
void GameDataOpenLevel(char *chFileName);

// close currently opened level, free memory, reset vars
void GameDataEndLevel();