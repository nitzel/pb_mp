# TODO next
- [x] Game::pack/unpackData !
- [x] Send packed data to client and display it. 
- [x] ENet with commands (net.hpp)
- [x] Send new shots only
- [x] Send new ships and changed ones
- [x] Game::select
- [ ] Commands from client to server
  - [x] Send ships to a target
  - [ ] Planet Upgrades (first with shortcuts)
- [ ] Client GUI
  - [ ] Minimap
  - [x] Select and order ships
  - [ ] Select and order Planets
  - [ ] Zoom ! 
- [x] assign party to clients
  - [x] so that they are either PA or PB
  - [x] They should get a notification of that
  - [x] adapt commands like ship-sending to this
  
# TODO 2
- [x] colorize planets,            
- [x] let planets shoot            
- [x] target planets for shooting  
- [x] planets loose levels on death
- [x] and then multiplayer (its not realy "playing" yet)      
- [x] make shields usable!
- [ ] check for game over                 todo later
  1. count planets per player
  2. if still has planets: count total amount of ships with spaceTree


#Client needs
- [ ] A list of his planets 
  - [ ] to build ships on all of them with a single click
  - [ ] to set ralley points
- [ ] status 
  - [ ] of planets: economy, buildqueue, ...
  - [ ] number of ships
- [ ] mark all ships 
- [ ] mark all ships in a "close range"

# Later
- [ ] let only a given number of ships shoot per frame, will keep FPS more constant or have max-time to be used in shoot that must not be exceeded while shifting the "start-ship" with each frame further, so that all the ships come to shoot
