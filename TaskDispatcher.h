#ifndef TASKDISPATCHER_H
#define TASKDISPATCHER_H

#include "Task.h"

namespace pckben {

	class Worker;
	class Semaphore;

	class TaskDispatcher : public TaskObserver {
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

			void OnTaskDone(Task* task);

		private:
			int size_;
			Semaphore* semaphore_;
			Worker** worker_;
	};
}
#endif /* TASKDISPATCHER_H */
