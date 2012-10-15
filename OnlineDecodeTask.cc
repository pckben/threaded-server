#include "OnlineDecodeTask.h"
#include "Socket.h"

using namespace pckben;

OnlineDecodeTask::OnlineDecodeTask(int sock) {
	socket_ = new Socket(sock);
}

OnlineDecodeTask::~OnlineDecodeTask() {
	delete socket_;
}

void OnlineDecodeTask::Execute() {
	// TODO
}
