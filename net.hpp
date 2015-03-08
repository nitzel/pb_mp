#ifndef __NET__
#define __NET__

#include <cstring>
#include <enet/enet.h>

#define __PTYPE enet_uint8
enum PacketType {PTYPE_TIME_SYNC=0, PTYPE_COMPLETE, PTYPE_UPDATE, PTYPE_GAME_CONFIG, PTYPE_SHIPS_MOVE, PTYPE_PLANET_ACTION, PTYPE_TEXT, PTYPE_START};

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