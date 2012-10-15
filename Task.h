#ifndef TASK_H
#define TASK_H

namespace pckben {
	/**
	 * Defines the methods of a Task that will
	 * be executed by a Worker
	 */
	class Task {
		public:
			virtual ~Task() { }
			virtual void Execute() =0;
	};
}

#endif /* TASK_H */
