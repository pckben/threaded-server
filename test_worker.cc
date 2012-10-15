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
			sleep(3);
			cout << "Done." << endl;
		}
};

int main() { 
	Worker worker;
	Task* task = new TestTask();
	assert(worker.TryAssign(task) == true);
	return 0; // will wait for the worker to complete the task
}

