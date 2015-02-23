#include "game.hpp"
#include "draw.hpp"

#include <enet/enet.h>

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
  
  enet_address_set_host(&address, "localhost");
  address.port = 12345;
  
  peer = enet_host_connect(host, &address, 2, 0);
  
  if(peer == nullptr){
    printf("no available peers for initiating an enet connection\n");
    exit(EXIT_FAILURE);
  }
  
  if(enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
    printf("Connection to host succeeded\n");
  else {
    enet_peer_reset(peer);
    printf("connection to host failed\n");
  }
  
  //ENetPacket * packet = enet_packet_create("packet",strlen("packet")+1, ENET_PACKET_FLAG_RELIABLE);
  //enet_packet_resize(packet, strlen("packetfoo")+1);
  //strcpy((char*)&packet->data[strlen("packet")], "foo");
  
  // send packet to peer over channel 0
  //enet_peer_send(peer, 0, packet);
  //enet_host_flush (host);
  enet_host_flush (host);
  enet_peer_disconnect(peer, 0);
  while (enet_host_service (host, & event, 3000) > 0)  {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_RECEIVE:
          enet_packet_destroy (event.packet);
          break;
      case ENET_EVENT_TYPE_DISCONNECT:
          puts ("Disconnection succeeded.\n");
          exit(EXIT_SUCCESS);
      default:
        break;
      }
  }
  puts ("Disconnection failed.\n");
  enet_peer_reset (peer);
  
  
  return 0;
}
