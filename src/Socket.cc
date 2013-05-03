#include "Socket.h"

#ifdef _WIN32 || _WIN64
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFSIZE 32

using namespace pckben;
using namespace std;

#ifdef _WIN32 || _WIN64
static bool __wsa_init_ = false;
#endif

Socket::Socket(SOCKET socket)
: socket_(socket) {
  #ifdef _WIN32 || _WIN64
  if (!__wsa_init_) {
		int wsaerr;
		WORD wVersionRequested;
		WSADATA wsaData;
		wVersionRequested = MAKEWORD(2,2);
		wsaerr = WSAStartup(wVersionRequested,&wsaData);
		if (wsaerr != 0) {
			state = CSOCKET_ERR; return;
		}else
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion)!=2) {
			state = CSOCKET_ERR; return;
		} else { // success
			wsa_initialized = true;
		}
  }
  #endif
}

Socket::Socket() {
}

void Socket::Connect(char* serveraddr, int port) {
  struct sockaddr_in addr;

  // Create the TCP socket
  if ((socket_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("Failed to create socket");
    exit(1);
  }

  // Construct the server sockaddr_in structure
  memset(&addr, 0, sizeof(addr));       // Clear struct
  addr.sin_family = AF_INET;                  // Internet/IP
  addr.sin_addr.s_addr = inet_addr(serveraddr);  // IP address
  addr.sin_port = htons(port);       // server port
  // Establish connection
  if (connect(socket_,
        (struct sockaddr *) &addr,
        sizeof(addr)) < 0) {
    perror("Failed to connect with server");
    exit(1);
  }

}

void Socket::Send(const void* data, int length) {
  if (send(socket_, data, length, 0) != length) {
    perror("Mismatch in number of sent bytes");
    exit(1);
  }
}

void Socket::Receive(void* data, int length) {
  int received = 0;
  while (received < length) {
    int bytes = 0;
    if ((bytes = recv(socket_, (char *)data + received, length - received, 0)) < 1) {
      perror("Failed to receive bytes from remote");
      exit(1);
    }
    received += bytes;
  }
}
