#include "TaskDispatcher.h"
#include "Semaphore.h"
#include "Worker.h"
#include "assert.h"
#include "stdio.h"
#include "errno.h"

using namespace pckben;

TaskDispatcher::TaskDispatcher() {
  semaphore_ = new Semaphore(0);
}

TaskDispatcher::~TaskDispatcher() {
  while (!workers_.empty()) {
    delete workers_.back();
    workers_.pop_back();
  }
  delete semaphore_;
}

void TaskDispatcher::AddWorker(Worker* worker) {
  workers_.push_back(worker);
  semaphore_->Signal();
}

void TaskDispatcher::Dispatch(Task* task) {
  // wait until at least a thread is free
  semaphore_->Wait();
  //printf("Dispatching, semaphore value = %d\n", semaphore_->GetValue());

  // loop through the workers and try assigning the task, there must be at least
  // 1 free worker at this point (as indicated by the semaphore, so Assign(task)
  // must be successful for at least 1 worker.
  bool taskAssigned = false;
  task->SetObserver(this);
  for (std::vector<Worker*>::iterator it = workers_.begin(); it != workers_.end(); ++it) {
    if ((*it)->TryAssign(task)) {
      taskAssigned = true;
      break;
    }
  }
  assert(taskAssigned && "The new task could not be assigned to any worker.");
}

void TaskDispatcher::OnTaskDone(Task* task) {
  //printf("Releasing semaphore...\n");
  semaphore_->Signal();
  //printf("Released, semaphore value = %d\n", semaphore_->GetValue());
}
