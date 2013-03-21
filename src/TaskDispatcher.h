#ifndef TASKDISPATCHER_H
#define TASKDISPATCHER_H

#include <vector>
#include "Task.h"

namespace pckben {

  class Worker;
  class Semaphore;

  // A TaskDispatcher dispatch tasks to a pool of Workers.
  class TaskDispatcher : public TaskObserver {
   public:
    // Creates a dispatcher with the given number of available workers.
    TaskDispatcher();
    // Releases the dispatcher and all its workers.
    ~TaskDispatcher();
    // Add a Worker to the worker pool.
    void AddWorker(Worker* worker);
    // Dispatches the task to an available Worker. This will block if no worker
    // is available at the moment.
    void Dispatch(Task* task);

    void OnTaskDone(Task* task);

   private:
    Semaphore* semaphore_;
    std::vector<Worker*> workers_;
  };
}
#endif  // TASKDISPATCHER_H
