#include "Socket.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstdio>
#include <string>
#include <cstdio>

#define BUFFSIZE 32

using namespace pckben;
using namespace std;

Socket::Socket(int socket) 
: socket_(socket) {
}

Socket::Socket() {
}

void Socket::Connect(char* serveraddr, int port) {
	struct sockaddr_in addr;

	/* Create the TCP socket */
	if ((socket_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("Failed to create socket");
		exit(1);
	}

	/* Construct the server sockaddr_in structure */
	memset(&addr, 0, sizeof(addr));       /* Clear struct */
	addr.sin_family = AF_INET;                  /* Internet/IP */
	addr.sin_addr.s_addr = inet_addr(serveraddr);  /* IP address */
	addr.sin_port = htons(port);       /* server port */
	/* Establish connection */
	if (connect(socket_,
				(struct sockaddr *) &addr,
				sizeof(addr)) < 0) {
		perror("Failed to connect with server");
		exit(1);
	}

}

void Socket::Send(char* data, int length) {
	if (send(socket_, data, length, 0) != length) {
		perror("Mismatch in number of sent bytes");
		exit(1);
	}
}

void Socket::Receive(char* data, int length) {
	int received = 0;
	while (received < length) {
		int bytes = 0;
		if ((bytes = recv(socket_, data, BUFFSIZE-1, 0)) < 1) {
			perror("Failed to receive bytes from server");
			exit(1);
		}
		received += bytes;
		data[bytes] = '\0';        /* Assure null terminated string */
	}

}
