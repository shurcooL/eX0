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

#include "../../shared/GlobalSettings.h"

///////////////////////
// standard includes //
///////////////////////

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
#include <deque>
#include <GL/glfw.h>	// the glfw header

//#include "mmgr/mmgr.h"	// Fluid Studios Memory Manager


////////////////////
// standard other //
////////////////////
using namespace std;	// so that we can use string class
//#pragma warning(once : 4018 4244 4305 4996)
#pragma warning(disable : 4351)		// Array initializer

// Classes
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
#include "PolyBoolean/polybool.h"
#include "PolyBoolean/pbio.h"
using namespace POLYBOOLEAN;
#include "main.h"
#include "Mgc/MgcIntr2DLinLin.h"
#include "Mgc/MgcDist2DVecLin.h"
using namespace Mgc;
#include "IndexedCircularBuffer.h"
#include "../../shared/ThreadSafeQueue.h"
#include "../../shared/math.h"
#include "../../shared/col_hand.h"
#include "input.h"
#include "render.h"
#include "CHudMessageQueue.h"
#include "ogl_utils.h"
#include "game_data.h"
#include "../../shared/particle.h"
#include "../../shared/weapon.h"
#include "../../shared/PlayerController.h"
#include "../../shared/LocalController.h"
#include "../../shared/NetworkController.h"
#include "../../shared/PlayerStateAuther.h"
#include "../../shared/LocalStateAuther.h"
#include "../../shared/NetworkStateAuther.h"
#include "../../shared/player.h"
//#include "RemoteAuthPlayer.h"
#include "../../shared/Network.h"
#include "../../shared/HashMatcher.h"
#include "../../shared/NetworkConnection.h"
#include "ServerConnection.h"
#include "../../shared/CPacket.h"
#include "../../shared/CTimedEvent.h"
#include "../../shared/CTimedEventScheduler.h"
#include "../../shared/GameLogicThread.h"
#include "OGLTextureManager/TextureManager.h"
#include "../../shared/MovingAverage.h"


//////////////////
// custom other //
//////////////////


//////////////////////
// global variables //
//////////////////////
extern volatile int	iGameState;
extern bool			bPaused;

// DEBUG: Debug state variables
extern bool			bWireframe;
extern bool			bUseDefaultTriangulation;
extern bool			bStencilOperations;

extern GLFWvidmode	oDesktopMode;
extern bool			bFullscreen;
extern float		fMouseSensitivity;
extern bool			bAutoReload;

extern int			nDesktopCursorX, nDesktopCursorY;
extern int			iMouseX, iMouseY;
extern int			iMouseMovedX[MOUSE_FILTERING_SAMPLES], iMouseMovedY[MOUSE_FILTERING_SAMPLES];
//extern int			iMouseMovedX, iMouseMovedY;
extern float		fFilteredMouseMovedX, fFilteredMouseMovedY;
extern int			iMouseButtonsDown;

extern int			nChatMode;
extern string		sChatString;
extern CHudMessageQueue		*pChatMessages;

extern u_int		nPlayerCount;
extern u_int		iLocalPlayerID;
extern CPlayer *	pLocalPlayer;
extern string		sLocalPlayerName;
//extern CPlayer		*oPlayers[32];
//extern float		fPlayerTickTime;

extern int			iCameraType;

extern double		dTimePassed;
extern double		dCurTime, dBaseTime;
extern string		sFpsString;

extern string		sTempString;
extern float		fTempFloat;
extern int			iTempInt;

extern gpc_polygon	oPolyLevel;
extern gpc_tristrip	oTristripLevel;
extern PAREA		*pPolyBooleanLevel;

extern TextureIDs_t	oTextureIDs;
extern CParticle	oParticleEngine;

extern GLUquadricObj *	oQuadricObj;

extern CTimedEventScheduler *	pTimedEventScheduler;
extern GameLogicThread *		pGameLogicThread;

extern ServerConnection *	pServer;

extern GLFWthread		oNetworkThread;
extern volatile bool	bNetworkThreadRun;

extern u_char			g_cCurrentCommandSequenceNumber;
extern double			g_dNextTickTime;
extern IndexedCircularBuffer<Move_t, u_char>	oUnconfirmedMoves;
extern GLFWmutex		oPlayerTick;

extern u_char			g_cCommandRate;
extern u_char			g_cUpdateRate;

extern bool				bSelectTeamDisplay;
extern bool				bSelectTeamReady;

#endif // __globals_H__
