#ifndef THREADPOOL_H
#define THREADPOOL_H

#define MAX_N_THREAD	128

#include "Thread.h"
#include "ThreadListener.h"
#include "semaphore.h"

namespace ntu {

	class ThreadPool : public ThreadListener {
		public:
			ThreadPool(int size);
			~ThreadPool();

			/**
			 * Get the next available thread if any.
			 * Blocked if no thread available.
			 */
			Thread* GetThread();

			void AddThread(Thread* thread);

		protected:
			void ThreadFinished(Thread* thread);

		private:
			int size_;
			sem_t semaphore_;
			Thread* thread_[MAX_N_THREAD];
	};
}
#endif /* THREADPOOL_H */
