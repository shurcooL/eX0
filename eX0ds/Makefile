SRC_PATH :=

CC     = g++
CFLAGS = -w -I../glfw/include -I../../glfw/include -I/usr/X11R6/include \
	 -O3 -ffast-math -Wall
LFLAGS = -L../glfw/lib/x11 -L../../glfw/lib/x11 -L/usr/X11R6/lib
# -lglfw -lGL -lGLU
# -lglfw -lGL -lGLU -lX11 -lXxf86vm -lXext -lpthread -lm

# Rule for eX0ds
eX0ds: $(SRC_PATH)Main.cpp $(SRC_PATH)player.cpp $(SRC_PATH)Network.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp
	$(CC) $(CFLAGS) $(SRC_PATH)Main.cpp $(SRC_PATH)player.cpp $(SRC_PATH)Network.cpp \
$(SRC_PATH)Mgc/MgcDist2DVecLin.cpp $(SRC_PATH)Mgc/MgcIntr2DLinLin.cpp $(SRC_PATH)Mgc/MgcMath.cpp \
$(SRC_PATH)Mgc/MgcVector2.cpp \
$(LFLAGS) -o $@
