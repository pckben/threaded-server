#include <iostream>
#include <cstdlib>

#include "protocol.h"
#include <Socket.h>

#include <feat/wave-reader.h>
#include <util/kaldi-io.h>

using namespace pckben;
using namespace std;
using namespace kaldi;
using namespace speechsvr;

int main(int argc, char **argv) {
  if (argc < 4) {
    cout << "Syntax:\n\tspeechclient <server_address> <port> <path/to/wavefile>\n";
    exit(1);
  }
  char *addr = argv[1];
  int port = atoi(argv[2]);
  string wave_file = string(argv[3]);

  Input inp(wave_file);
  WaveHolder holder;
  holder.Read(inp.Stream());
  int length = holder.Value().Data().NumCols();
  const float *wave = holder.Value().Data().Row(0).Data();
  Socket client;
  client.Connect(addr, port);

  cout << "Connected to " << addr << ":" << port << endl;

  PacketHeader header;
  header.payload_length = length * sizeof(float);
  header.type = DATA_WAVE;

  cout << "Sending wave file '" << wave_file << "' ("
       << header.payload_length << " bytes)\n";
  client.Send((const char *)&header, sizeof(PacketHeader));
  client.Send((const char *)wave, header.payload_length);

  cout << "Waiting for server result...\n";

  client.Receive((char *)&header, sizeof(PacketHeader));
  char output[65536];
  client.Receive(output, header.payload_length);
  printf("%s\n", output);
  return 0;
}
