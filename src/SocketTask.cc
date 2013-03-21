#include "SocketTask.h"
#include "Socket.h"

using namespace pckben;

SocketTask::SocketTask(int sock) {
  socket_ = new Socket(sock);
}

SocketTask::~SocketTask() {
  delete socket_;
}
