#include "Thread.h"
#include "ThreadPool.h"
#include "Runner.h"
#include "Server.h"
#include "Socket.h"
#include <iostream>
#include <cstdlib>

#define N_THREADS 5

using namespace std;
using namespace ntu;

class TestThread;
class TestRunner;
class TestServer;

class TestThread : public Thread {
		public:
			TestThread(TestRunner* runner) {
				runner_ = runner;
			}

			TestRunner* GetRunner() { return runner_; }

		private:
			TestRunner* runner_;

};


class TestRunner : public Runner {
	public:
		TestRunner() : socket_(0) { }
		~TestRunner() {
			if (socket_) 
				delete socket_;
		}

		void SetSocket(int sock) {
			if (socket_)
				delete socket_;

			socket_ = new Socket(sock);
		}


	protected:
		void DoRun() {
			cout << "Runner started" << endl;
			char msg[100];
			socket_->Receive(msg, 10);
			cout << "Received: " << string(msg) << endl;
			socket_->Send(msg, 10);
			cout << "Sent: " << string(msg) << endl;
		}

	private:
		Socket* socket_;
};


class TestServer : public Server {
	public:
		TestServer(int nThreads) {
			nThreads_ = nThreads;
			pool_ = new ThreadPool(nThreads);

			for (int i=0; i<nThreads; i++) {
				runner_[i] = new TestRunner();
				thread_[i] = new TestThread(runner_[i]);
				pool_->AddThread(thread_[i]);
			}
		}

		~TestServer() {
			for (int i=0; i<nThreads_; i++) {
				delete runner_[i];
				delete thread_[i];
			}
		}

	protected:
		void HandleClient(int sock) {
			// get available thread, blocked if no thread available
			Thread* thread = pool_->GetThread();
			// Get the corresponding runner
			TestRunner* runner = ((TestThread*)thread)->GetRunner();
			// pass sock to decoder
			runner->SetSocket(sock);
			// run on a the given thread
			runner->Run(thread);
		}

	private:
		int nThreads_;
		ThreadPool* pool_;
		Thread* thread_[128];
		TestRunner* runner_[128];
};


int main(int argc, char* argv[]) {
	if (argc < 2) {
		cout << "Syntax: test_decode_server <port>" << endl;
		return 1;
	}

	int port = atoi(argv[1]);

	TestServer server(N_THREADS);
	server.Start(port);

	return 0;
}
