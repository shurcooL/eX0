SRC_PATH := src/

CC     = g++
CFLAGS = -I../glfw/include -I../../glfw/include -I/usr/X11R6/include \
	-O3 -ffast-math -w
LFLAGS = -L../glfw/lib/x11 -L../../glfw/lib/x11 -L/usr/X11R6/lib -lglfw -lGL -lGLU
# -lX11 -lXxf86vm -lXext -lpthread -lm

# Rule for eX0
eX0: $(SRC_PATH)mmgr/mmgr.cpp $(SRC_PATH)col_hand.cpp $(SRC_PATH)game_data.cpp $(SRC_PATH)input.cpp $(SRC_PATH)main.cpp \
$(SRC_PATH)math.cpp $(SRC_PATH)ogl_utils.cpp $(SRC_PATH)particle.cpp $(SRC_PATH)player.cpp $(SRC_PATH)render.cpp $(SRC_PATH)weapon.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp $(SRC_PATH)gpc/gpc.o $(SRC_PATH)OGLTextureManager/TextureManager.cpp \
$(SRC_PATH)PolyBoolean/pbgeom.cpp $(SRC_PATH)PolyBoolean/polybool.cpp \
$(SRC_PATH)PolyBoolean/pbio.cpp $(SRC_PATH)PolyBoolean/triacons.cpp \
$(SRC_PATH)PolyBoolean/pbpolys.cpp $(SRC_PATH)PolyBoolean/triamono.cpp \
$(SRC_PATH)PolyBoolean/pbsweep.cpp $(SRC_PATH)Network.cpp $(SRC_PATH)CPacket.cpp \
$(SRC_PATH)CTimedEvent.cpp $(SRC_PATH)CTimedEventScheduler.cpp
	$(CC) $(CFLAGS) $(SRC_PATH)mmgr/mmgr.cpp $(SRC_PATH)col_hand.cpp $(SRC_PATH)game_data.cpp \
$(SRC_PATH)input.cpp $(SRC_PATH)main.cpp \
$(SRC_PATH)math.cpp $(SRC_PATH)ogl_utils.cpp $(SRC_PATH)particle.cpp $(SRC_PATH)player.cpp $(SRC_PATH)render.cpp $(SRC_PATH)weapon.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp $(SRC_PATH)gpc/gpc.o $(SRC_PATH)OGLTextureManager/TextureManager.cpp \
$(SRC_PATH)PolyBoolean/pbgeom.cpp $(SRC_PATH)PolyBoolean/polybool.cpp \
$(SRC_PATH)PolyBoolean/pbio.cpp $(SRC_PATH)PolyBoolean/triacons.cpp \
$(SRC_PATH)PolyBoolean/pbpolys.cpp $(SRC_PATH)PolyBoolean/triamono.cpp \
$(SRC_PATH)PolyBoolean/pbsweep.cpp $(SRC_PATH)Network.cpp $(SRC_PATH)CPacket.cpp \
$(SRC_PATH)CTimedEvent.cpp $(SRC_PATH)CTimedEventScheduler.cpp \
$(LFLAGS) -o $@

$(SRC_PATH)gpc/gpc.o: $(SRC_PATH)gpc/gpc.c
	gcc -c $(CFLAGS) $(SRC_PATH)gpc/gpc.c -o $@
