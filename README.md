# Planet Battle Multiplayer
[![Join the chat at https://gitter.im/dmalatesta/spaceterm](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/nitzel/nitzel?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

A [Planet Battle 3](http://forums.purebasic.com/german/viewtopic.php?f=12&t=18101&sid=6ac3cca19644aa677b6a1aae8b797853 "Planet Battle 3 - by kswb73") clone to implement multiplayer in this fun game!


# Compiling 
Requires: [glfw3](http://www.glfw.org/download.html "glfw"), [ENet](http://enet.bespin.org/ "ENet")
#### Visual Studio
- Install [vcpkg](https://github.com/Microsoft/vcpkg "MS Visual C++ Package Manager") to manage the libraries on Windows
  - Run `vcpkg integrate install` so that Visual Studio will find the includes and libraries on its own
  - Or update the project's `Properties -> VC++ Directories -> Include/Library Directories` to your path of `vcpkg`
- Install the required libraries`vcpkg install glfw3:x64-windows enet:x64-windows`
#### make
- Install `enet` and `glfw` e.g. via `apt-get` or by compiling them yourself. Make sure the comiler knows where they are. (`-I`)
- Run `make`

# Running
- Built with Visual Studio: Run the executable via Visual Studio or from the `x64/Debug` or `x64/Release` folder with the working directory being above `src`
- Built with make: Run the executable.
The game will not begin until there are two clients connected to the server, so make sure you start the game as client twice or have a friend that connects, too.
To start the game as a client or as a sever you can use the console paramters `-c` and `-s` respectively or just run it up and enter the configuration in the console.
The `settings.ini` file must be used to configure the window size, network port and server IP or name.

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
  
