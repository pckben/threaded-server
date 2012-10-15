#include "Worker.h"
#include "TaskDispatcher.h"
#include "Task.h"
#include "ThreadedServer.h"
#include "Socket.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace pckben;

class TestTask;
class TestServer;

class TestTask : public Task {
	public:
		TestTask(int sock) {
			socket_ = new Socket(sock);
		}
		virtual ~TestTask() {
			delete socket_;
		}

		void Execute() {
			char msg[100];
			socket_->Receive(msg, 10);
			cout << "Received: " << string(msg) << endl;
			usleep((rand() % 20)*100000 + 1000000); // sleep 1-3s
			socket_->Send(msg, 10);
			cout << "Sent: " << string(msg) << endl;
		}

	private:
		Socket* socket_;
};


class TestServer : public ThreadedServer {
	public:
		TestServer(int nThreads) : ThreadedServer(nThreads) { }
		virtual ~TestServer() { }

	protected:
		Task* CreateTask(int sock) {
			return new TestTask(sock);
		}
};


int main(int argc, char* argv[]) {
	if (argc < 2) {
		cout << "Syntax: test_decode_server <n_threads> <port>" << endl;
		return 1;
	}

	int nWorkers = atoi(argv[1]);
	int port = atoi(argv[2]);

	cout << "Starting a multi-threading server with " << nWorkers << " workers.\n";

	Server* server = new TestServer(nWorkers);
	server->Start(port);
	delete server;

	return 0;
}
