#ifndef __NET__
#define __NET__

#include <cstring>
#include <enet/enet.h>

#define __PTYPE enet_uint8
enum PacketType {
  PTYPE_TIME_SYNC=0,        // C requests/S answers timesync
  PTYPE_COMPLETE,           // C requests/S answers complete game-state sync
  PTYPE_UPDATE,             // S sends partial game-state (changed data)
  PTYPE_PARTY_ASSIGN,       // S sends assigned party to client
  PTYPE_GAME_CONFIG,        // C requests/S answers game configuration (x ships, y shots, z planets, mapsize...)
  PTYPE_SHIPS_MOVE,         // C sends commanded ships
  PTYPE_PLANET_ACTION,      // C sends planet action (upgrade, shipqueue)
  PTYPE_TEXT,               // C/S sends text message
  PTYPE_READY,              // C signals ready true/false
  PTYPE_START,              // S starts game
  PTYPE_PAUSE,              // S pauses game
};

ENetPacket* enet_packet_create(	const void * 	data, size_t 	dataLength, enet_uint32 	flags, const PacketType packetType);
enet_uint8 * enet_packet_data(ENetPacket * packet);
PacketType enet_packet_type(ENetPacket * packet);
size_t enet_packet_size(ENetPacket * packet);


/// creates an enet packet with a packetType added in the beginning
ENetPacket* enet_packet_create(	const void * 	data, size_t 	dataLength, enet_uint32 	flags, PacketType packetType) {
  ENetPacket* packet = enet_packet_create(nullptr, sizeof(__PTYPE)+dataLength, flags);
  *(__PTYPE*)packet->data = (__PTYPE)packetType;
  memcpy(packet->data+sizeof(__PTYPE), data, dataLength);
  return packet;
}
/// gets the packettype from an enentpacket
PacketType enet_packet_type(ENetPacket * packet){
  return (PacketType)*(__PTYPE*)packet->data;
}
/// gets the data from an enetpacket (without the packetType, so its just +1)
enet_uint8 * enet_packet_data(ENetPacket * packet){
  return packet->data+sizeof(__PTYPE);
}
/// gets the size of the data, packettype-exclusive
size_t enet_packet_size(ENetPacket * packet){
  return packet->dataLength - sizeof(__PTYPE);
}

#endif // __NET__