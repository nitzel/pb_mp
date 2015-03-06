#include "game.hpp"
#include "draw.hpp"
#include "net.hpp"

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);


#define TIME_TO_SYNC_TIME 0.5f
#define GAMESTATE_OLD 0.5f // gamestates older than this will be discarded
int main(int argc, char ** argv){
  printf("Client\n");
  if (enet_initialize () != 0)
  {
      fprintf (stderr, "An error occurred while initializing ENet.\n");
      return EXIT_FAILURE;
  }
  atexit (enet_deinitialize);
  
  ENetHost * host;
  ENetPeer * peer;
  ENetAddress address;
  ENetEvent event;
  
  // client
  host = enet_host_create(NULL,1,2,0,0);
  if(host == NULL) {
    printf("an error occured while trying to create an enet client.\n");
    exit(EXIT_FAILURE);
  }
  enet_host_bandwidth_throttle(host);
  enet_address_set_host(&address, "localhost");
  address.port = 12345;
  
  peer = enet_host_connect(host, &address, 2, 0);
  
  if(peer == nullptr){
    printf("no available peers for initiating an enet connection\n");
    exit(EXIT_FAILURE);
  }
  
  if(enet_host_service(host, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
    printf("Connection to host succeeded\n");
  else {
    enet_peer_reset(peer);
    printf("connection to host failed\n");
  }
  enet_host_flush (host);
  
  mouseR = {0,0};
  mouseV = {0,0};
  screen = {800,600}; // {640, 480};//
  view   = {0,0};
  map = {2000, 2000};
  Game game(1000, 6);
  ///////////////////////////////
  // init GLFW
  /////////////////////////////////s
  initGlfw("PB-MP-Client", screen.x, screen.y);
  initGfx();
  // add input listeners
  glfwSetCursorPosCallback(info.window, cursor_pos_callback);
  ////////////////
  // VARS
  ////////////////
  double time = glfwGetTime();
  double dt = 0, vdt = 0; // virtuel dt, added to dt on data-arrival
  double fps = 0;
  bool paused = false;
  double timeToSyncT = 0;
  
  while(!glfwWindowShouldClose(info.window)){
    // update timer
    dt = glfwGetTime() - time;
    time = glfwGetTime();
    fps = (fps*500 + 10/dt)/510;
    
    
    // sync time
    timeToSyncT -= dt;
    if(timeToSyncT<0)
    {
      timeToSyncT = TIME_TO_SYNC_TIME; // new timer for sync
      double pTime[2] = {glfwGetTime(),0};
      ENetPacket * packet = enet_packet_create((void*)pTime, 2*sizeof(double), 0, PTYPE_TIME_SYNC); // ENET_PACKET_FLAG_RELIABLE
      enet_peer_send(peer, 0, packet);
      enet_host_flush(host);
    }
    
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
      game.clearChanged();
      game.update(dt+vdt, false); // dont update planets as client
      game.generateTree();
      //game.shootAndCollide();
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
    
    while (enet_host_service (host, & event, 0) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_RECEIVE:
          //printf("packet received type=%d\n",enet_packet_type(event.packet));
          switch(enet_packet_type(event.packet)){
            case PTYPE_TIME_SYNC:
            {
              double * t = (double*)enet_packet_data(event.packet); // time
              //printf("timepacket received %.1f %.1f \n",t[0],t[1]);
              glfwSetTime(t[1]+(glfwGetTime()-t[0])/2);
            } break;
            case PTYPE_COMPLETE: // complete gamestate
            {
              const double t = *(double*)enet_packet_data(event.packet);
              const double pDt = glfwGetTime()-t; // packet delta time (packet age)
              //printf("bc size=%d servertime=%.2f dt=%.2f\n", enet_packet_size(event.packet), t, pDt);
              if(pDt<0) { // packet from "future"
                fprintf(stderr, "future packet received from t=%.2f at %.2f\n",t,glfwGetTime());
              } else if(pDt < GAMESTATE_OLD) {  // todo better algorithm than just age! we discard too old packets
                vdt = game.unpackData(enet_packet_data(event.packet), enet_packet_size(event.packet), glfwGetTime());
              }
            } break;
            case PTYPE_UPDATE:
            {
              const double t = *(double*)enet_packet_data(event.packet);
              const double pDt = glfwGetTime()-t; // packet delta time (packet age)
              //printf("bc size=%d servertime=%.2f dt=%.2f\n", enet_packet_size(event.packet), t, pDt);
              if(pDt<0) { // packet from "future"
                fprintf(stderr, "future updatepacket received from t=%.2f at %.2f\n",t,glfwGetTime());
              } 
              // todo: discard old updates or not? they are still important...
              game.unpackUpdateData(enet_packet_data(event.packet), enet_packet_size(event.packet), glfwGetTime());
              
            } break;
            case PTYPE_TEXT:
            {
              
            } break;
            default: ;
          }
          // Clean up the packet now that we're done using it.
          enet_packet_destroy (event.packet);
          
          break;
      case ENET_EVENT_TYPE_DISCONNECT:
          printf ("%s disconnected.\n", (char*)event.peer -> data);
          // Reset the peer's client information. 
          event.peer -> data = NULL;
          paused = true; // todo remove
          break;
      case ENET_EVENT_TYPE_NONE:
          break;
      default:;
      }
    }  
  }
  
  
  
  
  
  
  enet_host_flush(host);
  enet_peer_disconnect(peer, 0);
  double disconnected = false;
  while (enet_host_service (host, & event, 1000) > 0 && !disconnected)  {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_RECEIVE:
          enet_packet_destroy (event.packet);
          break;
      case ENET_EVENT_TYPE_DISCONNECT:
          puts ("Disconnection succeeded.\n");
          disconnected = true;
      default:
        break;
      }
  }
  if(!disconnected){
    puts ("Disconnection failed.\n");
    enet_peer_reset (peer);
  }  
  
  glfwDestroyWindow(info.window);
  
  glfwTerminate();

  enet_deinitialize();
  
  return 0;
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)  {
  mouseR.x=(float)xpos;
  mouseR.y=(float)ypos;
  mouseV.x=mouseR.x + view.x;
  mouseV.y=mouseR.y + view.y;
}