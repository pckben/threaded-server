#include "Server.h"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

using namespace std;
using namespace pckben;

void Die(string msg) { perror(msg.c_str()); exit(1); }

void Server::Start(int port) {
	/* Create the TCP socket */
	if ((listener_sock_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		Die("Failed to create socket");
	}
	
	struct sockaddr_in serveraddr;
	/* Construct the server sockaddr_in structure */
	memset(&serveraddr, 0, sizeof(serveraddr));       /* Clear struct */
	serveraddr.sin_family = AF_INET;                  /* Internet/IP */
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
	serveraddr.sin_port = htons(port);       /* server port */


	/* Bind the server socket */
	if (bind(listener_sock_, (struct sockaddr *) &serveraddr,
				sizeof(serveraddr)) < 0) {
		Die("Failed to bind the server socket");
	}
	/* Listen on the server socket */
	if (listen(listener_sock_, SOMAXCONN) < 0) {
		Die("Failed to listen on server socket");
	}

	struct sockaddr_in clientaddr;

	cout << "Listening on 0.0.0.0:" << port << endl;

	/* Run until cancelled */
	while (1) {
		int clientsock;
		unsigned int clientlen = sizeof(clientaddr);
		/* Wait for client connection */
		if ((clientsock =
					accept(listener_sock_, (struct sockaddr *) &clientaddr,
						&clientlen)) < 0) {
			Die("Failed to accept client connection");
		}
		printf("Client connected: %s\n", inet_ntoa(clientaddr.sin_addr));

		HandleClient(clientsock);
	}
}

