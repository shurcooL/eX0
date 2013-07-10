SRC_PATH := src/
SHARED_PATH := ../shared/

CC     = g++
CFLAGS = -I../../../Libraries/glfw/include -I/usr/X11R6/include \
	-O0 -g -ffast-math -pthread -Wall \
	-DEX0_CLIENT
LFLAGS = ../../../Libraries/glfw/lib/x11/libglfw.a -L/usr/X11R6/lib -lGL -lGLU \
	-lX11 -lXrandr -lpthread -lm
# -lXxf86vm -lXext

# Rule for eX0
eX0: $(SHARED_PATH)col_hand.cpp $(SRC_PATH)game_data.cpp $(SRC_PATH)input.cpp $(SRC_PATH)main.cpp \
$(SHARED_PATH)math.cpp $(SRC_PATH)ogl_utils.cpp $(SHARED_PATH)particle.cpp $(SHARED_PATH)player.cpp $(SRC_PATH)render.cpp $(SHARED_PATH)weapon.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp $(SRC_PATH)gpc/gpc.o $(SRC_PATH)OGLTextureManager/TextureManager.cpp \
$(SRC_PATH)PolyBoolean/pbgeom.cpp $(SRC_PATH)PolyBoolean/polybool.cpp \
$(SRC_PATH)PolyBoolean/pbio.cpp $(SRC_PATH)PolyBoolean/triacons.cpp \
$(SRC_PATH)PolyBoolean/pbpolys.cpp $(SRC_PATH)PolyBoolean/triamono.cpp \
$(SRC_PATH)PolyBoolean/pbsweep.cpp $(SRC_PATH)Network.cpp $(SHARED_PATH)CPacket.cpp \
$(SHARED_PATH)CTimedEvent.cpp $(SHARED_PATH)CTimedEventScheduler.cpp $(SRC_PATH)CHudMessageQueue.cpp \
$(SHARED_PATH)MovingAverage.cpp
	$(CC) $(CFLAGS) $(SHARED_PATH)col_hand.cpp $(SRC_PATH)game_data.cpp \
$(SRC_PATH)input.cpp $(SRC_PATH)main.cpp \
$(SHARED_PATH)math.cpp $(SRC_PATH)ogl_utils.cpp $(SHARED_PATH)particle.cpp $(SHARED_PATH)player.cpp $(SRC_PATH)render.cpp $(SHARED_PATH)weapon.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp $(SRC_PATH)gpc/gpc.o $(SRC_PATH)OGLTextureManager/TextureManager.cpp \
$(SRC_PATH)PolyBoolean/pbgeom.cpp $(SRC_PATH)PolyBoolean/polybool.cpp \
$(SRC_PATH)PolyBoolean/pbio.cpp $(SRC_PATH)PolyBoolean/triacons.cpp \
$(SRC_PATH)PolyBoolean/pbpolys.cpp $(SRC_PATH)PolyBoolean/triamono.cpp \
$(SRC_PATH)PolyBoolean/pbsweep.cpp $(SRC_PATH)Network.cpp $(SHARED_PATH)CPacket.cpp \
$(SHARED_PATH)CTimedEvent.cpp $(SHARED_PATH)CTimedEventScheduler.cpp $(SRC_PATH)CHudMessageQueue.cpp \
$(SHARED_PATH)MovingAverage.cpp \
$(LFLAGS) -o $@

$(SRC_PATH)gpc/gpc.o: $(SRC_PATH)gpc/gpc.c
	gcc -c $(CFLAGS) $(SRC_PATH)gpc/gpc.c -o $@
