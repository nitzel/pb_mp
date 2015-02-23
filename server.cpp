#include "inc.hpp"

//todo next 
//  colorize planets,             DONE / was already ...
//  let planets shoot             DONE
//  target planets for shooting   DONE
//  planets loose levels on death DONE
//  and then multiplayer :)      

//  check for game over
//  make shields usable!   

float money[2] = {0,0};
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
  mouseR.x=(float)xpos;
  mouseR.y=(float)ypos;
  mouseV.x=mouseR.x + view.x;
  mouseV.y=mouseR.y + view.y;
  
}


void initPlanets(saPlanet & planets, unsigned int size);
void initShips(saShip & ships, unsigned int size);
void initShots(saShot & shots, unsigned int size);

void processPlanets(saPlanet & sPlanets, saShip * sShips, double dt);
void processShips(saShip * sShips, double dt);
void processShots(saShot * sShots, double dt);

void shoot(saShip * sShips, saPlanet & sPlanets, saShot * sShots,double dt);
int main(int argc, char ** argv){  
  freopen("stderr.txt","w",stderr);
  fprintf(stderr, "\nSTART\n");
  ////////////////
  // VARS
  ////////////////
  screen = {800,600}; // {640, 480};//
  view = {0,0};
  map = {2000,2000};
  mouseR = {0,0};
  mouseV = {0,0};
  
  ////////////////
  // GAME VARS
  ////////////////
  saPlanet planets;
  saShot shots[2];
  saShip ships[2];
  initPlanets(planets, 6);
  initShots(shots[PA],  10000);
  initShots(shots[PB],  10000);
  initShips(ships[PA],  10000);
  initShips(ships[PB],  10000);
  map.w = 2000;
  map.h = 2000;
  
  //////////
  // ENET
  /////////
  if (enet_initialize () != 0)
  {
      fprintf (stderr, "An error occurred while initializing ENet.\n");
      return EXIT_FAILURE;
  }
  atexit (enet_deinitialize);
  
  ENetHost * host;
  //ENetPeer * peer;
  ENetAddress address;
  ENetEvent event;
  
  // server
  printf("Server\n");
  address.host = ENET_HOST_ANY;
  address.port = 12345;
  host = enet_host_create(&address, 32, 2, 0, 0);
  
  if(host == NULL) {
    printf("An error occured while trying to create an ENet server host.\n");
    exit(EXIT_FAILURE);
  }
  
  
  /* Wait up to 1000 milliseconds for an event. */
  while(0){
    while (enet_host_service (host, & event, 1000) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            printf ("A new client connected from %x:%u.\n", 
                    event.peer -> address.host,
                    event.peer -> address.port);
            /* Store any relevant client information here. */
            event.peer -> data = (void*)"Client information";
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            printf ("A packet of length %u containing %s was received from %s on channel %u.\n",
                    event.packet -> dataLength,
                    event.packet -> data,
                    (char*)event.peer -> data,
                    event.channelID);
            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy (event.packet);
            
            break;
           
        case ENET_EVENT_TYPE_DISCONNECT:
            printf ("%s disconnected.\n", (char*)event.peer -> data);
            /* Reset the peer's client information. */
            event.peer -> data = NULL;
        case ENET_EVENT_TYPE_NONE:
        default:;
        }
    }
  }
  
  ///////////////////////////////
  // GLFW
  /////////////////////////////////s
  if(!glfwInit())
    exit("Failed at glfwInit()\n",EXIT_FAILURE);
  atexit(glfwTerminate);
  glfwSetErrorCallback(cb_error);
  //struct sInfo & info = *getInfo(); // todo decide if necessary
  
  info.window = glfwCreateWindow(screen.w,screen.h,"PBENET",NULL,NULL);
  if(!info.window){
    exit("Failed at glfwCreateWindow()",EXIT_FAILURE);
  }
  // add input listeners
  glfwSetCursorPosCallback(info.window, cursor_pos_callback);
  
  glfwMakeContextCurrent(info.window);
  glViewport(0, 0, (GLsizei)screen.w, (GLsizei)screen.h);
  // change to projection matrix, reset the matrix and set upt orthogonal view
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, screen.w, screen.h, 0, 0, 1);// parameters: left, right, bottom, top, near, far

  // ---- OpenGL settings
  glfwSwapInterval(1); // lock to vertical sync of monitor (normally 60Hz/)
  glEnable(GL_SMOOTH); // enable (gouraud) shading
  glDisable(GL_DEPTH_TEST); // disable depth testing
  glEnable(GL_BLEND); // enable blending (used for alpha)
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // set blending mode
  
  // -------- load images to info
  loadTextures();
  
  // make a cursor
  unsigned int pixels[15][15];
  memset(pixels, 0x00, sizeof(pixels));
  pixels[0][0] = 0xffffffff;
  pixels[14][0] = 0xffffffff;
  pixels[14][14] = 0xffffffff;
  pixels[0][14] = 0xffffffff;
  pixels[7][7] = 0xffffffff;
  GLFWimage image;
  image.width = 15;
  image.height = 15;
  image.pixels = (unsigned char*)pixels;
  GLFWcursor* cursor = glfwCreateCursor(&image, 8, 8);
  glfwSetCursor(info.window, cursor);
  // cursor done
  
  // GAME LOOP
  double time = glfwGetTime();
  double dt = 0;
  double fps = 0;
  bool paused = false;
  while(!glfwWindowShouldClose(info.window)){ 
    // update timer
    dt = glfwGetTime() - time;
    time = glfwGetTime();
    fps = (fps*500 + 10/dt)/510;
    // move view
    if(mouseR.x<40) view.x--;
    if(mouseR.y<40) view.y--;
    if(mouseR.x>screen.w-40) view.x++;
    if(mouseR.y>screen.h-40) view.y++;
    if(view.x<0) view.x = 0;
    if(view.y<0) view.y = 0;    
    if(view.x>map.w-screen.w) view.x = map.w-screen.w;
    if(view.y>map.h-screen.h) view.y = map.h-screen.h;
    
    // clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); // reset the matrix
    glLoadIdentity();
    
    // process game content
    if(!paused) {
      processPlanets(planets, ships, dt);
      processShips(ships, dt);
      processShots(shots, dt);
      shoot(ships, planets, shots, dt);
    }
    
    // draw gamecontent
    drawPlanets(planets, -view.x, -view.y);
    drawShips(ships, -view.x, -view.y);
    drawShots(shots, -view.x, -view.y);
    
    char s[100];
    sprintf(s,"FPS=%4.0f t=%.1fs Money A=%5i B=%5i MR%i/%i MV%i/%i View%i/%i",fps, time, (int)money[PA],(int)money[PB], (int)mouseR.x, (int)mouseR.y, (int)mouseV.x, (int)mouseV.y, (int)view.x, (int)view.y);
    glColor3ub(255,255,255);
    drawString(s,strlen(s),10,10);
    
    glfwSwapBuffers(info.window);
    glfwPollEvents();
  }
  
  glfwDestroyWindow(info.window);
  
  
  enet_host_destroy(host);
  return 0;
}


/// set dx, dy relative to vector (xy)->(tx,ty)
inline void delta(const float x, const float y, const float tx, const float ty, float & dx, float & dy){
  dx = tx-x;
  dy = ty-y;
}
inline void normalize(float & x, float & y, const float LEN){
  const float normFac = LEN/sqrt(x*x + y*y);
  if(std::isnan(normFac)) {
    x=0; 
    y=0; 
  } else {
    x = normFac*x;
    y = normFac*y;
  }
}
inline unsigned int distanceSQ(const float x, const float y, const float x2, const float y2) {
  return (x-x2)*(x-x2)+(y-y2)*(y-y2);
}
void flyToTarget(sShot & shot, const float tx, const float ty){
  delta(shot.x, shot.y, tx, ty, shot.dx, shot.dy);
  normalize(shot.dx, shot.dy, SHOT_SPEED);
}
void flyToTarget(sShip & ship, const float tx, const float ty){
  // dont let them go out map
  ship.tx = (tx<0)?0:(tx>=map.w?map.w-5:tx); // todo define macro or so to make nicer ...
  ship.ty = (ty<0)?0:(ty>=map.h?map.h-5:ty); // todo why -5 ?? 

  delta(ship.x, ship.y, ship.tx, ship.ty, ship.dx, ship.dy);
  normalize(ship.dx, ship.dy, SHIP_SPEED);
}
void deleteShip(saShip & ships, const unsigned int id){
  ships.ships[id].health = 0;  // mark ship as dead 
  ships.free[ships.freePush] = id; // insert index to freeIndices list
  ships.freePush = (ships.freePush+1)%ships.size;           // increment insert pointer
}

bool addShot(saShot & shots, const float x, const float y, const float tx, const float ty) {
  sShot & shot = shots.shots[shots.insertPos];
  shot.timeToLive = SHOT_LIFETIME;
  shot.x = x;
  shot.y = y;
  
  flyToTarget(shot, tx, ty);
  shots.insertPos = (shots.insertPos+1)%shots.size; // increase insertpointer
  return true;
}

bool addShip(saShip & ships, const float x, const float y, const float tx, const float  ty){
  // Check if the shiplist is full (no elements between freepop and freepush and the ship they are pointing to is alive)
  if(ships.freePop==ships.freePush && ships.ships[ships.free[ships.freePop]].health){
      return false;
  } else {  
    sShip & ship = ships.ships[ships.free[ships.freePop]];
    ship.health = SHIP_HEALTH_MAX;
    ship.timeToShoot = 0;
    ship.x = x;
    ship.y = y;
    flyToTarget(ship,tx,ty);
    
    ships.freePop = (ships.freePop+1)%ships.size;// increment free index counter
    return true;
  }
}

bool upgradePlanet(sPlanet & planet, Upgrade upgrade) {
  if(planet.level[upgrade] < UPGRADE_MAX_LVL){
    unsigned int costs = UPGRADE_COSTS; // const upgrade costs
    if(money[planet.party] >= costs) {
      money[planet.party] -= costs;
      planet.level[upgrade]++;
    }
  }
  return false;
}

void capturePlanet(sPlanet & planet, const unsigned int newParty){
  // When a planet gets neutral, it looses a lot of it's infrastructure
  // ECONOMY is a little bit affected
  planet.level[ECONOMY] = std::max(0, planet.level[ECONOMY]-1);
  // More than half of the DEFENSE is destroyed. 10 -> 5 -> 2 -> 1 -> 0
  planet.level[DEFENSE] = planet.level[DEFENSE]/2;
  // PRODUCTION is a little bit affected
  planet.level[PRODUCTION] = std::max(0, planet.level[ECONOMY]-1);
  
  // SET new party and reset health
  planet.party = newParty;
  // set health to 100% (remember, neutral full health is 0 ... ;) confusing, huh? But this way we can store it in one var)
  planet.health = newParty==PN?0:HEALTH_MAX; 
}

void takeDamage(sShip & ship){
  ship.health--;
  if(ship.health<=0){ // ship dead
    ship.health = -1; // mark for deletion
  }
}
/**
planet - planet to take damage/ that was shot
party - party dealing the damage
*/
void takeDamage(sPlanet & planet, const unsigned int party){
  if(planet.party == PN){ // neutral planet
    planet.health -= party*2-1; // Add one for PA, take one for PB. Remember, neutral planets are full health at 0 and overtaken at +-100.
    if(planet.health <= -HEALTH_MAX || planet.health >= HEALTH_MAX){ // overtake
      capturePlanet(planet, party);
    }
  } else { // belongs to the enemy party! 
    if (!planet.shieldActive) {
      planet.health --; // take life away!
      if(planet.health <= 0) { // make neutral!
        capturePlanet(planet, PN);
      }
    }
  }
}
/**
ship/planet - ship/planet to check for closest enemy and shoot
sPlanets - list of planets
shots - list of shots from the same party as ships
rivalTree - spacepartioning tree with the rival-partys ships
W,H - Size of rivalTree
*/
bool shoot(sShip & ship, saPlanet & sPlanets, saShot & shots, sSquare * rivalTree, const unsigned int W, const unsigned int H, const unsigned int PARTY){
    const unsigned int MAX_GRIDS = (2*SHIP_AIM_RANGE/GRID_SIZE+1)*(2*SHIP_AIM_RANGE/GRID_SIZE+1); // at max we need to check this many grids
  
  int dir = 0; // direction we're inserting the next lines in the rectangle
  // location to insert ships (+1 because first dir is up(-1))
  int lx = (int)ship.x/GRID_SIZE;
  int ly = (int)ship.y/GRID_SIZE+1; 
  int stepsPerLevel = 1; // how many ships to draw per line (will be counted down)
  int level = 0; // how many ships per level
  int repeat = 1; // how many lines to draw before level++  (will be counted down)
  
  bool targetFound = false; // will be true when found ;)
  for(unsigned int j=0; j<MAX_GRIDS && !targetFound; j++) {
    switch(dir){ // get location for next insertion
    case 0:
            ly --;
            break; // up
    case 1:
            lx ++;
            break; // right
    case 2:
            ly ++;
            break; // down
    case 3:
            lx --;
            break; //left
    }
    if(ly >= 0 && (unsigned int)ly < H && lx >= 0 && (unsigned int)lx < W && rivalTree[lx*W+ly].size) {           // valid lxy and not empty 
      // todo check if square is in range
      // range checking in here :)
      for(sShip * target : rivalTree[lx*W+ly].shiplist){
        if(distanceSQ(ship.x, ship.y, target->x, target->y) < SHIP_AIM_RANGE_SQ) {
          // okey we found someone to shoot
          targetFound = true;
          addShot(shots, ship.x, ship.y, target->x, target->y);
          ship.timeToShoot += SHIP_SHOOT_DELAY;
          return true;
        }
      }
    }
    
    // further calculations for spiral stuff
    stepsPerLevel--;
    if(stepsPerLevel <= 0){
        dir = (dir + 1) % 4;
        repeat--;
        if(repeat <= 0){
            level++;
            repeat = 2;
        }
        stepsPerLevel = level;
    }
  }
  // aim for planets
  if(!targetFound) {
    for(unsigned int i=0; i<sPlanets.size; i++){
      sPlanet & target = sPlanets.planets[i];      
      if(target.party == PARTY) { // planet from same party
        continue;
      }
      if(distanceSQ(ship.x, ship.y, target.x, target.y) < SHIP_AIM_RANGE_SQ) {
        // okey we found someone to shoot
        targetFound = true;
        addShot(shots, ship.x, ship.y, target.x, target.y);
        ship.timeToShoot += SHIP_SHOOT_DELAY;
        return true;
      }
    }
  }
  
  
  return true; // todo
}
bool shoot(sPlanet & planet, saPlanet & sPlanets, saShot & shots, sSquare * rivalTree, const unsigned int W, const unsigned int H){
  return shoot(*(sShip*)(((double*)&planet)+1), sPlanets, shots, rivalTree, W, H, planet.party);
}

// todo the built structure here is very useful for collision detection!
void shoot(saShip * sShips, saPlanet & sPlanets, saShot * sShots,double dt){
  /// The idea is to split the playground into squares lying next to 
  /// each other, each the size of the aiming-range of the ships.
  /// By coordinates you can directly calculated in which square a ship is
  /// It then has only to check against the 9 squares around.
  ///
  /// If there are :alot: of ships in a square it is further divided 
  /// into smaller ones, until they contain a reasonable amount. 
  /// Only the ones on aim-range will be tested
  
  const unsigned int W = map.w/GRID_SIZE;
  const unsigned int H = map.h/GRID_SIZE;
  
  /////////////////
  // Set up Structure
  ////////////////
  // todo improve with several layers, experiment
  // 1st: SHIP_AIM_RANGE^2, containing the amount in the whole Rectangle AND a list of layer2 nodes that are not empty
  // 2nd: Smaller
  // we dont need THE nearest enemy, just a close one!
  sSquare tree[2][W][H];
  // init size of tree with zero
  for(unsigned int party=PA; party<PN; party++)
    for(unsigned int x=0; x<W; x++)
      for(unsigned int y=0; y<H; y++)
        tree[party][x][y].size = 0;
  
  /////////////////
  // Fill space partitioning structure with ships
  ////////////////
  // todo try to optimize, takes 100fps away
  for(unsigned int party=PA; party<PN; party++){
    sShip * ships = sShips[party].ships;
    for(unsigned int i=0; i<sShips[party].size; i++){
      if(ships[i].health){
        tree[party][(unsigned int)(ships[i].x)/GRID_SIZE][(unsigned int)(ships[i].y)/GRID_SIZE].size++;
        tree[party][(unsigned int)(ships[i].x)/GRID_SIZE][(unsigned int)(ships[i].y)/GRID_SIZE].shiplist.push_front(&ships[i]);
      }
    }
  }
  
  /////////////////
  // RangeTesting and shooting
  //////////////// 
  // hint: im using a cool algorithm i invented on my own xD
  // commenting might be a bit odd, since it was to let the ships
  // go in a squared without counting how many ships there are
  // therefore we start in the center of the square and going outwards
  // in a spiral way
  // Here it is used for going through the grid, using closer grid
  // parts first to find a good-enough shootable ship (optimal would take too much time, we take the first we can find in reach, which is roughly the closest. the really closest may be about sqrt(GRID_size) closer, which is acceptable)
  for(unsigned int party=PA; party<PN; party++){
    //sShip * lastTarget = nullptr;
    //float lastTargetDistanceSQ = 0;
    unsigned int rival = !party; // opponents party ID :)
    // let ships shoot
    for(unsigned int i=0; i<sShips[party].size; i++){
      sShip & ship = sShips[party].ships[i];
      if(!ship.health || ship.timeToShoot>0) {// dead or weapon not ready
        continue;
      }
      shoot(ship, sPlanets, sShots[party],(sSquare*)tree[rival],  W, H, party);
    }
    // let planets shoot
    for(unsigned int i=0; i<sPlanets.size; i++){
      sPlanet & planet = sPlanets.planets[i];      
      if(planet.party == PN || planet.timeToShoot>0) {// dead or weapon not ready
        continue;
      }
      shoot(planet, sPlanets, sShots[planet.party],(sSquare*)tree[!planet.party],  W, H);
    }
  }  
  
  /////////////////
  // CollisionTesting
  ////////////////
  for(unsigned int party=0; party<PN; party++) {
    unsigned int rival = !party; // opponents party ID :)
    sShot * shots = sShots[party].shots;
    for(unsigned int i=0; i<sShots[party].size; i++){
      if(shots[i].timeToLive>0){ // shot exists 
        /////////////////////
        // test against ships
        for(sShip * target : tree[rival][(int)shots[i].x/GRID_SIZE][(int)shots[i].y/GRID_SIZE].shiplist){
          if(target->health && distanceSQ(shots[i].x, shots[i].y, target->x, target->y) < SHIP_RADIUS*SHIP_RADIUS) {
            // collision!!!
            shots[i].timeToLive = -1;
            takeDamage(*target);
            break;
          }
        }
        ///////////////////////
        // test against planets
        if(shots[i].timeToLive>0) { // shot did not hit a ship, still alive
          sPlanet * planets = sPlanets.planets;
          for(unsigned int j=0; j<sPlanets.size; j++){ // take the FIRST planet you can find that is not in our party
            if(planets[j].party != party && distanceSQ(shots[i].x, shots[i].y, planets[j].x, planets[j].y) < PLANET_RADIUS*PLANET_RADIUS) {
              // collision!!!
              shots[i].timeToLive = -1;
              takeDamage(planets[j], party);
              break;
            }
          }
        }
      }
    }
  }
  
  
  // draw space partition tree todo remove or make key dependent. should not be called here...
  drawTree((sSquare*)tree, -view.x, -view.y);
}

void processPlanets(saPlanet & sPlanets, saShip * sShips, double dt){
  sPlanet * planets = (sPlanet*)(const char*)sPlanets.planets;
  for(unsigned int i=0; i<sPlanets.size; i++){
    //planets[i].tx = mouseV.x; // todo remove
    //planets[i].ty = mouseV.y;
    
    if(planets[i].party!=PN){ // not neutral
      // generate money for the player
      money[planets[i].party] += (planets[i].level[ECONOMY]+1) * MONEY_GEN * dt; 
      // build ships if queue is not empty and production time is over
      if(planets[i].shipQueue) { // ships in queue
        planets[i].timeToBuild -= dt;      // subtract time
        if(planets[i].timeToBuild < 0) { // send a new ship out
          // create new ship 
          // put ship in a random circle around tx/ty
          float dx = rand(-50,50), dy = rand(-50,50);
          float len = sqrt(dx*dx + dy*dy);
          dx = dx*SEND_SHIP_RAND_RADIUS/len;
          dy = dy*SEND_SHIP_RAND_RADIUS/len;
          // add it
          if(!addShip(sShips[planets[i].party], planets[i].x+dt,planets[i].y,planets[i].tx+dx,planets[i].ty+dy)) {
            fprintf(stderr, "ship insert failed\n"); // todo think of sth better than restoring money
            // restore money to player
            money[planets[i].party] += SHIP_COSTS * planets[i].shipQueue;
            planets[i].shipQueue = 0; 
          } else {
            planets[i].shipQueue --; // decrement build list
          }
          
          // still building - reset time
          if(planets[i].shipQueue)  
            planets[i].timeToBuild += SHIP_PROD_TIME*(1-(planets[i].level[PRODUCTION]+1)/UPGRADE_MAX_LVL);
        }
      }
      // use power if shield if active
      if(planets[i].shieldActive) {
        planets[i].power -= (double)POWER_DRAIN/(planets[i].level[DEFENSE]+1)*dt;
        if(planets[i].power<0) //  no more power -> turn shield off
          planets[i].shieldActive = false;
      } else { // restore power, shield is off
        planets[i].power += POWER_REGEN*(planets[i].level[DEFENSE]+1)*dt;
        if(planets[i].power > POWER_MAX) // limit to maximum powercapacity
          planets[i].power = POWER_MAX;
      }
    }
    // restore health
    if(planets[i].party == PN) {                // neutral planets are special, 0=full health. The sign tells us who is trying to take it over
      if(planets[i].health < 0) {               // PA is trying to take it
        planets[i].health += HEALTH_REGEN * dt; // regenerate
        if(planets[i].health > 0) {
          planets[i].health = 0;                // set to full health
        }     
      } else if (planets[i].health > 0) {       // PB is trying to take it
        planets[i].health -= HEALTH_REGEN * dt; // regenerate
        if(planets[i].health < 0) {
          planets[i].health = 0;                // set to full health
        }
      }
    } else  {                                    // Planet owned by a party, they act normal xD
      planets[i].health += HEALTH_REGEN * dt;   // regenerate
      if(planets[i].health > HEALTH_MAX)
        planets[i].health = HEALTH_MAX;         // set to full health
    } /**/
    // reload weapons
    planets[i].timeToShoot -= dt * (1+(float)planets[i].level[DEFENSE]/4);
  }
}
// todo maybe check for ships and shots leaving the map area.
// maybe not necessary, since shots will die either way and ships can only be sent
// to valid positions ... so it would computationtime not to test it
void processShots(saShot * sShots, double dt){
  for(unsigned int party=0; party<PN; party++) {
    sShot * shots = sShots[party].shots;
    for(unsigned int i=0; i<sShots[party].size; i++){
      if(shots[i].timeToLive>0){ // shot exists
        // decrease lifetime
        shots[i].timeToLive -= dt;
        // move shot
        shots[i].x += shots[i].dx * dt;
        shots[i].y += shots[i].dy * dt;
        // shots outside screen will be deleted
        if(shots[i].x<0 || shots[i].x>map.w || shots[i].y<0 || shots[i].y>map.h) {
          shots[i].timeToLive = -1;
        }
      }
    }
  }
}

void processShips(saShip * sShips, double dt){
  for(unsigned int party=0; party<PN; party++) {
    sShip * ships = sShips[party].ships;
    for(unsigned int i=0; i<sShips[party].size; i++){
      if(ships[i].health){ // ship alive, handle it
        if(ships[i].health < 0){
          deleteShip(sShips[party], i);
          continue;
        }
        if(party==PA){
          //float dx = rand(-3000,3000), dy = rand(-3000,3000);
          //normalize(dx,dy,SEND_SHIP_RAND_RADIUS);
          //flyToTarget(ships[i], mouseV.x + dx, mouseV.y + dy);
        }
        /**/
      
        // if moving, move :)
        if(ships[i].dx || ships[i].dy) {
          // move
          ships[i].x += ships[i].dx *dt;
          ships[i].y += ships[i].dy *dt;
          // if overshooting target, teleport to target
          float dx = ships[i].tx - ships[i].x;
          float dy = ships[i].ty - ships[i].y;
          if((dx>0 && ships[i].dx<0)||(dx<0 && ships[i].dx>0)) {ships[i].x=ships[i].tx; ships[i].dx=0;}
          if((dy>0 && ships[i].dy<0)||(dy<0 && ships[i].dy>0)) {ships[i].y=ships[i].ty; ships[i].dy=0;}
          
        }
        // reduce shooting time
        ships[i].timeToShoot -= dt;         
      }
    }
  }
}

void initPlanets(saPlanet & planets, unsigned int size){
  planets.size = size;
  planets.planets = new sPlanet[planets.size];
  memset(planets.planets, 0, sizeof(sPlanet)*size); // clear
  
  planets.planets[0] = sPlanet{0,0,180,100,180,100,0,0,0, PA,3000,80,50,false};
  planets.planets[1] = sPlanet{0,0,120,230,120,230,0,0,0, PA,3000,80,50,false};
  planets.planets[2] = sPlanet{0,0,240,420,240,420,0,0,0,PA,3000,80,50,false};
  planets.planets[3] = sPlanet{0,0,800,110,500,110,0,0,0, PB,3000,80,50,false};
  planets.planets[4] = sPlanet{0,0,920,280,420,280,0,0,0, PB,3000,80,50,false};
  planets.planets[5] = sPlanet{0,0,730,380,630,380,0,0,0, PB,3000,80,50,false};
}
void initShots(saShot & shots, unsigned int size){
  shots.size = size;
  shots.insertPos = 0;
  shots.shots = new sShot[shots.size];
  memset(shots.shots, 0, sizeof(sShot)*size); // clear data
}
void initShips(saShip & ships, unsigned int size){
  ships.size = size;
  ships.ships = new sShip[ships.size];
  
  memset(ships.ships, 0, sizeof(sShip)*size); // clear
  ships.freePush = 0;
  ships.freePop  = 0;
  ships.free = new int[ships.size];
  for(unsigned int i=0; i<ships.size; i++)
    ships.free[i]=i;
  
}

