#include "TextureManager.h"
#include <string.h>
#include <stdio.h>

//#include <iostream.h>	// DEBUG ONLY

#ifdef WIN32
#include <windows.h>
#else
#include <ctype.h>
#endif // WIN32

#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glfw.h>

/**
	Note : Being a singleton all the data we 'want' is located
	in CTextureManager::m_Singleton, so although it looks really
	ugly to have so many 'm_Singleton->'s this is so the code will
	actually work as designed ;)
**/

CTextureManager *CTextureManager::m_Singleton = 0;
// ===================================================================
/**
No use for a constructor because this singlton is never created,
when GetSingleton is called for the first time and issued a 'new'
command the constructor is called, however because of the memory
isn't valid until AFTER the constructor it just is a bad idea...
Use Initialize and Destroy for your dirty work
**/
CTextureManager::CTextureManager (void) {
	// This is just to be clean, but all 'real' data
	// is in m_Singleton
	
	szErrorMessage [0] = '\0';
	nNumTextures	   = 0;
	nAvailable     	   = 0;
	nTexIDs            = 0;
}

CTextureManager::~CTextureManager (void) {

}

CTextureManager &CTextureManager::GetSingleton (void) {
	if (!m_Singleton) {
		m_Singleton = new CTextureManager;
		Initialize ();
	}

	return *m_Singleton;
}

void CTextureManager::Initialize (void) {
	sprintf (m_Singleton->szErrorMessage, "Texture Manager Initialized!");

	m_Singleton->nNumTextures = 0;
	m_Singleton->nAvailable   = INITIAL_SIZE;
	m_Singleton->nTexIDs      = new int [INITIAL_SIZE];
	
	for (int i = 0; i < m_Singleton->nAvailable; i++) {
		m_Singleton->nTexIDs [i] = -1;
	}
}

void CTextureManager::Destroy (void) {
	if (m_Singleton) {
		delete [] m_Singleton->nTexIDs;
		m_Singleton->nTexIDs = 0;
	
		delete m_Singleton;
		m_Singleton = 0;
	}
}

// ===================================================================

int CTextureManager::LoadTexture (const char *szFilename, int nTextureID) {
	sprintf (m_Singleton->szErrorMessage, "Beginning to Loading [%s]", szFilename);

	int nWidth = 0, nHeight = 0, nBPP = 0;
	UBYTE *pData = 0;
	
	// Determine the type and actually load the file
	// ===========================================================================================
	char szCapFilename [80];
	int nLen = strlen (szFilename);
	for (int c = 0; c <= nLen; c++)	// <= to include the NULL as well
		szCapFilename [c] = toupper (szFilename [c]);
	
	if (strcmp (szCapFilename + (nLen - 3), "BMP") == 0 ||
		strcmp (szCapFilename + (nLen - 3), "TGA") == 0) {
		// Actually load them
		if (strcmp (szCapFilename + (nLen - 3), "BMP") == 0) {
#ifdef WIN32
			pData = LoadBitmapFile (szFilename, nWidth, nHeight, nBPP);
#endif
			if (pData == 0)
				return -1;
		}
		if (strcmp (szCapFilename + (nLen - 3), "TGA") == 0) {
			pData = LoadTargaFile (szFilename, nWidth, nHeight, nBPP);
			if (pData == 0)
				return -1;
		}
	}
	else {
		sprintf (m_Singleton->szErrorMessage, "ERROR : Unable to load extension [%s]", szCapFilename + (nLen - 3));
		return -1;
	}

	// Assign a valid Texture ID (if one wasn't specified)
	// ===========================================================================================
	int nNewTextureID = GetNewTextureID (nTextureID);	// Also increases nNumTextures!

	// ===========================================================================================

	// Register and upload the texture in OpenGL
	glBindTexture (GL_TEXTURE_2D, nNewTextureID);

	// NOTE : Making some assumptions about texture parameters
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	//	You can use this one if you don't want to eat memory and Mip-mapped textures
	/*glTexImage2D (GL_TEXTURE_2D,  0, nGLFormat,
		  		 nWidth, nHeight, 0, nGLFormat,
				 GL_UNSIGNED_BYTE, pData);*/
	glTexImage2D (GL_TEXTURE_2D,  0, nBPP,
		  		 nWidth, nHeight, 0, (nBPP == 3 ? GL_RGB : GL_RGBA),
				 GL_UNSIGNED_BYTE, pData);

	/*gluBuild2DMipmaps (GL_TEXTURE_2D,
					   nBPP, nWidth, nHeight,
					   (nBPP == 3 ? GL_RGB : GL_RGBA),
					   GL_UNSIGNED_BYTE,
					   pData);*/

	delete [] pData;

	sprintf (m_Singleton->szErrorMessage, "Loaded [%s] W/O a hitch!", szFilename);
	return nNewTextureID;
}

int CTextureManager::LoadTextureFromMemory (UBYTE *pData, int nWidth, int nHeight, int nBPP, int nTextureID) {

	// First we do ALOT of error checking on the data...
	if (!CheckSize (nWidth) || !CheckSize (nHeight)) {
		sprintf (m_Singleton->szErrorMessage, "ERROR : Improper Dimension");
		return -1;
	}
	if (nBPP != 3 && nBPP != 4) {
		sprintf (m_Singleton->szErrorMessage, "ERROR : Unsuported Color Depth");
		return -1;
	}

	// I guess were good to go...
	// ---------------------------------------------------------------------
	int nNewTextureID = GetNewTextureID (nTextureID);	// Also increases nNumTextures!

	// Register and upload the texture in OpenGL
	glBindTexture (GL_TEXTURE_2D, nNewTextureID);

	// NOTE : Making some assumptions about texture parameters
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps (GL_TEXTURE_2D,
					   nBPP, nWidth, nHeight,
					   (nBPP == 3 ? GL_RGB : GL_RGBA),
					   GL_UNSIGNED_BYTE,
					   pData);
	// ---------------------------------------------------------------------

	 // delete [] pData;	// Leave memory clearing upto the caller...

	sprintf (m_Singleton->szErrorMessage, "Loaded Some Memory Perfectly!");
	return nNewTextureID;
}

void CTextureManager::FreeTexture (int nID) {
	int nIndex = -1;
	for (int i = 0; i < m_Singleton->nAvailable; i++) {
		if (m_Singleton->nTexIDs [i] == nID) {
			m_Singleton->nTexIDs [i] = -1;
			nIndex = i;	// to indicate a match was found
			break;		// their _should_ only be one instance of nID (if any)
		}
	}

	if (nIndex != -1) {
		unsigned int uiGLID = (unsigned int) nID;
		glDeleteTextures (1, &uiGLID);
	}
}

void CTextureManager::FreeAll (void) {
	
	// copy the ids to an unsigned integer array, so GL will like it ;)
	unsigned int *pUIIDs = new unsigned int [m_Singleton->nNumTextures];
	int i, j;
	for (i = 0, j = 0; i < m_Singleton->nNumTextures; i++) {
		if (m_Singleton->nTexIDs [i] != -1) {
			pUIIDs [j] = m_Singleton->nTexIDs [i];
			j++;
		}
	}

	glDeleteTextures (m_Singleton->nNumTextures, pUIIDs);

	delete [] pUIIDs;
	delete [] m_Singleton->nTexIDs;
	m_Singleton->nTexIDs = new int [INITIAL_SIZE];
	m_Singleton->nAvailable = INITIAL_SIZE;
	for (i = 0; i < INITIAL_SIZE; i++)
		m_Singleton->nTexIDs [i] = -1;
	
	m_Singleton->nNumTextures = 0;
}

// ===================================================================

#ifdef WIN32
UBYTE *CTextureManager::LoadBitmapFile (const char *filename, int &nWidth, int &nHeight, int &nBPP) {
	

	// These are both defined in Windows.h
	BITMAPFILEHEADER	BitmapFileHeader;
	BITMAPINFOHEADER	BitmapInfoHeader;
	
	// Old Skool C-style code	
	FILE	*pFile;
	UBYTE	*pImage;			// bitmap image data
	UBYTE	tempRGB;				// swap variable

	// open filename in "read binary" mode
	pFile = fopen(filename, "rb");
	if (pFile == 0) {
		
		sprintf (m_Singleton->szErrorMessage, "ERROR : [%s] File Not Found!", filename);	
		return 0;
	}

	// Header
	fread (&BitmapFileHeader, sizeof (BITMAPFILEHEADER), 1, pFile);
	if (BitmapFileHeader.bfType != 'MB') {
		
		sprintf (m_Singleton->szErrorMessage, "ERROR : [%s] Is not a valid Bitmap!", filename);
		fclose (pFile);
		return 0;
	}

	// Information
	fread (&BitmapInfoHeader, sizeof (BITMAPINFOHEADER), 1, pFile);

	if (!CheckSize (BitmapInfoHeader.biWidth) || !CheckSize (BitmapInfoHeader.biHeight)) {

		sprintf (m_Singleton->szErrorMessage, "ERROR : Improper Dimension");
		fclose (pFile);
		return 0;
	}

	
	fseek (pFile, BitmapFileHeader.bfOffBits, SEEK_SET);
	pImage = new UBYTE [BitmapInfoHeader.biSizeImage];
	if (!pImage) {
		delete [] pImage;
		
		sprintf (m_Singleton->szErrorMessage, "ERROR : Out Of Memory!");

		fclose (pFile);
		return 0;
	}
	fread (pImage, 1, BitmapInfoHeader.biSizeImage, pFile);

	// Turn BGR to RBG
	for (int i = 0; i < (int) BitmapInfoHeader.biSizeImage; i += 3) {
		tempRGB = pImage [i];
		pImage [i + 0] = pImage [i + 2];
		pImage [i + 2] = tempRGB;
	}

	fclose(pFile);

	// THIS IS CRUCIAL!  The only way to relate the size information to the
	// OpenGL functions back in ::LoadTexture ()
	nWidth  = BitmapInfoHeader.biWidth;
	nHeight = BitmapInfoHeader.biHeight;
	nBPP    = 3;	// Only load 24-bit Bitmaps

	return pImage;
}
#endif // WIN32

UBYTE *CTextureManager::LoadTargaFile (const char *filename, int &nWidth, int &nHeight, int &nBPP) {

	// Get those annoying data structures out of the way...
	struct {
		unsigned char imageTypeCode;
		short int imageWidth;
		short int imageHeight;
		unsigned char bitCount;
	} TgaHeader;

	// Let 'er rip!
	FILE	*pFile;
	UBYTE	uCharDummy;
	short	sIntDummy;
	UBYTE	colorSwap;	// swap variable
	UBYTE	*pImage;	// the TGA data

	// open the TGA file
	pFile = fopen (filename, "rb");
	if (!pFile) {
	
		sprintf (m_Singleton->szErrorMessage, "ERROR : [%s] File Not Found!", filename);
		return 0;
	}

	// Ignore the first two bytes
	fread (&uCharDummy, sizeof (UBYTE), 1, pFile);
	fread (&uCharDummy, sizeof (UBYTE), 1, pFile);

	// Pop in the header
	fread(&TgaHeader.imageTypeCode, sizeof (unsigned char), 1, pFile);

	// Only loading RGB and RGBA types
	if ((TgaHeader.imageTypeCode != 2) && (TgaHeader.imageTypeCode != 3)) {

		sprintf (m_Singleton->szErrorMessage, "ERROR : Unsuported Image Type (Color Depth or Compression)");
		fclose (pFile);
		return 0;
	}

	// More data which isn't important for now
	fread (&uCharDummy, sizeof (unsigned char), 1, pFile);
	fread (&sIntDummy,  sizeof (short), 1, pFile);
	fread (&sIntDummy,  sizeof (short), 1, pFile);
	fread (&sIntDummy,  sizeof (short), 1, pFile);
	fread (&sIntDummy,  sizeof (short), 1, pFile);

	// Get some rather important data
	fread (&TgaHeader.imageWidth,  sizeof (short int), 1, pFile);
	fread (&TgaHeader.imageHeight, sizeof (short int), 1, pFile);
	fread (&TgaHeader.bitCount, sizeof (unsigned char), 1, pFile);

	// Skip past some more
	fread (&uCharDummy, sizeof (unsigned char), 1, pFile);

	// THIS IS CRUCIAL
	nBPP    = TgaHeader.bitCount / 8;
	nWidth  = TgaHeader.imageWidth;
	nHeight = TgaHeader.imageHeight;
	

	if (!CheckSize (nWidth) || !CheckSize (nHeight)) {

		sprintf (m_Singleton->szErrorMessage, "ERROR : Improper Dimension");
		fclose (pFile);
		return 0;
	}


	int nImageSize = nWidth * nHeight * nBPP;
	pImage = new UBYTE [nImageSize];
	if (pImage == 0) {
		
		sprintf (m_Singleton->szErrorMessage, "ERROR : Out Of Memory");
		return 0;
	}

	// actually read it (finally!)
	fread (pImage, sizeof (UBYTE), nImageSize, pFile);

	// BGR to RGB
	for (int i = 0; i < nImageSize; i += nBPP) {
		colorSwap = pImage [i + 0];
		pImage [i + 0] = pImage [i + 2];
		pImage [i + 2] = colorSwap;
	}
	fclose (pFile);

	return pImage;
}

int CTextureManager::GetNewTextureID (int nPossibleTextureID) {

	// First check if the possible textureID has already been
	// used, however the default value is -1, err that is what
	// this method is passed from LoadTexture ()
	if (nPossibleTextureID != -1) {
		for (int i = 0; i < m_Singleton->nAvailable; i++) {
			if (m_Singleton->nTexIDs [i] == nPossibleTextureID) {
				FreeTexture (nPossibleTextureID);	// sets nTexIDs [i] to -1...
				m_Singleton->nNumTextures--;		// since we will add the ID again...
				break;
			}
		}
	}

	// Actually look for a new one
	int nNewTextureID;
	if (nPossibleTextureID == -1) {
		unsigned int nGLID;	
		glGenTextures (1, &nGLID);
		nNewTextureID = (int) nGLID;
	}
	else	// If the user is handle the textureIDs
		nNewTextureID = nPossibleTextureID;
	
	// find an empty slot in the TexID array
	int nIndex = 0;
	while (m_Singleton->nTexIDs [nIndex] != -1 && nIndex < m_Singleton->nAvailable)
		nIndex++;

	// all space exaused, make MORE!
	if (nIndex >= m_Singleton->nAvailable) {
		int *pNewIDs = new int [m_Singleton->nAvailable + TEXTURE_STEP];
		int i;
		
		// copy the old
		for (i = 0; i < m_Singleton->nAvailable; i++)
			pNewIDs [i] = m_Singleton->nTexIDs [i];
		// set the last increment to the newest ID
		pNewIDs [m_Singleton->nAvailable] = nNewTextureID;
		// set the new to '-1'
		for (i = 1; i < TEXTURE_STEP; i++)
			pNewIDs [i + m_Singleton->nAvailable] = -1;

		m_Singleton->nAvailable += TEXTURE_STEP;
		delete [] m_Singleton->nTexIDs;
		m_Singleton->nTexIDs = pNewIDs;
	}
	else
		m_Singleton->nTexIDs [nIndex] = nNewTextureID;

	// Welcome to our Texture Array!
	m_Singleton->nNumTextures++;
	return nNewTextureID;
}

// ===================================================================

char *CTextureManager::GetErrorMessage (void) {
	return m_Singleton->szErrorMessage;
}

bool CTextureManager::CheckSize (int nDimension) {
	// Portability issue, check your endian...

	int i = 1;
	while (i < nDimension) {
		i <<= 1;
		if (i == nDimension)
			return true;
	}

	return false;
}

int	CTextureManager::GetNumTextures (void) {
	return m_Singleton->nNumTextures;
}

int CTextureManager::GetAvailableSpace (void) {
	return m_Singleton->nAvailable;
}

int CTextureManager::GetTexID (int nIndex) {
	if (nIndex >= 0 && nIndex < m_Singleton->nAvailable)
		return m_Singleton->nTexIDs [nIndex];

	// else
	return 0;
}