#include "ThreadedServer.h"
#include "TaskDispatcher.h"

using namespace pckben;

ThreadedServer::ThreadedServer(int num_threads)
: num_threads_(num_threads) {
  dispatcher_ = new TaskDispatcher();
}

ThreadedServer::~ThreadedServer() {
  delete dispatcher_;
}

void ThreadedServer::Start(int port) {
  for (int i = 0; i < num_threads_; i++) {
    dispatcher_->AddWorker(CreateWorker());
  }
  Server::Start(port);
}

void ThreadedServer::HandleClient(int sock) {
  Task* task = CreateTask(sock);
  dispatcher_->Dispatch(task);  // block if all workers are busy
}
