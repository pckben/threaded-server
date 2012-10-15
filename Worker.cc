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

	// thread is started, now signal the thread_started event:
	pthread_mutex_lock(&thread->thread_started_mutex_);
	pthread_cond_signal(&thread->thread_started_cond_);
	pthread_mutex_unlock(&thread->thread_started_mutex_);

	while (true) {
		// wait for work condition signal,
		// unblock mutex at the same time
		pthread_cond_wait(&thread->task_assigned_cond_, &thread->task_assigned_mutex_);
		// when work condition is signaled,
		// mutex is locked again for this thread!

		// Now, check if the thread was signaled
		// with a new task
		if (thread->task_) {
			// acknowledges the task assigner that the thread
			// has received its task.
			pthread_mutex_lock(&thread->task_acked_mutex_);
			pthread_cond_signal(&thread->task_acked_cond_);
			pthread_mutex_unlock(&thread->task_acked_mutex_);

			thread->task_->Execute();
			delete thread->task_;
			thread->task_ = NULL;
		}
		// if not, probably the main thread has signaled
		// this thread to exit
		else {
			break; // exit main loop
		}
	}
	// release the mutex locked by work conditional variable
	pthread_mutex_unlock(&thread->task_assigned_mutex_);
	return NULL;
}

Worker::Worker() : task_(NULL) {
	// initialize task_assigned conditional
	// variable and its mutex object
	pthread_mutex_init(&task_assigned_mutex_, NULL);
	pthread_cond_init(&task_assigned_cond_, NULL);

	// initialize task_acked conditional variable
	// and its mutex object
	pthread_mutex_init(&task_acked_mutex_, NULL);
	pthread_cond_init(&task_acked_cond_, NULL);

	// initialize thread_started conditional
	// variable and its mutex object
	pthread_mutex_init(&thread_started_mutex_, NULL);
	pthread_cond_init(&thread_started_cond_, NULL);

	// creates the thread in a joinable state
	pthread_attr_init(&thread_attr_);
	pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);

	// To make sure that the thread is started before returning,
	// we lock the thread_started mutex and start the thread,
	// then wait for thread_started signal.
	//
	// This is to prevent the case when the thread is created
	// and not yet started, but the main thread already tries
	// to assign task to it, or is exiting, causing deadlock
	// states.
	pthread_mutex_lock(&thread_started_mutex_);
	pthread_create(&thread_, &thread_attr_, &worker_func, this);
	pthread_cond_wait(&thread_started_cond_, &thread_started_mutex_);
	pthread_mutex_unlock(&thread_started_mutex_);
}

Worker::~Worker() {
	// signal the thread to stop
	// but first must wait for it to finish
	// its current task first.
	pthread_mutex_lock(&task_assigned_mutex_);
	pthread_cond_signal(&task_assigned_cond_);
	pthread_mutex_unlock(&task_assigned_mutex_);
	// wait for it to stop
	pthread_join(thread_, NULL);

	// destroy everything
	pthread_cond_destroy(&task_assigned_cond_);
	pthread_mutex_destroy(&task_assigned_mutex_);

	pthread_cond_destroy(&task_acked_cond_);
	pthread_mutex_destroy(&task_acked_mutex_);

	pthread_cond_destroy(&thread_started_cond_);
	pthread_mutex_destroy(&thread_started_mutex_);

	pthread_attr_destroy(&thread_attr_);
}

bool Worker::TryAssign(Task* task) {
	// try locking the work condition, fails if
	// the worker is busy.
	if (pthread_mutex_trylock(&task_assigned_mutex_) == 0) {
		// the thread must be free to receive the new task.
		// by locking the mutex, it is guaranteed that
		// it should be free by now. This is to double
		// confirm it.
		assert(task_ == NULL && "The worker must be free to receive a new task.");
		// assign the task
		task_ = task;
		// signal the thread
		// make sure the task is received by the thread
		// otherwise, if Worker is destroyed immediately
		// after this function, the thread will wake up,
		// perform the task, then back to wait state, forever.
		pthread_mutex_lock(&task_acked_mutex_);
		pthread_cond_signal(&task_assigned_cond_);					// signal task_assigned event
		pthread_mutex_unlock(&task_assigned_mutex_);
		pthread_cond_wait(&task_acked_cond_, &task_acked_mutex_);	// wait for task_acked event
		pthread_mutex_unlock(&task_acked_mutex_);

		return true;
	}
	return false;
}
