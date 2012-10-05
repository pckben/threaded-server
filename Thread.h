#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

namespace ntu {
	class ThreadListener;

	typedef void* (RunFunction)(void*);

	class Thread {
		public:
			Thread();
			~Thread(); 

			bool IsFree() const { return isFree_; }

			/**
			 * The thread is being used, but not yet running
			 */
			void Use() { isFree_ = false; }

			/**
			 * Starts the thread.
			 */
			void Run(RunFunction* func, void* args);

			void SetThreadListener(ThreadListener* listener) { listener_ = listener; }

		protected:

			/**
			 * To be called by the main running function once finished.
			 */
			void Finish(); 

		private:
			bool isFree_;
			pthread_t thread_;
			ThreadListener* listener_;
	};
}
#endif /* THREAD_H */
