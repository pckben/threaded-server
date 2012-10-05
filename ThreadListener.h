#ifndef THREADLISTENER_H
#define THREADLISTENER_H

namespace ntu {
	class Thread;

	/**
	 * Interface ThreadListener
	 */
	class ThreadListener {
		public: 
			/**
			 * Event ThreadFinished occurs at the end of a thread life time
			 */
			virtual void ThreadFinished(Thread* thread) = 0;
	};
}
#endif /* THREADLISTENER_H */
