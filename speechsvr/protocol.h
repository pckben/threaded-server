#ifndef SPEECHSVR_PROTOCOL_H
#define SPEECHSVR_PROTOCOL_H

namespace pckben {
  class Socket;
}

/**
 * TODO: backward-compatible support for future updated protocols.
 *
 * File-based decode:
 *     client                               server
 *     DECODE_REQUEST(false)
 *                                          DECODE_ACCEPT
 *     DATA_WAVE(data)
 *                                          DECODE_OUTPUT
 *
 * Online decoding:
 *      DECODE_REQUEST(true)
 *                                          DECODE_ACCEPT
 *                                          DATA_WAVE_REQUEST(length)
 *      DATA_WAVE(data)
 *                                          DECODE_OUTPUT
 *                                          DATA_WAVE_REQUEST(length)
 *      DATA_WAVE(data)
 *                                          DECODE_OUTPUT
 *                                          DATA_WAVE_REQUEST(length)
 *      DATA_WAVE(data)
 *                                          DECODE_OUTPUT
 */
namespace speechsvr {
  enum PacketType {
    DECODE_REQUEST = 0,
    DECODE_ACCEPT,
    DATA_WAVE,
    DATA_WAVE_REQUEST,
    DATA_FEATURE,
    DECODE_OUTPUT,
  };

  struct PacketHeader {
    int payload_length;
    PacketType type;
  };

  struct DecodeRequest {
    bool online_mode;
  };

  struct DecodeResponse {
    bool accepted;
    // void *extra_info;
  };

  struct DataWaveRequest {
    int num_samples;
    int sample_size;
  };

  struct DataWave {
    void *samples;
  };

  struct DecodeOutput {
    bool end_utterance;
    const char *output;
  };

  void SendPacket(pckben::Socket *socket, PacketType type, int payload_length,
                  const void *payload);
}
#endif  // SPEECHSVR_PROTOCOL_H
