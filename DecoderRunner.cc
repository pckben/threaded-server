#include "DecoderRunner.h"
#include "Socket.h"

using namespace ntu;

DecoderRunner::DecoderRunner()
	: socket_(0) {
}

DecoderRunner::~DecoderRunner() {
	if (socket_)
		delete socket_;
}

void DecoderRunner::SetSocket(int sock) {
	if (socket_)
		delete socket_;

	// creates a new Socket object that 
	// wraps the given UNIX socket.
	socket_ = new Socket(sock);
}

void DecoderRunner::DoRun() {
	Decode();
}

#include <iostream>
using namespace std;
void DecoderRunner::Decode() {
	// TODO
	cout << "Decoder started" << endl;
	char msg[100];
	socket_->Receive(msg, 10);
	cout << "Received: " << string(msg) << endl;
	socket_->Send(msg, 10);
	cout << "Sent: " << string(msg) << endl;

}
