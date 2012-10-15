#include "TaskDispatcher.h"
#include "Semaphore.h"
#include "Worker.h"
#include "assert.h"
#include "stdio.h"
#include "errno.h"

using namespace pckben;

TaskDispatcher::TaskDispatcher(int size) {
	size_ = size;
	semaphore_ = new Semaphore(size);
	printf("Initializing, semaphore value = %d\n", semaphore_->GetValue());

	worker_  = new Worker*[size];
	for (int i=0; i<size; i++)
		worker_[i] = new Worker();
}

TaskDispatcher::~TaskDispatcher() {
	for (int i=0; i<size_; i++)
		delete worker_[i];
	delete [] worker_;
	delete semaphore_;
}

void TaskDispatcher::Dispatch(Task* task) {
	// wait until at least a thread is free
	semaphore_->Wait();
	printf("Dispatching, semaphore value = %d\n", semaphore_->GetValue());

	// loop through the workers and try assigning
	// the task, there must be at least 1 free
	// worker at this point (as indicated by the
	// semaphore, so Assign(task) must
	// be successful for at least 1 worker.
	bool taskAssigned = false;
	task->SetObserver(this);
	for (int i=0; i<size_; i++)
		if (worker_[i]->TryAssign(task)) {
			taskAssigned = true;
			break;
		}
	assert(taskAssigned && "The new task could not be assigned to any worker.");
}

void TaskDispatcher::OnTaskDone(Task* task) {
	printf("Releasing semaphore...\n");
	semaphore_->Signal();
	printf("Released, semaphore value = %d\n", semaphore_->GetValue());
}
