#ifndef WORKER_H
#define WORKER_H

#include <pthread.h>

namespace pckben {
  class Task;

  // A Worker can be assigned a Task to work with.
  class Worker {
   public:
    // Try assigning the Task to the Worker.
    // @return true if successfull.
    virtual bool TryAssign(Task* task) = 0;
    // Start working on the assigned Task.
    virtual void Work() = 0;
    // Get the assigned Task.
    virtual Task* GetTask() = 0;
    virtual ~Worker() {}
  };

  // Abstract background worker class that can be assigned tasks to. Once a task
  // is assigned to a worker, the work is done in the worker's own thread.
  // Implementation of this class must define the actual task handling.
  class BackgroundWorker : public Worker {
   public:
    // Creates the worker and initialize its worker thread, which will
    // immediately be put into waiting state to wait for a new Task to be
    // assigned.
    BackgroundWorker();
    // The destructor will wait for the worker thread to complete before
    // releasing its resources.
    //
    // NOTE: If the worker is being destroyed before a task assigned to him
    // being attempted, the task will be discarded and destroyed.
    virtual ~BackgroundWorker(); 
    // Try assigning the Task to this Worker.
    //
    // @return true if successful, in which case the worker starts working on
    // the task immediately. If the worker is busy at the moment, this will
    // return false.
    //
    // Once finished, the Worker will destroy the Task himself.
    bool TryAssign(Task* task);
    Task* GetTask() { return task_; }

   private:
    // making the thread running function friend of this class so it can access
    // its private members
    friend void* worker_func(void* args);
    pthread_t thread_;
    pthread_attr_t thread_attr_;

    // conditional variable to wait for task_assigned event.
    pthread_cond_t task_assigned_cond_;
    pthread_mutex_t task_assigned_mutex_;
    bool enabled_;  //< set to false and signal the task_assigned event to stop
                    //< the thread.

    Task* task_;
  };

  void* worker_func(void* args);
}
#endif  // WORKER_H
