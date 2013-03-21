#ifndef SOCKETTASK_H
#define SOCKETTASK_H
#include "Task.h"
namespace pckben {
  class Socket;
  // A Task requested by a network remote client.
  class SocketTask : public Task {
   public:
    SocketTask(int sock);
    virtual ~SocketTask();
    Socket* GetSocket() { return socket_; }
   private:
    Socket* socket_;
  };
}
#endif  // SOCKETTASK_H
