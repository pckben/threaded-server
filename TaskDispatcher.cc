#include "TaskDispatcher.h"
#include "Task.h"
#include "Worker.h"
#include "assert.h"

using namespace pckben;

TaskDispatcher::TaskDispatcher(int size) {
	size_ = size;
	sem_init(&semaphore_, 0, size);
	worker_  = new Worker*[size];
	for (int i=0; i<size; i++)
		worker_[i] = new Worker();
}

TaskDispatcher::~TaskDispatcher() {
	sem_destroy(&semaphore_);
	for (int i=0; i<size_; i++)
		delete worker_[i];
	delete [] worker_;
}

void TaskDispatcher::Dispatch(Task* task) {
	// wait until at least a thread is free
	sem_wait(&semaphore_);

	// loop through the workers and try assigning
	// the task, there must be at least 1 free
	// worker at this point (as indicated by the
	// semaphore, so Assign(task) must
	// be successful for at least 1 worker.
	bool taskAssigned = false;
	for (int i=0; i<size_; i++)
		if (worker_[i]->TryAssign(task)) {
			taskAssigned = true;
			break;
		}
	assert(taskAssigned);
}
