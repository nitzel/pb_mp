#ifndef __NET__
#define __NET__

#include <cstring>
#include <enet/enet.h>

#define __PTYPE unsigned char
enum PacketType {PTYPE_TIME_SYNC=0, PTYPE_COMPLETE, PTYPE_UPDATE, PTYPE_TEXT};

ENetPacket* enet_packet_create(	const void * 	data, size_t 	dataLength, enet_uint32 	flags, const PacketType packetType);
void * enet_packet_data(ENetPacket * packet);
PacketType enet_packet_type(ENetPacket * packet);
size_t enet_packet_size(ENetPacket * packet);


/// creates an enet packet with a packetType added in the beginning
ENetPacket* enet_packet_create(	const void * 	data, size_t 	dataLength, enet_uint32 	flags, PacketType packetType) {
  ENetPacket* packet = enet_packet_create(nullptr, sizeof(__PTYPE)+dataLength, flags);
  *(unsigned char*)packet->data = (__PTYPE)packetType;
  memcpy(packet->data+sizeof(__PTYPE), data, dataLength);
  return packet;
}
/// gets the packettype from an enentpacket
PacketType enet_packet_type(ENetPacket * packet){
  return (PacketType)*(__PTYPE*)packet->data;
}
/// gets the data from an enetpacket (without the packetType, so its just +1)
void * enet_packet_data(ENetPacket * packet){
  return packet->data+sizeof(__PTYPE);
}
/// gets the size of the data, packettype-exclusive
size_t enet_packet_size(ENetPacket * packet){
  return packet->dataLength - sizeof(__PTYPE);
}

#endif // __NET__