#include "inc.hpp"

float mouseRX=0, mouseRY=0; // real position
float mouseVX=0, mouseVY=0; // virtual position 
float viewX=0, viewY=0;
float money[2] = {0,0};
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
  mouseRX=(float)xpos;
  mouseRY=(float)ypos;
  mouseVX=mouseRX + viewX;
  mouseVY=mouseRY + viewY;
  
}


void initPlanets(saPlanet & planets, unsigned int size);
void initShips(saShip & ships, unsigned int size);
void initShots(saShot & shots, unsigned int size);

void processPlanets(saPlanet & sPlanets, saShip * sShips, double dt);
void processShips(saShip * sShips, double dt);
void processShots(saShot * sShots, saShip * sShips, saPlanet & sPlanets, double dt);

void shoot(saShip * sShips, saPlanet & sPlanets, double dt);

int main(int argc, char ** argv){  
  for(int i=0; i<100; i++)
    printf("%i\n",rand(-50, 50));
  for(int i=0; i<100; i++)
    printf("%.4f\n",randf());
  
  ////////////////
  // GAME VARS
  ////////////////
  saPlanet planets;
  saShot shots[2];
  saShip ships[2];
  initPlanets(planets, 4);
  initShots(shots[PA],  10000);
  initShots(shots[PB],  10000);
  initShips(ships[PA],  5000);
  initShips(ships[PB],  5000);
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
  enet_host_destroy(host);
  
  ///////////////////////////////
  // GLFW
  /////////////////////////////////s
  if(!glfwInit())
    exit("Failed at glfwInit()\n",EXIT_FAILURE);
  atexit(glfwTerminate);
  glfwSetErrorCallback(cb_error);
  struct sInfo & info = *getInfo();
  
  info.window = glfwCreateWindow(SCREENW,SCREENH,"PBENET",NULL,NULL);
  if(!info.window){
    exit("Failed at glfwCreateWindow()",EXIT_FAILURE);
  }
  // add input listeners
  glfwSetCursorPosCallback(info.window, cursor_pos_callback);
  
  glfwMakeContextCurrent(info.window);
  glViewport(0, 0, (GLsizei)SCREENW, (GLsizei)SCREENH);
  // change to projection matrix, reset the matrix and set upt orthogonal view
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, SCREENW, SCREENH, 0, 0, 1);// parameters: left, right, bottom, top, near, far

  // ---- OpenGL settings
  glfwSwapInterval(1); // lock to vertical sync of monitor (normally 60Hz/)
  glEnable(GL_SMOOTH); // enable (gouraud) shading
  glDisable(GL_DEPTH_TEST); // disable depth testing
  glEnable(GL_BLEND); // enable blending (used for alpha) and blending function to use
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
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
  while(!glfwWindowShouldClose(info.window)){ 
    // update timer
    dt = glfwGetTime() - time;
    time = glfwGetTime();
    fps = (fps*500 + 10/dt)/510;
    // move view
    if(mouseRX<40) viewX--;
    if(mouseRY<40) viewY--;
    if(mouseRX>SCREENW-40) viewX++;
    if(mouseRY>SCREENH-40) viewY++;
    if(viewX<0) viewX = 0;
    if(viewY<0) viewY = 0;    
    if(viewX>10000) viewX = 10000;
    if(viewY>10000) viewY = 10000;
    // process game content
    processPlanets(planets, ships, dt);
    processShips(ships, dt);
    // clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); // reset the matrix
    glLoadIdentity();
    // draw gamecontent
    drawPlanets(planets, -viewX, -viewY);
    drawShips(ships, -viewX, -viewY);
    drawShots(shots, -viewX, -viewY);

    
    char s[100];
    sprintf(s,"FPS=%4.0f Money A=%5i B=%5i",fps, (int)money[PA],(int)money[PB]);
    drawString(s,strlen(s),10,10);
    
    glfwSwapBuffers(info.window);
    glfwPollEvents();
  }
  
  glfwDestroyWindow(info.window);
  return 0;
}


/// set dx, dy relative to vector (xy)->(tx,ty)
inline void delta(const float x, const float y, const float tx, const float ty, float & dx, float & dy){
  dx = tx-x;
  dy = ty-y;
}
inline void normalize(float & x, float & y, const float LEN){
  float vecLen = sqrt(x*x + y*y);
  x = LEN*x/vecLen;
  y = LEN*y/vecLen;
}
void flyToTarget(sShot & shot, const float tx, const float ty){
  delta(shot.x, shot.y, tx, ty, shot.dx, shot.dy);
  normalize(shot.dx, shot.dy, SHOT_SPEED);
}
void flyToTarget(sShip & ship, const float tx, const float ty){
  ship.tx = tx;
  ship.ty = ty;
  delta(ship.x, ship.y, ship.tx, ship.ty, ship.dx, ship.dy);
  normalize(ship.dx, ship.dy, SHIP_SPEED);
}
void deleteShip(saShip & ships, const unsigned int id){
  ships.ships[id].health = 0;  // mark ship as dead 
  ships.free[ships.freePush] = id; // insert index to freeIndices list
  ships.freePush = (ships.freePush+1)%ships.size;           // increment insert pointer
}

bool addShot(saShot & shots, const float x, const float y, const float ty, const float tx) {
  sShot & shot = shots.shots[shots.insertPos];
  shot.timeToLive = SHOT_LIFETIME;
  shot.x = x;
  shot.y = y;
  
  flyToTarget(shot, tx, ty);
  shots.insertPos = (shots.insertPos+1)%shots.size; // increase insertpointer
  return true;
}

bool addShip(saShip & ships, const float x, const float y, const float tx, const float  ty){
  if(ships.freePop==ships.freePush){ // ships may be either empty or full
    if(ships.ships[ships.free[ships.freePop]].health){ // shiplimit reached
      return false;
    }
  }
  sShip & ship = ships.ships[ships.free[ships.freePop]];
  ship.health = SHIP_HEALTH_MAX;
  ship.timeToShoot = 0;
  ship.x = x;
  ship.y = y;
  flyToTarget(ship,tx,ty);
  
  //printf("ship created xy(%i,%i) txy(%i,%i) dxy(%.2f,%.2f) H%i\n", (int)ship.x, (int)ship.y, (int)ship.tx, (int)ship.ty, ship.dx, ship.dy, (int)ship.health);
  
  ships.freePop = (ships.freePop+1)%ships.size;// increment free index counter
  return true;
}
void processPlanets(saPlanet & sPlanets, saShip * sShips, double dt){
  sPlanet * planets = (sPlanet*)(const char*)sPlanets.planets;
  for(unsigned int i=0; i<sPlanets.size; i++){
    planets[i].tx = mouseVX; // todo remove
    planets[i].ty = mouseVY;
    
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
            printf("ship insert failed\n"); // todo think of sth better than restoring money
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
      // shoot if applicable todo
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
    planets[i].health += HEALTH_REGEN * dt;
    if(planets[i].health > HEALTH_MAX)
      planets[i].health = HEALTH_MAX;
  }
}

void processShots(saShot * sShots, saShip * sShips, saPlanet & sPlanets, double dt){
  for(unsigned int party=0; party<PN; party++) {
    sShot * shots = sShots[party].shots;
    for(unsigned int i=0; i<sShots[party].size; i++){
      if(shots[i].timeToLive>0){ // shot exists
        // move shot
        shots[i].x += shots[i].dx * SHOT_SPEED;
        shots[i].y += shots[i].dy * SHOT_SPEED;
        // check for collision todo
        
      }
    }
  }
}

void processShips(saShip * sShips, double dt){
    for(unsigned int party=0; party<PN; party++) {
      sShip * ships = sShips[party].ships;
      for(unsigned int i=0; i<sShips[party].size; i++){
        if(ships[i].health){ // ship alive, handle it
          
          /*float dx = rand(-3000,3000), dy = rand(-3000,3000);
          float len = sqrt(dx*dx + dy*dy);
          dx = dx*SEND_SHIP_RAND_RADIUS/len;
          dy = dy*SEND_SHIP_RAND_RADIUS/len;
          flyToTarget(ships[i], mouseVX + dx, mouseVY + dy);
          /**/
        
          // if moving, move :)
          if(ships[i].dx || ships[i].dy) {
            // move
            ships[i].x += ships[i].dx *dt;
            ships[i].y += ships[i].dy *dt;
            // if near target, stop move-motion and teleport to target
            float dx = ships[i].x - ships[i].tx;
            if(dx > -SHIP_TELEPORT_DIST && dx < SHIP_TELEPORT_DIST) {
              ships[i].dx = 0;
              ships[i].x = ships[i].tx;
            }
            float dy = ships[i].y - ships[i].ty;
            if(dy > -SHIP_TELEPORT_DIST && dy < SHIP_TELEPORT_DIST) {
              ships[i].dy = 0;
              ships[i].y = ships[i].ty;
            }
          }
          
          // shoot todo, better separate to use with planets
          
        }
        //printf("%i:%i health %i \n",party, i,ships[i].health);
      }
    }
}

void initPlanets(saPlanet & planets, unsigned int size){
  planets.size = size;
  planets.planets = new sPlanet[planets.size];
  memset(planets.planets, 0, sizeof(sPlanet)*size); // clear
  
  planets.planets[0] = sPlanet{0,0,100,100,700,600,0,0,0,PA,5000,100,50,true};
  planets.planets[1] = sPlanet{0,0,270,170,700,600,3,5,3,PB,5000,20,50,true};
  planets.planets[2] = sPlanet{0,0,140,280,700,600,0,0,0,PB,5000,33,50,false};
  planets.planets[3] = sPlanet{0,0,250,300,700,600,0,9,0,PA,5000,150,50,true};
}
void initShots(saShot & shots, unsigned int size){
  shots.size = size;
  shots.insertPos = 0;
  shots.shots = new sShot[shots.size];
  memset(shots.shots, 0, sizeof(sShot)*size); // clear
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

