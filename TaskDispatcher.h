#ifndef TASKDISPATCHER_H
#define TASKDISPATCHER_H

#define MAX_N_THREAD	128

#include "semaphore.h"

namespace pckben {

	class Task;
	class Worker;

	class TaskDispatcher {
		public:

			/**
			 * Creates a dispatcher with the given
			 * number of available workers.
			 */
			TaskDispatcher(int size);

			/**
			 * Releases the dispatcher and all its
			 * workers.
			 */
			~TaskDispatcher();

			/**
			 * Dispatches the task to an available
			 * Worker. This will block if no worker
			 * is available at the moment.
			 */
			void Dispatch(Task* task);

		private:
			int size_;
			sem_t semaphore_;
			Worker** worker_;
	};
}
#endif /* TASKDISPATCHER_H */
