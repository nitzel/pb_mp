# Planet Battle Multiplayer
[![Join the chat at https://gitter.im/dmalatesta/spaceterm](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/nitzel/nitzel?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

A [Planet Battle 3](http://forums.purebasic.com/german/viewtopic.php?f=12&t=18101&sid=6ac3cca19644aa677b6a1aae8b797853 "Planet Battle 3 - by kswb73") clone to implement multiplayer in this fun game!


# Compiling / Running

Requires: [glfw3](http://www.glfw.org/download.html "glfw"), [ENet](http://enet.bespin.org/ "ENet")
> Note: I tweaked `math.h` to only not only `hypotf` inlined when using `-D__NO_INLINE_HYPOTF__`. If you get compilerproblems regarding `hypotf`, try `-D__NO_INLINE__` instead.

Run `make` to compile and run `./game` to play!
It is important that the game will not start until there are two clients connected to the server, so make sure you start the game as client twice or have a friend that connects, too.

To start the game as a client or as a sever you can use the console paramters `-c [<targethost>]` and `-s [<maxshipsperplayer>]` respectively or just run it up and enter the configuration in the console.
The settings.ini file is not yet functional (see issue #15).

# ToDo
  - [ ] [another todolist](https://github.com/nitzel/pb_mp/blob/master/notes/todo%2Bideas.md "more extensive todo list")
  - [ ] All :)
  - [x] Check if it compiles on
    - [ ] linux
    - [x] mac
    - [x] windows
  - [ ] Finish the server
  - [ ] Finish the client
  - [ ] Attempt to make the code look clean
    - [x] die trying :broken_heart:
    
# Credits

  - Inspiration: [Planet Battle 3](http://forums.purebasic.com/german/viewtopic.php?f=12&t=18101&sid=6ac3cca19644aa677b6a1aae8b797853 "Planet Battle 3 - by kswb73")

  - [stb_image](https://github.com/nothings/stb "stb_image lib by Sean Barrett") library by Sean Barret
  
  - [glfw3](http://www.glfw.org/download.html "glfw") awesome cross os lib for GL setup and window creation
  
  - [ENet](http://enet.bespin.org/ "ENet") networking library. (un-)reliable packets via UDP.

  - [inih](https://github.com/benhoyt/inih "INI Not Invented Here by Ben Hoyt") by Ben Hoyt to read ini files.
  