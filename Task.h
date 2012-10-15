#ifndef TASK_H
#define TASK_H

namespace pckben {
	class TaskObserver;

	/**
	 * Defines the methods of a Task that will
	 * be executed by a Worker
	 */
	class Task {
		public:
			Task() : observer_(0) { }

			virtual ~Task() { }
			virtual void Execute() =0;

			void SetObserver(TaskObserver* observer) { observer_ = observer; }

			/**
			 * Call this to notify the observer
			 * that the task has been done.
			 */
			void Done();

		private:
			TaskObserver* observer_;
	};

	class TaskObserver {
		public:
			virtual ~TaskObserver() { }
			virtual void OnTaskDone(Task* task) = 0;
	};
}

#endif /* TASK_H */
