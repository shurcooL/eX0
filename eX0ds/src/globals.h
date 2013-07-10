/*
ch = char
b = bool
n/i = int
f = float
d = double
o = other
p = pointer
*/

#pragma once			// used to optimize compilation times (somehow)
#ifndef __globals_H__		// if we haven't done this where this file can see it, then...
#define __globals_H__		// all header files included

///////////////////////
// standard includes //
///////////////////////

#define NOMINMAX
#include "../../shared/NetworkIncludes.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>	// standard header for ms windows applications
#endif // WIN32

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <string>		// for std::string avaliablity
#include <sstream>		// for string to int and back conversions
#include <time.h>		// for date and time functions (used in log)
#include <math.h>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <GL/glfw.h>	// the glfw header

//#include "mmgr/mmgr.h"	// Fluid Studios Memory Manager


////////////////////
// standard other //
////////////////////
using namespace std;	// so that we can use string class
#pragma warning(once : 4018 4244 4305 4996)

// Classes
class CClient;
class CPlayer;
class CPacket;
class CTimedEvent;
class CTimedEventScheduler;

/////////////////////
// custom includes //
/////////////////////
extern "C"
{
#include "gpc/gpc.h"		// the gpc library
}
/*#include "PolyBoolean/polybool.h"
#include "PolyBoolean/pbio.h"
using namespace POLYBOOLEAN;*/
#include "Main.h"
#include "Mgc/MgcIntr2DLinLin.h"
#include "Mgc/MgcDist2DVecLin.h"
using namespace Mgc;
#include "../../shared/math.h"
/*#include "input.h"
#include "render.h"
#include "ogl_utils.h"*/
#include "../../shared/col_hand.h"
#include "game_data.h"
#include "particle.h"
#include "weapon.h"
#include "player.h"
#include "../../shared/Network.h"
#include "../../shared/CPacket.h"
#include "Server.h"
#include "../../shared/HashMatcher.h"
#include "CClient.h"
#include "../../shared/CTimedEvent.h"
#include "../../shared/CTimedEventScheduler.h"
//#include "OGLTextureManager/TextureManager.h"


//////////////////
// custom other //
//////////////////


//////////////////////
// global variables //
//////////////////////
/*extern int			iGameState;
extern bool			bPaused;

// DEBUG: Debug state variables
extern bool			bWireframe;
extern bool			bUseDefaultTriangulation;
extern bool			bStencilOperations;

extern GLFWvidmode	oDesktopMode;
extern bool			bFullscreen;
extern float		fMouseSensitivity;
extern bool			bAutoReload;

extern int			iCursorX, iCursorY;
extern int			iMouseX, iMouseY;
extern int			iMouseMovedX[MOUSE_FILTERING_SAMPLES], iMouseMovedY[MOUSE_FILTERING_SAMPLES];
//extern int			iMouseMovedX, iMouseMovedY;
extern float		fFilteredMouseMovedX, fFilteredMouseMovedY;
extern int			iMouseButtonsDown;

extern int			nChatMode;
extern string		sChatString;*/

extern int			nPlayerCount;
//extern float		fPlayerTickTime;
//extern int			iLocalPlayerID;
//extern CPlayer		*oPlayers[32];

/*extern int			iCameraType;

extern float		fTimePassed;
extern float		fCurTime, fBaseTime;
extern int			iFpsFrames;
extern float		fFpsTimePassed, fFpsBaseTime;
extern string		sFpsString;

extern string		sTempString;
extern float		fTempFloat;
extern int			iTempInt;*/

extern string		sLevelName;
extern gpc_polygon	oPolyLevel;
//extern gpc_tristrip	oTristripLevel;
//extern PAREA		*pPolyBooleanLevel;

//extern TextureIDs_t	oTextureIDs;
extern CParticle	oParticleEngine;

extern CTimedEventScheduler	*pTimedEventScheduler;

//extern GLUquadricObj	*oQuadricObj;

extern SOCKET		nUdpSocket;

extern GLFWthread		oServerThread;
extern volatile bool	bServerThreadRun;

extern GLFWmutex		oTcpSendMutex;
extern GLFWmutex		oUdpSendMutex;
extern GLFWmutex		oPlayerTick;

#endif // __globals_H__
