#ifndef DECODESERVER_H
#define THREADEDSERVER_H

#include "Server.h"

namespace pckben {
	class Task;
	class TaskDispatcher;

	/**
	 * Multi-threadded server, wraps the basic Server with
	 * a TaskDispatcher to handle multiple clients simultaneously
	 * on separate worker threads.
	 */
	class ThreadedServer: public Server {
		public:

			ThreadedServer(int nThreads);
			virtual ~ThreadedServer();

		protected: 
			/**
			 * Handles the connected socket by building
			 * a new Task and dispatching it to an available
			 * Worker.
			 */
			void HandleClient(int sock);
			
			/**
			 * Creates a Task from the given connected
			 * socket. This Task will be performed by
			 * a worker on a separated thread.
			 *
			 * Derived class should only care about what
			 * task should be carried out with this socket.
			 */
			virtual Task* CreateTask(int sock) = 0;

		private:
			TaskDispatcher* dispatcher_;
	};
}

#endif /* DECODESERVER_H */
