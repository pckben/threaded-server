#ifndef SPEECHSVR_PROTOCOL_H
#define SPEECHSVR_PROTOCOL_H
namespace speechsvr {
  enum PacketType {
    DATA_WAVE = 0x01,
    DATA_FEATURE = 0x02,
    DECODE_OUTPUT = 0x0F,
  };

  struct PacketHeader {
    int payload_length;
    PacketType type;
    bool online_mode;
  };

  struct Packet {
    PacketHeader header;
    void *payload;
  };
}
#endif  // SPEECHSVR_PROTOCOL_H
