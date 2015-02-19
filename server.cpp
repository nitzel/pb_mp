#include "inc.hpp"

float mouseX=0, mouseY=0;
float viewX=0, viewY=0;
float money[2] = {0,0};
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
  mouseX=(float)xpos;
  mouseY=(float)ypos;
}


void initPlanets(saPlanet & planets, unsigned int size);
void initShots(saShot & shots, unsigned int size);
void initShips(saShip & ships, unsigned int size);

void processPlanets(saPlanet & sPlanets, saShip * sShips, double dt);
void processShips(saShip * sShips, double dt);

int main(int argc, char ** argv){  
  ////////////////
  // GAME VARS
  ////////////////
  saPlanet planets;
  saShot shots[2];
  saShip ships[2];
  initPlanets(planets, 4);
  initShots(shots[PA], 1000);
  initShots(shots[PB], 1000);
  initShips(ships[PA],  5);
  initShips(ships[PB],  5);
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
  
  info.window = glfwCreateWindow(640,480,"PBENET",NULL,NULL);
  if(!info.window){
    exit("Failed at glfwCreateWindow()",EXIT_FAILURE);
  }
  // add input listeners
  glfwSetCursorPosCallback(info.window, cursor_pos_callback);
  
  glfwMakeContextCurrent(info.window);
  glViewport(0, 0, (GLsizei)640, (GLsizei)480);
  // change to projection matrix, reset the matrix and set upt orthogonal view
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 640, 480, 0, 0, 1);// parameters: left, right, bottom, top, near, far

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
    fps = (fps*500 + 1/dt)/501;
    // move view
    if(mouseX<40) viewX--;
    if(mouseY<40) viewY--;
    if(mouseX>600) viewX++;
    if(mouseY>440) viewY++;
    if(viewX<0) viewX = 0;
    if(viewY<0) viewY = 0;    
    if(viewX>200) viewX = 200;
    if(viewY>200) viewY = 200;
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

void flyToTarget(sShip & ship, float tarX, float tarY){
  ship.tx = tarX;
  ship.ty = tarY;
  ship.dx = 0;
  ship.dy = 0;
}
void deleteShip(saShip & ships, unsigned int id){
  ships.ships[id].health = 0;  // mark ship as dead 
  ships.free[ships.freePush] = id; // insert index to freeIndices list
  ships.freePush = (ships.freePush+1)%ships.size;           // increment insert pointer
}
bool addShip(saShip & ships, float x, float y, float tarX, float  tarY){
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
  flyToTarget(ship,tarX,tarY);
  
  printf("ship created xy(%i,%i) txy(%i,%i) dxy(%.2f,%.2f) H%i\n", (int)ship.x, (int)ship.y, (int)ship.tx, (int)ship.ty, ship.dx, ship.dy, (int)ship.health);
  
  ships.freePop = (ships.freePop+1)%ships.size;// increment free index counter
  return true;
}
void processPlanets(saPlanet & sPlanets, saShip * sShips, double dt){
  sPlanet * planets = (sPlanet*)(const char*)sPlanets.planets;
  for(unsigned int i=0; i<sPlanets.size; i++){
    if(planets[i].party!=PN){ // not neutral
      // generate money for the player
      money[planets[i].party] += (planets[i].level[ECONOMY]+1) * MONEY_GEN * dt; 
      // build ships if queue is not empty and production time is over
      if(planets[i].shipQueue) { // ships in queue
        planets[i].timeToBuild -= dt;      // subtract time
        if(planets[i].timeToBuild < 0) { // send a new ship out
          // create new ship 
          planets[i].shipQueue --;
          if(!addShip(sShips[planets[i].party], planets[i].x+dt,planets[i].y,planets[i].tx,planets[i].ty)) {
            printf("ship insert failed\n"); // todo restore money if list full/fails
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
void processShips(saShip * sShips, double dt){
    for(unsigned int party=0; party<PN; party++) {
      sShip * ships = sShips[party].ships;
      for(unsigned int i=0; i<sShips[party].size; i++){
        if(ships[i].health){ // ship alive, handle it
          // move
          // if near target, stop move-motion
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
  
  planets.planets[0] = sPlanet{0,0,100,100,150,120,0,0,0,PA,2,100,50,true};
  planets.planets[1] = sPlanet{0,0,270,170,160,130,3,5,3,PB,3,20,50,true};
  planets.planets[2] = sPlanet{0,0,140,280,130,110,0,0,0,PN,1,33,50,false};
  planets.planets[3] = sPlanet{0,0,250,300,250,310,0,10,0,PA,9,150,50,true};
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

