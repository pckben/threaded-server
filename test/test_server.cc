#include "TaskDispatcher.h"
#include "Task.h"
#include "ThreadedServer.h"
#include "SocketTask.h"
#include "Socket.h"
#include "Worker.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace pckben;

class TestWorker : public BackgroundWorker {
 public:
  void Work() {
    cout << "working\n";
    char msg[100];
    Socket *sock = ((SocketTask *)GetTask())->GetSocket();
    sock->Receive(msg, 10);
    cout << "Received: " << string(msg) << endl;
    usleep((rand() % 10)*100000 + 1000000); // sleep 1-2s
    sock->Send(msg, 10);
    cout << "Sent: " << string(msg) << endl;
  }
};

class TestServer : public ThreadedServer {
 public:
  TestServer(int nThreads) : ThreadedServer(nThreads) { }

 protected:
  Task* CreateTask(int sock) {
    return new SocketTask(sock);
  }
  Worker* CreateWorker() {
    return new TestWorker();
  }
};


int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "Syntax: test_decode_server <n_threads> <port>" << endl;
    return 1;
  }

  int num_workers = atoi(argv[1]);
  int port = atoi(argv[2]);

  cout << "Starting a multi-threading server with " << num_workers << " workers.\n";

  Server* server = new TestServer(num_workers);
  server->Start(port);
  delete server;

  return 0;
}
