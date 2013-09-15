# Linux version

SRC_PATH := src/

CC     = g++
CC2	= gcc
#CC		= clang++
#CC2		= clang

CFLAGS = -I./include/ -I/usr/X11R6/include \
	-DEX0_CLIENT -D_DEBUG \
	-O0 -g -pthread -Wall \
	-Wno-unknown-pragmas -Wno-comment \
	-m32							# Force 32-bit on 64-bit linux
# Old
# -ffast-math
CPPFLAGS = -std=c++0x
LFLAGS = ./libglfw.linux.a \
	-L/usr/X11R6/lib -lGL -lGLU -lX11 -lXrandr -lpthread -lm \
	-L/usr/lib/i386-linux-gnu		# Force 32-bit on 64-bit linux
# Old
# -lXxf86vm -lXext

OBJS   = $(SRC_PATH)GameTimer.cpp $(SRC_PATH)GameSession.cpp $(SRC_PATH)RemoteClientConnection.cpp \
$(SRC_PATH)HidController.cpp $(SRC_PATH)InputManager.cpp $(SRC_PATH)InputListener.cpp $(SRC_PATH)PlayerInputListener.cpp \
$(SRC_PATH)WeaponSystem.cpp $(SRC_PATH)col_hand.cpp

# Rule for eX0
eX0: $(OBJS) $(SRC_PATH)game_data.cpp $(SRC_PATH)input.cpp $(SRC_PATH)main.cpp \
$(SRC_PATH)math.cpp $(SRC_PATH)ogl_utils.cpp $(SRC_PATH)particle.cpp $(SRC_PATH)player.cpp \
$(SRC_PATH)render.cpp \
$(SRC_PATH)NetworkConnection.cpp $(SRC_PATH)ServerConnection.cpp $(SRC_PATH)ClientConnection.cpp \
$(SRC_PATH)LocalServerConnection.cpp $(SRC_PATH)LocalClientConnection.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp $(SRC_PATH)gpc/gpc.o $(SRC_PATH)OGLTextureManager/TextureManager.cpp \
$(SRC_PATH)PolyBoolean/pbgeom.cpp $(SRC_PATH)PolyBoolean/polybool.cpp \
$(SRC_PATH)PolyBoolean/pbio.cpp $(SRC_PATH)PolyBoolean/triacons.cpp \
$(SRC_PATH)PolyBoolean/pbpolys.cpp $(SRC_PATH)PolyBoolean/triamono.cpp \
$(SRC_PATH)PolyBoolean/pbsweep.cpp $(SRC_PATH)Network.cpp $(SRC_PATH)ServerNetwork.cpp $(SRC_PATH)CPacket.cpp \
$(SRC_PATH)CTimedEvent.cpp $(SRC_PATH)CTimedEventScheduler.cpp $(SRC_PATH)CHudMessageQueue.cpp \
$(SRC_PATH)MovingAverage.cpp \
$(SRC_PATH)GameLogicThread.cpp $(SRC_PATH)PlayerStateAuther.cpp $(SRC_PATH)NetworkStateAuther.cpp \
$(SRC_PATH)PlayerController.cpp $(SRC_PATH)LocalController.cpp $(SRC_PATH)AiController.cpp \
$(SRC_PATH)Thread.cpp $(SRC_PATH)GameServer.cpp $(SRC_PATH)FpsCounter.cpp \
$(SRC_PATH)NetworkController.cpp $(SRC_PATH)LocalStateAuther.cpp $(SRC_PATH)ThroughputMonitor.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SRC_PATH)game_data.cpp \
$(OBJS) $(SRC_PATH)input.cpp $(SRC_PATH)main.cpp \
$(SRC_PATH)math.cpp $(SRC_PATH)ogl_utils.cpp $(SRC_PATH)particle.cpp $(SRC_PATH)player.cpp \
$(SRC_PATH)render.cpp \
$(SRC_PATH)NetworkConnection.cpp $(SRC_PATH)ServerConnection.cpp $(SRC_PATH)ClientConnection.cpp \
$(SRC_PATH)LocalServerConnection.cpp $(SRC_PATH)LocalClientConnection.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp $(SRC_PATH)gpc/gpc.o $(SRC_PATH)OGLTextureManager/TextureManager.cpp \
$(SRC_PATH)PolyBoolean/pbgeom.cpp $(SRC_PATH)PolyBoolean/polybool.cpp \
$(SRC_PATH)PolyBoolean/pbio.cpp $(SRC_PATH)PolyBoolean/triacons.cpp \
$(SRC_PATH)PolyBoolean/pbpolys.cpp $(SRC_PATH)PolyBoolean/triamono.cpp \
$(SRC_PATH)PolyBoolean/pbsweep.cpp $(SRC_PATH)Network.cpp $(SRC_PATH)ServerNetwork.cpp $(SRC_PATH)CPacket.cpp \
$(SRC_PATH)CTimedEvent.cpp $(SRC_PATH)CTimedEventScheduler.cpp $(SRC_PATH)CHudMessageQueue.cpp \
$(SRC_PATH)MovingAverage.cpp \
$(SRC_PATH)GameLogicThread.cpp $(SRC_PATH)PlayerStateAuther.cpp $(SRC_PATH)NetworkStateAuther.cpp \
$(SRC_PATH)PlayerController.cpp $(SRC_PATH)LocalController.cpp $(SRC_PATH)AiController.cpp \
$(SRC_PATH)Thread.cpp $(SRC_PATH)GameServer.cpp $(SRC_PATH)FpsCounter.cpp \
$(SRC_PATH)NetworkController.cpp $(SRC_PATH)LocalStateAuther.cpp $(SRC_PATH)ThroughputMonitor.cpp \
$(LFLAGS) -o $@

$(SRC_PATH)gpc/gpc.o: $(SRC_PATH)gpc/gpc.c
	$(CC2) -c $(CFLAGS) $(SRC_PATH)gpc/gpc.c -o $@
