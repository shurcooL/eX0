SRC_PATH := src/
SHARED_PATH := ../shared/

CC     = g++
CFLAGS = -I../../../Libraries/glfw/include -I/usr/X11R6/include \
	-DEX0_CLIENT -D_DEBUG \
	-O0 -g -pthread -Wall
# -ffast-math
LFLAGS = ../../../Libraries/glfw/lib/x11/libglfw.a -L/usr/X11R6/lib -lGL -lGLU \
	-lX11 -lXrandr -lpthread -lm
# -lXxf86vm -lXext
OBJS   = $(SHARED_PATH)GameTimer.cpp $(SHARED_PATH)GameSession.cpp $(SHARED_PATH)RemoteClientConnection.cpp \
$(SRC_PATH)HidController.cpp $(SRC_PATH)InputManager.cpp $(SRC_PATH)InputListener.cpp $(SRC_PATH)PlayerInputListener.cpp

# Rule for eX0
eX0: $(OBJS) $(SHARED_PATH)col_hand.cpp $(SHARED_PATH)game_data.cpp $(SRC_PATH)input.cpp $(SRC_PATH)main.cpp \
$(SHARED_PATH)math.cpp $(SRC_PATH)ogl_utils.cpp $(SHARED_PATH)particle.cpp $(SHARED_PATH)player.cpp \
$(SRC_PATH)render.cpp $(SHARED_PATH)weapon.cpp \
$(SHARED_PATH)NetworkConnection.cpp $(SRC_PATH)ServerConnection.cpp $(SHARED_PATH)ClientConnection.cpp \
$(SRC_PATH)LocalServerConnection.cpp $(SHARED_PATH)LocalClientConnection.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp $(SRC_PATH)gpc/gpc.o $(SRC_PATH)OGLTextureManager/TextureManager.cpp \
$(SRC_PATH)PolyBoolean/pbgeom.cpp $(SRC_PATH)PolyBoolean/polybool.cpp \
$(SRC_PATH)PolyBoolean/pbio.cpp $(SRC_PATH)PolyBoolean/triacons.cpp \
$(SRC_PATH)PolyBoolean/pbpolys.cpp $(SRC_PATH)PolyBoolean/triamono.cpp \
$(SRC_PATH)PolyBoolean/pbsweep.cpp $(SRC_PATH)Network.cpp $(SHARED_PATH)ServerNetwork.cpp $(SHARED_PATH)CPacket.cpp \
$(SHARED_PATH)CTimedEvent.cpp $(SHARED_PATH)CTimedEventScheduler.cpp $(SRC_PATH)CHudMessageQueue.cpp \
$(SHARED_PATH)MovingAverage.cpp \
$(SHARED_PATH)GameLogicThread.cpp $(SHARED_PATH)PlayerStateAuther.cpp $(SHARED_PATH)NetworkStateAuther.cpp \
$(SHARED_PATH)PlayerController.cpp $(SHARED_PATH)LocalController.cpp $(SHARED_PATH)AiController.cpp \
$(SHARED_PATH)Thread.cpp $(SHARED_PATH)GameServer.cpp $(SHARED_PATH)FpsCounter.cpp \
$(SHARED_PATH)NetworkController.cpp $(SHARED_PATH)LocalStateAuther.cpp
	$(CC) $(CFLAGS) $(SHARED_PATH)col_hand.cpp $(SHARED_PATH)game_data.cpp \
$(OBJS) $(SRC_PATH)input.cpp $(SRC_PATH)main.cpp \
$(SHARED_PATH)math.cpp $(SRC_PATH)ogl_utils.cpp $(SHARED_PATH)particle.cpp $(SHARED_PATH)player.cpp \
$(SRC_PATH)render.cpp $(SHARED_PATH)weapon.cpp \
$(SHARED_PATH)NetworkConnection.cpp $(SRC_PATH)ServerConnection.cpp $(SHARED_PATH)ClientConnection.cpp \
$(SRC_PATH)LocalServerConnection.cpp $(SHARED_PATH)LocalClientConnection.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp $(SRC_PATH)gpc/gpc.o $(SRC_PATH)OGLTextureManager/TextureManager.cpp \
$(SRC_PATH)PolyBoolean/pbgeom.cpp $(SRC_PATH)PolyBoolean/polybool.cpp \
$(SRC_PATH)PolyBoolean/pbio.cpp $(SRC_PATH)PolyBoolean/triacons.cpp \
$(SRC_PATH)PolyBoolean/pbpolys.cpp $(SRC_PATH)PolyBoolean/triamono.cpp \
$(SRC_PATH)PolyBoolean/pbsweep.cpp $(SRC_PATH)Network.cpp $(SHARED_PATH)ServerNetwork.cpp $(SHARED_PATH)CPacket.cpp \
$(SHARED_PATH)CTimedEvent.cpp $(SHARED_PATH)CTimedEventScheduler.cpp $(SRC_PATH)CHudMessageQueue.cpp \
$(SHARED_PATH)MovingAverage.cpp \
$(SHARED_PATH)GameLogicThread.cpp $(SHARED_PATH)PlayerStateAuther.cpp $(SHARED_PATH)NetworkStateAuther.cpp \
$(SHARED_PATH)PlayerController.cpp $(SHARED_PATH)LocalController.cpp $(SHARED_PATH)AiController.cpp \
$(SHARED_PATH)Thread.cpp $(SHARED_PATH)GameServer.cpp $(SHARED_PATH)FpsCounter.cpp \
$(SHARED_PATH)NetworkController.cpp $(SHARED_PATH)LocalStateAuther.cpp \
$(LFLAGS) -o $@

$(SRC_PATH)gpc/gpc.o: $(SRC_PATH)gpc/gpc.c
	gcc -c $(CFLAGS) $(SRC_PATH)gpc/gpc.c -o $@
