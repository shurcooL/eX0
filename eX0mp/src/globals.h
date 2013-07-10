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

#ifndef _globals_H_		// if we haven't done this where this file can see it, then...
#define _globals_H_		// all header files included

///////////////////////
// standard includes //
///////////////////////
//#include "MMGR/nommgr.h"	// first disable the Fluid Studios Memory Manager

#include "NetworkIncludes.h"

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
#include <deque>
#include <GL/glfw.h>	// the glfw header

#include "mmgr/mmgr.h"	// Fluid Studios Memory Manager


////////////////////
// standard other //
////////////////////
using namespace std;	// so that we can use string class
#pragma warning(once : 4244 4305 4996)


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
#include "math.h"
#include "col_hand.h"
#include "input.h"
#include "render.h"
#include "ogl_utils.h"
#include "game_data.h"
#include "particle.h"
#include "weapon.h"
#include "player.h"
#include "Network.h"
#include "Packet.h"
#include "OGLTextureManager/TextureManager.h"


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

extern int			iCursorX, iCursorY;
extern int			iMouseX, iMouseY;
extern int			iMouseMovedX[MOUSE_FILTERING_SAMPLES], iMouseMovedY[MOUSE_FILTERING_SAMPLES];
//extern int			iMouseMovedX, iMouseMovedY;
extern float		fFilteredMouseMovedX, fFilteredMouseMovedY;
extern int			iMouseButtonsDown;

extern int			nChatMode;
extern string		sChatString;

extern int			nPlayerCount;
extern int			iLocalPlayerID;
extern string		sLocalPlayerName;
extern CPlayer		*oPlayers[32];

extern int			iCameraType;

extern float		fTimePassed;
extern float		fCurTime, fBaseTime;
extern int			iFpsFrames;
extern float		fFpsTimePassed, fFpsBaseTime;
extern string		sFpsString;

extern string		sTempString;
extern float		fTempFloat;
extern int			iTempInt;

extern gpc_polygon	oPolyLevel;
extern gpc_tristrip	oTristripLevel;
extern PAREA		*pPolyBooleanLevel;

extern TextureIDs_t	oTextureIDs;
extern CParticle	oParticleEngine;

extern GLUquadricObj	*oQuadricObj;

extern SOCKET			nServerTcpSocket;
extern SOCKET			nServerUdpSocket;
extern volatile int		nJoinStatus;

extern GLFWthread		oNetworkThread;
extern volatile bool	bNetworkThreadRun;

extern u_char			cLocalMovementSequenceNumber;
extern u_char			cRemoteUpdateSequenceNumber;
extern IndexedCircularBuffer<Move_t>	oUnconfirmedMoves;
extern GLFWmutex		oPlayerTick;

extern float			fLastLatency;
extern float			fPingPacketTime;
extern int				nPingPacketNumber;

#endif // _globals_H_
