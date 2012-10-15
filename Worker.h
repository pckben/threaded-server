#ifndef THREAD_H
#define WORKER_H

#include <pthread.h>

namespace pckben {
	class Task;

	class Worker {
		public:
			/**
			 * Creates the worker and initialize its
			 * worker thread, which will immediately
			 * be put into waiting state to wait for
			 * a new Task to be assigned.
			 */
			Worker();

			/**
			 * The destructor will wait for the worker
			 * thread to complete before releasing its
			 * resources.
			 *
			 * NOTE: If the worker is being destroyed before
			 * the thread has started, the thread will
			 * wait forever! Thus, the main thread should
			 * make sure it should pause for sometime
			 *
			 */
			~Worker(); 

			/**
			 * Try assigning the Task to this Worker.
			 * @return true if successful, in which case
			 * the worker starts working on the task
			 * immediately. If the worker is busy at the
			 * moment, this will return false.
			 *
			 * Once finished, the Worker will destroy the
			 * Task himself.
			 */
			bool TryAssign(Task* task);

		private:
			/**
			 * making the thread running function friend
			 * of this class so it can access its private
			 * members
			 */
			friend void* worker_func(void* args);

			pthread_t thread_;
			pthread_attr_t thread_attr_;

			// conditional variable to wait for task
			// assigned event.
			pthread_cond_t task_assigned_cond_;
			pthread_mutex_t task_assigned_mutex_;

			// conditional variable to acknowledge the
			// task dispatcher that the task has been
			// received by the worker thread. This is
			// to prevent deadlock when the Worker is
			// destroyed immediately after being
			// assigned a new task.
			pthread_cond_t task_acked_cond_;
			pthread_mutex_t task_acked_mutex_;

			// conditional variable to wait for thread
			// to fully started.
			pthread_cond_t thread_started_cond_;
			pthread_mutex_t thread_started_mutex_;

			Task* task_;
	};

	void* worker_func(void* args);
}
#endif /* THREAD_H */
