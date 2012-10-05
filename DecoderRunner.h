#ifndef DECODERRUNNER_H
#define DECODERRUNNER_H

#include "Runner.h"

namespace ntu {
	class Socket;

	class DecoderRunner : public Runner {
		public:
			DecoderRunner();
			~DecoderRunner();

			void SetSocket(int sock);

		protected:
			void DoRun();

		private:
			void Decode();

			Socket* socket_;
	};
}
#endif /* DECODERRUNNER_H */
