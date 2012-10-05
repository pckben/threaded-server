#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include "Thread.h"

namespace ntu {
	class DecoderRunner;

	class DecodeThread : public Thread {
		public:
			DecodeThread(DecoderRunner* runner);

			DecoderRunner* GetRunner() { return runner_; }

		private:
			DecoderRunner* runner_;
	};
}

#endif /* DECODETHREAD_H */
