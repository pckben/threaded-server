#ifndef SOCKET_H
#define SOCKET_H

namespace pckben {
  // Wraps over a UNIX socket and provide
  // helper functions to send/receive data.
  class Socket {
   public:
     // Creates a new Socket
     Socket();
     // Creates a Socket that wraps the given
     // UNIX socket.
     Socket(int socket);

     void Connect(char* server, int port);

     void Send(const char* data, int length);
     void Receive(char* data, int length);

   private:
     int socket_;
  };
}

#endif  // SOCKET_H
