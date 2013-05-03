#ifndef SOCKET_H
#define SOCKET_H

namespace pckben {

  #ifndef _WIN32 || _WIN64
  typedef int SOCKET;
  #endif

  // Wraps over a UNIX socket and provide
  // helper functions to send/receive data.
  class Socket {
   public:
     // Creates a new Socket
     Socket();
     // Creates a Socket that wraps the given
     // UNIX socket.
     Socket(SOCKET socket);

     void Connect(char* server, int port);

     void Send(const void* data, int length);
     void Receive(void* data, int length);

   private:
     SOCKET socket_;
  };
}

#endif  // SOCKET_H
