EXECLIENT=game
EXESERVER=server
# -Wno-comment // or /* within a comment
CFLAGS=-c -Wall -std=c++11 -Wno-comment -g -Os -D__NO_INLINE_HYPOTF__
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
else 
#RM=del -f
EXECLIENT = client.exe
EXESERVER = server.exe
LDFLAGS += -lopengl32 -lgdi32 -lws2_32 -lwinmm
endif

all: $(EXECLIENT) $(EXESERVER)
client: $(EXECLIENT)
server: $(EXESERVER)

#client
$(EXECLIENT): client.o inc.o draw.o game.o stb_image.o makefile
	$(CC) client.o inc.o draw.o game.o stb_image.o -o $(EXECLIENT) $(LDFLAGS)
#server
$(EXESERVER): server.o inc.o draw.o game.o stb_image.o makefile
	$(CC) server.o inc.o draw.o game.o stb_image.o -o $(EXESERVER) $(LDFLAGS)
  
server.o: server.cpp draw.hpp game.hpp net.hpp makefile
	$(CC) $(CFLAGS) $(LDFLAGS) server.cpp

client.o: client.cpp draw.hpp game.hpp net.hpp makefile
	$(CC) $(CFLAGS) $(LDFLAGS) client.cpp
  
draw.o: draw.cpp draw.hpp makefile
	$(CC) $(CFLAGS) $(LDFLAGS) draw.cpp
  
game.o: game.cpp game.hpp makefile
	$(CC) $(CFLAGS) $(LDFLAGS) game.cpp
  
inc.o: inc.cpp inc.hpp makefile
	$(CC) $(CFLAGS) $(LDFLAGS) inc.cpp
  
stb_image.o: include/stb_image.h include/stb_image.c makefile
	$(CC) $(CFLAGS) $(LDFLAGS) include/stb_image.c

#lodepng.o: lodepng.cpp lodepng.h
#	$(CC) $(CFLAGS) $(LDFLAGS) lodepng.cpp

clean:
	$(RM) *.o $(EXECLIENT) $(EXESERVER)
