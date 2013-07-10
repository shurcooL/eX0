SRC_PATH :=

CC     = g++
CFLAGS = -I../glfw/include -I../../glfw/include -I/usr/X11R6/include \
	-O3 -ffast-math
# -w -Wall
LFLAGS = -L../glfw/lib/x11 -L../../glfw/lib/x11 -L/usr/X11R6/lib -lglfw -lGL
# -lGLU -lX11 -lXxf86vm -lXext -lpthread -lm

# Rule for eX0ds
eX0ds: $(SRC_PATH)Main.cpp $(SRC_PATH)player.cpp $(SRC_PATH)Network.cpp $(SRC_PATH)mmgr/mmgr.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp $(SRC_PATH)Client.cpp $(SRC_PATH)Packet.cpp $(SRC_PATH)col_hand.cpp \
$(SRC_PATH)game_data.cpp $(SRC_PATH)math.cpp $(SRC_PATH)particle.cpp $(SRC_PATH)weapon.cpp \
$(SRC_PATH)gpc/gpc.o $(SRC_PATH)Server.cpp
	$(CC) $(CFLAGS) $(SRC_PATH)Main.cpp $(SRC_PATH)player.cpp $(SRC_PATH)Network.cpp $(SRC_PATH)mmgr/mmgr.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp $(SRC_PATH)Client.cpp $(SRC_PATH)Packet.cpp $(SRC_PATH)col_hand.cpp \
$(SRC_PATH)game_data.cpp $(SRC_PATH)math.cpp $(SRC_PATH)particle.cpp $(SRC_PATH)weapon.cpp \
$(SRC_PATH)gpc/gpc.o $(SRC_PATH)Server.cpp $(LFLAGS) -o $@

$(SRC_PATH)gpc/gpc.o: $(SRC_PATH)gpc/gpc.c
	gcc -c $(CFLAGS) $(SRC_PATH)gpc/gpc.c -o $@
