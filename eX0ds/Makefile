SRC_PATH := src/
SHARED_PATH := ../shared/

CC     = g++
CFLAGS = -I../../../Libraries/glfw/include -I/usr/X11R6/include \
	-O0 -g -ffast-math -pthread -Wall \
	-D_DEBUG
LFLAGS = ../../../Libraries/glfw/lib/x11/libglfw.a -L/usr/X11R6/lib -lGL \
	-lX11 -lXrandr -lpthread -lm
# -lXxf86vm -lXext

# Rule for eX0ds
eX0ds: $(SRC_PATH)Main.cpp $(SHARED_PATH)player.cpp $(SRC_PATH)Network.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp $(SHARED_PATH)NetworkConnection.cpp $(SRC_PATH)ClientConnection.cpp $(SHARED_PATH)CPacket.cpp $(SHARED_PATH)col_hand.cpp \
$(SRC_PATH)game_data.cpp $(SHARED_PATH)math.cpp $(SHARED_PATH)particle.cpp $(SHARED_PATH)weapon.cpp \
$(SRC_PATH)gpc/gpc.o $(SRC_PATH)Server.cpp $(SHARED_PATH)CTimedEvent.cpp $(SHARED_PATH)CTimedEventScheduler.cpp \
$(SHARED_PATH)GameLogicThread.cpp $(SHARED_PATH)PlayerStateAuther.cpp $(SHARED_PATH)LocalStateAuther.cpp \
$(SHARED_PATH)PlayerController.cpp $(SHARED_PATH)NetworkController.cpp
	$(CC) $(CFLAGS) $(SRC_PATH)Main.cpp $(SHARED_PATH)player.cpp $(SRC_PATH)Network.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp $(SHARED_PATH)NetworkConnection.cpp $(SRC_PATH)ClientConnection.cpp $(SHARED_PATH)CPacket.cpp $(SHARED_PATH)col_hand.cpp \
$(SRC_PATH)game_data.cpp $(SHARED_PATH)math.cpp $(SHARED_PATH)particle.cpp $(SHARED_PATH)weapon.cpp \
$(SRC_PATH)gpc/gpc.o $(SRC_PATH)Server.cpp $(SHARED_PATH)CTimedEvent.cpp $(SHARED_PATH)CTimedEventScheduler.cpp \
$(SHARED_PATH)GameLogicThread.cpp $(SHARED_PATH)PlayerStateAuther.cpp $(SHARED_PATH)LocalStateAuther.cpp \
$(SHARED_PATH)PlayerController.cpp $(SHARED_PATH)NetworkController.cpp \
$(LFLAGS) -o $@

$(SRC_PATH)gpc/gpc.o: $(SRC_PATH)gpc/gpc.c
	gcc -c $(CFLAGS) $(SRC_PATH)gpc/gpc.c -o $@
