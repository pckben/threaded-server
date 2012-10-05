#include "Thread.h"
#include "ThreadPool.h"
#include "Runner.h"
#include <iostream>

using namespace std;
using namespace ntu;

class TestRunner : public Runner {
	protected:
		void DoRun() {
			cout << "Running..." << endl;
			sleep(1);
			cout << "Done." << endl;
		}
};

int main() { 
	Thread thread;
	Runner* runner = new TestRunner();
	runner->Run(&thread);

	cin.get();

	delete runner;

	return 0;
}

