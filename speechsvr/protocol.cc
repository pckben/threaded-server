#include "protocol.h"
#include <Socket.h>

using namespace pckben;
using namespace speechsvr;

void speechsvr::SendPacket(Socket *socket, PacketType type, int length, const void *payload) {
  PacketHeader header = { length, type };
  socket->Send(&header, sizeof(PacketHeader));
  socket->Send(payload, header.payload_length);
}
