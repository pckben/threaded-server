#include "Socket.h"
#include <iostream>
#include <cstdlib>

using namespace pckben;
using namespace std;

int main(int argc, char* argv[]) {
  if (argc < 4) {
    cout << "Syntax:\n\ttest_client <server_address> <port> <message>\n";
    exit(1);
  }

  char* addr = argv[1];
  int port = atoi(argv[2]);
  char* message = argv[3];

  if (strlen(message) != 10) {
    cout << "Testing with message length = 10 only.\n";
    exit(1);
  }

  Socket client;
  client.Connect(addr, port);
  
  client.Send(message, strlen(message));
  cout << "Sent: " << string(message) << endl;

  char received[100];
  client.Receive(received, 10);
  cout << "Received: " << string(received) << endl;

  return 0;
}
