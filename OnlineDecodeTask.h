#ifndef ONLINEDECODETASK_H
#define ONLINEDECODETASK_H

#include "Task.h"

namespace pckben {
	class Socket;

	class OnlineDecodeTask : public Task {
		public:
			OnlineDecodeTask(int sock);
			~OnlineDecodeTask();

			void Execute();

		private:
			Socket* socket_;
	};
}
#endif /* ONLINEDECODETASK_H */
