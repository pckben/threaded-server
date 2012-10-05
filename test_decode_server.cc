#include "DecodeServer.h"
#include <iostream>
#include <cstdlib>

#define N_THREADS 5

using namespace std;
using namespace ntu;

int main(int argc, char* argv[]) {
	if (argc < 2) {
		cout << "Syntax: test_decode_server <port>" << endl;
		return 1;
	}

	int port = atoi(argv[1]);

	DecodeServer server(N_THREADS);
	server.Start(port);

	return 0;
}
