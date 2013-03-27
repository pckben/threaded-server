#include "RemoteAudioSource.h"
#include "protocol.h"
#include <Socket.h>
#include <iostream>

using namespace pckben;
using namespace speechsvr;
using namespace kaldi;
using namespace std;

RemoteAudioSource::RemoteAudioSource(Socket *socket)
: socket_(socket) {}

int32
RemoteAudioSource::Read(VectorBase<BaseFloat> *data, uint32 *timeout) {
  int32 n_elem = data->Dim();

  //printf("Requesting %d samples", n_elem);
  // request the client to send n_elem samples
  DataWaveRequest data_wave_req = { n_elem, sizeof(BaseFloat) };
  SendPacket(socket_, DATA_WAVE_REQUEST, sizeof(DataWaveRequest),
             &data_wave_req);
  //printf("\n");

  // receive n_elem samples from the client
  PacketHeader header;
  socket_->Receive((char *)&header, sizeof(PacketHeader));
  //printf("Receiving %d samples", header.payload_length / sizeof(BaseFloat));

  if (header.type != DATA_WAVE) {
    cerr << "Error: wrong packet type. DATA_WAVE (" << DATA_WAVE
         << "). Receive = " << header.type << endl;
    exit(1);
  }
  socket_->Receive((char *)data->Data(), header.payload_length);
  //printf("\n");
  //printf("+");

  return header.payload_length / sizeof(BaseFloat);
}
