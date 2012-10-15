#include "ThreadedServer.h"
#include "TaskDispatcher.h"
#include "OnlineDecodeTask.h"

using namespace pckben;

ThreadedServer::ThreadedServer(int nThreads) {
	dispatcher_ = new TaskDispatcher(nThreads);
}

ThreadedServer::~ThreadedServer() {
	delete dispatcher_;
}

void ThreadedServer::HandleClient(int sock) {
	Task* task = CreateTask(sock);
	dispatcher_->Dispatch(task);	// block if all workers are busy
}
