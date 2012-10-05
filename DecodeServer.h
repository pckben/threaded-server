#ifndef DECODESERVER_H
#define DECODESERVER_H

#include "Server.h"

namespace ntu {
	class Thread;
	class ThreadPool;
	class DecoderRunner;

	/**
	 * Multi-threadded server for decoding
	 */
	class DecodeServer: public Server {
		public:

			DecodeServer(int nThreads);
			~DecodeServer();

		protected: 
			void HandleClient(int sock);
			
		private:
			int nThreads_;
			ThreadPool* pool_;
			Thread* thread_[128];
			DecoderRunner* decoderRunner_[128];
	};
}

#endif /* DECODESERVER_H */
