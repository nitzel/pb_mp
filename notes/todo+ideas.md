# TODO next
- [x] Game::pack/unpackData !
- [x] Send packed data to client and display it. 
- [x] ENet with commands (net.hpp)
- [x] Send new shots only
- [x] Send new ships and changed ones
- [x] Game::select
- [ x Commands from client to server
  - [x] Send ships to a target
  - [ ] Planet Upgrades (first with shortcuts)
- [ ] Client GUI
  - [ ] Minimap
  - [ ] Select and order ships
  - [ ] Select and order Planets
  - [ ] Zoom ! 

# TODO 2
- [x] colorize planets,             DONE / was already ...
- [x] let planets shoot             DONE
- [x] target planets for shooting   DONE
- [x] planets loose levels on death DONE
- [ ] and then multiplayer :)      
- [ ] check for game over                 todo later
  - count total amount of ships with spaceTree
  - count planets per player
- [x] make shields usable!          DONE


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