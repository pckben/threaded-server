#include "Worker.h"
#include "Task.h"
#include "stddef.h"
#include "assert.h"
#include "stdio.h"

using namespace pckben;

// worker thread's main function
// this is a friend method of BackgroundWorker class
void* pckben::worker_func(void* args) {
  BackgroundWorker* worker = reinterpret_cast<BackgroundWorker*>(args);

  // required for conditional variable wait
  pthread_mutex_lock(&worker->task_assigned_mutex_);

  while (worker->enabled_) {
    // wait for work condition signal,
    // unblock mutex at the same time
    if (!worker->task_) {
      // prevent the case when task_assigned was signaled before the worker has
      // started.
      pthread_cond_wait(&worker->task_assigned_cond_,
                        &worker->task_assigned_mutex_);
    }
    // when work condition is signaled,
    // mutex is locked again for this worker!

    // Now, check if the worker was signaled
    // with a new task
    if (worker->task_) {
      worker->Work();
      //worker->task_->Execute();
      Task* task = worker->task_;
      worker->task_ = NULL;
      task->Done();
      delete task;
    }
  }
  // release the mutex locked by work conditional variable
  pthread_mutex_unlock(&worker->task_assigned_mutex_);
  return NULL;
}

BackgroundWorker::BackgroundWorker() : enabled_(true), task_(NULL) {
  // initialize task_assigned conditional
  // variable and its mutex object
  pthread_mutex_init(&task_assigned_mutex_, NULL);
  pthread_cond_init(&task_assigned_cond_, NULL);

  // creates the thread in a joinable state
  pthread_attr_init(&thread_attr_);
  pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);

  pthread_create(&thread_, &thread_attr_, &worker_func, this);
}

BackgroundWorker::~BackgroundWorker() {
  enabled_ = false;

  // signal the thread to stop
  // but first must wait for it to finish
  // its current task first.
  pthread_mutex_lock(&task_assigned_mutex_);
  // if the worker was assigned a Task, but
  // not yet started doing it, we just delete
  // the Task and set it to NULL to prevent
  // the BackgroundWorker from doing it. Otherwise,
  // he will do the Task and then wait for
  // a new task forever.
  if (task_) {
    delete task_;
    task_ = NULL;
  }
  pthread_cond_signal(&task_assigned_cond_);
  pthread_mutex_unlock(&task_assigned_mutex_);
  // wait for it to stop
  pthread_join(thread_, NULL);

  // destroy everything
  pthread_cond_destroy(&task_assigned_cond_);
  pthread_mutex_destroy(&task_assigned_mutex_);
  pthread_attr_destroy(&thread_attr_);
}

bool BackgroundWorker::TryAssign(Task* task) {
  // try locking the work condition, fails if
  // the worker is busy.
  if (pthread_mutex_trylock(&task_assigned_mutex_) == 0) {
    // if the previously assigned task wasn't attempted
    // yet by the worker, return false and let him
    // attempt the task.
    if (task_) {
      pthread_mutex_unlock(&task_assigned_mutex_);
      return false;
    }
    // assign the task
    task_ = task;
    // signal the thread
    pthread_cond_signal(&task_assigned_cond_);
    pthread_mutex_unlock(&task_assigned_mutex_);

    return true;
  }
  return false;
}
