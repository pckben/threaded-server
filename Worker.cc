#include "Worker.h"
#include "Task.h"
#include "stddef.h"
#include "assert.h"
#include "stdio.h"

using namespace pckben;

// worker thread's main function
// this is a friend method of Worker class
void* pckben::worker_func(void* args) {
	Worker* thread = reinterpret_cast<Worker*>(args);

	// required for conditional variable wait
	pthread_mutex_lock(&thread->task_assigned_mutex_);

	while (thread->enabled_) {
		// wait for work condition signal,
		// unblock mutex at the same time
		if (!thread->task_)	// this line prevent the case when task_assigned was signaled before the thread has started.
			pthread_cond_wait(&thread->task_assigned_cond_, &thread->task_assigned_mutex_);
		// when work condition is signaled,
		// mutex is locked again for this thread!

		// Now, check if the thread was signaled
		// with a new task
		if (thread->task_) {
			thread->task_->Execute();
			Task* task = thread->task_;
			thread->task_ = NULL;
			task->Done();
			delete task;
		}
	}
	// release the mutex locked by work conditional variable
	pthread_mutex_unlock(&thread->task_assigned_mutex_);
	return NULL;
}

Worker::Worker() : task_(NULL), enabled_(true) {
	// initialize task_assigned conditional
	// variable and its mutex object
	pthread_mutex_init(&task_assigned_mutex_, NULL);
	pthread_cond_init(&task_assigned_cond_, NULL);

	// creates the thread in a joinable state
	pthread_attr_init(&thread_attr_);
	pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);

	pthread_create(&thread_, &thread_attr_, &worker_func, this);
}

Worker::~Worker() {
	enabled_ = false;

	// signal the thread to stop
	// but first must wait for it to finish
	// its current task first.
	pthread_mutex_lock(&task_assigned_mutex_);
	// if the worker was assigned a Task, but
	// not yet started doing it, we just delete
	// the Task and set it to NULL to prevent
	// the Worker from doing it. Otherwise,
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

bool Worker::TryAssign(Task* task) {
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
