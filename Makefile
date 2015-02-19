EXECLIENT=game
EXESERVER=server
CFLAGS=-c -Wall -std=c++11 
#for optimization add -D__NO_INLINE_HYPOTF__
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
$(EXECLIENT): client.o inc.o stb_image.o Makefile
	$(CC) client.o inc.o stb_image.o -o $(EXECLIENT) $(LDFLAGS)
#server
$(EXESERVER): server.o inc.o stb_image.o Makefile
	$(CC) server.o inc.o stb_image.o -o $(EXESERVER) $(LDFLAGS)
  
server.o: server.cpp inc.hpp
	$(CC) $(CFLAGS) $(LDFLAGS) server.cpp

client.o: client.cpp inc.hpp
	$(CC) $(CFLAGS) $(LDFLAGS) client.cpp
  
inc.o: inc.cpp inc.hpp
	$(CC) $(CFLAGS) $(LDFLAGS) inc.cpp
  
stb_image.o: include/stb_image.h include/stb_image.c
	$(CC) $(CFLAGS) $(LDFLAGS) include/stb_image.c

#lodepng.o: lodepng.cpp lodepng.h
#	$(CC) $(CFLAGS) $(LDFLAGS) lodepng.cpp

clean:
	$(RM) *.o $(EXECLIENT) $(EXESERVER)