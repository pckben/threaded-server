#ifndef SERVER_H
#define SERVER_H

#define MAXPENDING 5

namespace ntu {
	/**
	 * Abstract Server class to listen to incoming requests.
	 */
	class Server {
		public:

			void Start(int port);

		protected:
			virtual void HandleClient(int sock) = 0;

		private:
			int listener_sock_;
	};
}
#endif /* SERVER_H */
