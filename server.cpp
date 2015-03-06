#include "game.hpp"
#include "draw.hpp"
#include "net.hpp"

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
  enet_host_bandwidth_throttle(host);
  
  ////////////////
  // VARS
  ////////////////
  double time = glfwGetTime();
  double dt = 0, vdt = 0; // virtuel dt, added to dt on data-arrival
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
  
  Game game(1000, 6);
  
  ///////////////////////////////
  // init GLFW
  /////////////////////////////////s
  initGlfw("PB-MP-SERVER", screen.x, screen.y);
  initGfx();
  // add input listeners
  glfwSetCursorPosCallback(info.window, cursor_pos_callback);
 
  
  double timeToBroadcast = 0;
  // GAME LOOP
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
      game.update(dt+vdt);
      game.shootAndCollide();
      vdt = 0;
    }
    
    // draw gamecontent
    drawPlanets(game.mPlanets,  -view.x, -view.y);
    drawShips  (game.mShips,    -view.x, -view.y);
    drawShots  (game.mShots,    -view.x, -view.y);
    drawTree   (game.mTree,     -view.x, -view.y);
    
    char s[100];
    sprintf(s,"FPS=%4.0f t=%.1fs Money A=%5i B=%5i MR%i/%i MV%i/%i View%i/%i",fps, time, (int)money[PA],(int)money[PB], (int)mouseR.x, (int)mouseR.y, (int)mouseV.x, (int)mouseV.y, (int)view.x, (int)view.y);
    glColor3ub(255,255,255);
    drawString(s,strlen(s),10,10);
    
    glfwSwapBuffers(info.window);
    glfwPollEvents();
    
    timeToBroadcast -= dt;
    if(timeToBroadcast<0)
    {
      timeToBroadcast = 0.1; /// 2x per sec
      size_t size;
      //void * d = game.packData(size, glfwGetTime());
      void * d = game.packUpdateData(size, glfwGetTime());
      //ENetPacket * packet = enet_packet_create(d,size,0,PTYPE_COMPLETE);// ENET_PACKET_FLAG_RELIABLE
      ENetPacket * packet = enet_packet_create(d,size,0,PTYPE_UPDATE);// ENET_PACKET_FLAG_RELIABLE
      //game.unpackUpdateData(enet_packet_data(packet), enet_packet_size(packet), glfwGetTime()+0.1f);
      //vdt = game.unpackData(enet_packet_data(packet), enet_packet_size(packet), glfwGetTime()+0.1f);
      // send packet to peer over channel 1
      enet_host_broadcast(host, 1, packet);
      enet_host_flush (host);
      free(d);
      game.clearChanged();
    }
    while (enet_host_service (host, & event, 0) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
          printf ("A new client connected from %x:%u.\n", 
                  event.peer -> address.host,
                  event.peer -> address.port);
          /* Store any relevant client information here. */
          event.peer -> data = (void*)"Client information";
          paused = false; // todo remove
          break;
      case ENET_EVENT_TYPE_RECEIVE:
          //printf ("A packet of length %u containing %f was received from %s on channel %u.\n",                  event.packet -> dataLength,                  *(double*)event.packet -> data,                  (char*)event.peer -> data,                  event.channelID);
          //printf("packet received type=%d\n",enet_packet_type(event.packet));
          switch(enet_packet_type(event.packet)){
            case PTYPE_TIME_SYNC:
            {
              double time[2] = {*(double*)enet_packet_data(event.packet), glfwGetTime()};
              printf("timepacket received %.1f %.1f \n",time[0],time[1]);
              ENetPacket * packet = enet_packet_create(&time,sizeof(double)*2, 0, PTYPE_TIME_SYNC); // ENET_PACKET_FLAG_RELIABLE
              // send packet to peer over channel 0
              enet_peer_send(event.peer, 0, packet);
              enet_host_flush (host);
            } break;
            case PTYPE_SHIPS_MOVE:
            {
              
            } break;
            case PTYPE_PLANET_ACTION:
            {
              
            } break;
            case PTYPE_TEXT:
            {
              
            } break;
            default: ;
          }
          
          /* Clean up the packet now that we're done using it. */
          enet_packet_destroy (event.packet);
          break;
      case ENET_EVENT_TYPE_DISCONNECT:
          printf ("%s disconnected.\n", (char*)event.peer -> data);
          /* Reset the peer's client information. */
          event.peer -> data = NULL;
          paused = true; // todo remove
          break;
      case ENET_EVENT_TYPE_NONE:
          break;
      default:;
      }
    }
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
