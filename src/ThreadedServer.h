#ifndef THREADEDSERVER_H
#define THREADEDSERVER_H

#include "Server.h"

namespace pckben {
  class Task;
  class Worker;
  class TaskDispatcher;

  // An abstract multi-threadded server, wraps the basic Server with a
  // TaskDispatcher to handle multiple clients simultaneously on separate worker
  // threads.
  class ThreadedServer: public Server {
   public:
    ThreadedServer(int nThreads);
    virtual ~ThreadedServer();

    virtual void Start(int port);

   protected: 
    // Handles the connected socket by building
    // a new Task and dispatching it to an available
    // Worker.
    void HandleClient(int sock);
    // Task factory method
    virtual Task* CreateTask(int sock) = 0;
    // Worker factory method
    virtual Worker* CreateWorker() = 0;

   private:
    TaskDispatcher* dispatcher_;
    int num_threads_;
  };
}

#endif  // THREADEDSERVER_H
