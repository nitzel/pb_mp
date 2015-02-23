#include "game.hpp"
#include "draw.hpp"

#include <enet/enet.h>

//todo next 
//  colorize planets,             DONE / was already ...
//  let planets shoot             DONE
//  target planets for shooting   DONE
//  planets loose levels on death DONE
//  and then multiplayer :)      

//  check for game over                 todo later
//  make shields usable!          DONE

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);

int main(int argc, char ** argv){  
  freopen("stderr.txt","w",stderr);
  fprintf(stderr, "Started\n");
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
  
  ////////////////
  // VARS
  ////////////////
  double time = glfwGetTime();
  double dt = 0;
  double fps = 0;
  bool paused = false;
  
  mouseR = {0,0};
  mouseV = {0,0};
  screen = {800,600}; // {640, 480};//
  view   = {0,0};
  ////////////////
  // GAME VARS
  ////////////////
  map = {2000,2000};
  
  saPlanet planets;
  saShot shots[2];
  saShip ships[2];
  
  ///////////////////////////////
  // init GLFW
  /////////////////////////////////s
  initGlfw("PB-MP", screen.x, screen.y);
  initGfx();
  // add input listeners
  glfwSetCursorPosCallback(info.window, cursor_pos_callback);
 
  
  
  // GAME LOOP
  initGame(planets, ships, shots, 10000);
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

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
  mouseR.x=(float)xpos;
  mouseR.y=(float)ypos;
  mouseV.x=mouseR.x + view.x;
  mouseV.y=mouseR.y + view.y;
}
