#include "Worker.h"
#include "Task.h"
#include <iostream>
#include "assert.h"
#include "time.h"

using namespace std;
using namespace pckben;

class TestTask : public Task {
	public:
		void Execute() {
			cout << "Running..." << endl;
			sleep(1);
			cout << "Done." << endl;
		}
};

int main() { 
	Worker worker;
	Task* task = new TestTask();
	assert(worker.TryAssign(task) == true);

	// wait a very short while for the thread
	// to start working on the task.
	// if we return right away, the worker
	// will stop before doing its assigned task.
	usleep(100000);	//100ms

	return 0; // will wait for the worker to complete the task
}
