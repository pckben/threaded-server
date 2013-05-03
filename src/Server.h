#ifndef SERVER_H
#define SERVER_H

#include "Socket.h"

namespace pckben {
   // Abstract Server class to listen to incoming requests.
  class Server {
   public:
     virtual ~Server() { }

     virtual void Start(int port);

   protected:
     virtual void HandleClient(SOCKET sock) = 0;

   private:
     SOCKET listener_sock_;
  };
}
#endif  // SERVER_H
