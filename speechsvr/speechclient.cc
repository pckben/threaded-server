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

void CheckPacket(PacketType expected, PacketType actual) {
  if (expected != actual) {
    cerr << "Error: wrong packet type received. Expected: "
         << expected << ", received: " << actual << endl;
    exit(1);
  }
}

string trim1(string s) {
  // trim back
  size_t pos = s.find_last_not_of(" \t\r\n");
  if (pos != string::npos)
    s = s.substr(0, pos+1);
  else
    s = "";
  // trim front
  pos = s.find_first_not_of(" \t\r\n");
  if (pos != string::npos)
    s = s.substr(pos);
  return s;
}

int main(int argc, char **argv) {
  if (argc < 4) {
    cout << "Syntax:\n\tspeechclient <server_address> <port> <path/to/wavefile> [--online]\n";
    exit(1);
  }
  char *addr = argv[1];
  int port = atoi(argv[2]);
  string wave_file = string(argv[3]);
  bool online_mode = false;
  if (argc >= 5) {
    for (int i = 4; i < argc; i++)
      if (strcmp(argv[i], "--online") == 0)
        online_mode = true;
  }

  Input inp(wave_file);
  WaveHolder holder;
  holder.Read(inp.Stream());
  int length = holder.Value().Data().NumCols();
  const float *wave = holder.Value().Data().Row(0).Data();
  Socket client;
  client.Connect(addr, port);

  cout << "Connected to " << addr << ":" << port << endl;

  DecodeRequest req;
  req.online_mode = online_mode;
  SendPacket(&client, DECODE_REQUEST, sizeof(DecodeRequest), &req);

  PacketHeader header;
  client.Receive(&header, sizeof(PacketHeader));

  CheckPacket(DECODE_ACCEPT, header.type);

  DecodeResponse res;
  client.Receive(&res, sizeof(DecodeResponse));
  if (!res.accepted) {
    cerr << "Error: request rejected by server." << endl;
    exit(1);
  }

  char output[65536];
  bool end_utterance;

  if (!online_mode) {
    cout << "Sending wave file '" << wave_file << "' ("
      << length * sizeof(float) << " bytes)\n";
    SendPacket(&client, DATA_WAVE, length * sizeof(float), wave);

    cout << "Waiting for server result...\n";
    client.Receive(&header, sizeof(PacketHeader));

    if (header.type != DECODE_OUTPUT) {
      cerr << "Error: wrong packet type received. Expected: DECODE_OUTPUT ("
           << DECODE_OUTPUT << "), received: " << header.type << endl;
      exit(1);
    }

    client.Receive(&end_utterance, sizeof(bool));
    client.Receive(output, header.payload_length - sizeof(bool));
    printf("%s\n", output);
  }
  else {
    int pos = 0;
    while (pos < length) {
      client.Receive(&header, sizeof(PacketHeader));

      if (header.type == DATA_WAVE_REQUEST) {
        DataWaveRequest req;
        client.Receive(&req, sizeof(DataWaveRequest));
        //printf("Request received: %d samples\n", req.num_samples);

        const float *payload = wave + pos;
        SendPacket(&client, DATA_WAVE, min(length-pos, req.num_samples) * sizeof(float), (void *)payload);
        pos += req.num_samples;
        //printf("-");
      }
      else if (header.type == DECODE_OUTPUT) {
        client.Receive(&end_utterance, sizeof(bool));
        int output_length = header.payload_length - sizeof(bool);
        client.Receive(output, output_length);
        output[output_length] = 0;
        //printf("+");
        cerr << trim1(string(output)) << " ";
        if (end_utterance)
          cerr << endl << endl;
      }
    }
  }

  return 0;
}
