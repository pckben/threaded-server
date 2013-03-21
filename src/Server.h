#ifndef SERVER_H
#define SERVER_H

namespace pckben {
   // Abstract Server class to listen to incoming requests.
  class Server {
   public:
     virtual ~Server() { }

     virtual void Start(int port);

   protected:
     virtual void HandleClient(int sock) = 0;

   private:
     int listener_sock_;
  };
}
#endif  // SERVER_H
