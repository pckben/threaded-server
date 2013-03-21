// wave input from client send to server, online vad in server multithread
#include <iostream>

#include "Config.h"
#include "SpeechServer.h"

using namespace std;
using namespace pckben;
using namespace speechsvr;

int main(int argc, char* argv[]) {
  string config_file = "server.conf";
  Config conf(config_file);
  int num_threads = conf.GetInt("num_threads");
  int port = conf.GetInt("port");
  cout << "Starting server with " << num_threads << " threads.\n";
  Server* server = new SpeechServer(num_threads, config_file);
  server->Start(port);
  delete server;

  return 0;
}
