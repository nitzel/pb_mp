EXE=pb_gnu
# -Wno-comment // or /* within a comment
CFLAGS=-c -Wall -std=c++11 -Wno-comment -g
#for optimization add -Os -D__NO_INLINE_HYPOTF__
#optimization gave a speed bonus from 40 to 65fps at 2x20.000 ships
LDFLAGS=-lglfw3  -lenet
# -s for strip symbols from binaries -> smaller

# Which compiler to use?
CC=g++
RM=rm -rf

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
CC=clang++
LDFLAGS += -stdlib=libc++ -framework Cocoa -framework OpenGL -framework IOKit
else ifeq ($(UNAME), Linux)
LDFLAGS += -lGL
# Windows
else
RM=del
EXE = pb_gnu.exe
LDFLAGS += -lopengl32 -lgdi32 -lws2_32 -lwinmm -static-libgcc -static-libstdc++
endif

all: $(EXE)

# executable
$(EXE): main.o server.o client.o inc.o draw.o game.o net.o makefile
	$(CC) main.o server.o client.o inc.o draw.o game.o net.o -o $(EXE) $(LDFLAGS)

main.o: main.cpp src/server.hpp src/client.hpp makefile
	$(CC) $(CFLAGS) $(LDFLAGS) main.cpp

server.o: src/server.cpp src/server.hpp src/draw.hpp src/game.hpp src/net.hpp makefile
	$(CC) $(CFLAGS) $(LDFLAGS) src/server.cpp

client.o: src/client.cpp src/client.hpp src/draw.hpp src/game.hpp src/net.hpp makefile
	$(CC) $(CFLAGS) $(LDFLAGS) src/client.cpp
  
draw.o: src/draw.cpp src/draw.hpp src/include/stb_image.h makefile
	$(CC) $(CFLAGS) $(LDFLAGS) src/draw.cpp
  
game.o: src/game.cpp src/game.hpp makefile
	$(CC) $(CFLAGS) $(LDFLAGS) src/game.cpp
  
inc.o: src/inc.cpp src/inc.hpp makefile
	$(CC) $(CFLAGS) $(LDFLAGS) src/inc.cpp
  
net.o: src/net.cpp src/net.hpp makefile
	$(CC) $(CFLAGS) $(LDFLAGS) src/net.cpp

clean:
	$(RM) *.o
	$(RM) $(EXE)
