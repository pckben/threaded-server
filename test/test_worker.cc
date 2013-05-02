#include "Worker.h"
#include "Task.h"
#include <iostream>
#include "assert.h"
#include "time.h"
#include <unistd.h>

using namespace std;
using namespace pckben;

class TestTask : public Task {
};

class TestWorker : public BackgroundWorker {
 public:
   void Work() {
     cout << "Running..." << endl;
     sleep(1);
     cout << "Done." << endl;
   }
};

int main() { 
  TestWorker worker;
  Task task;
  assert(worker.TryAssign(&task) == true);

  // wait a very short while for the thread
  // to start working on the task.
  // if we return right away, the worker
  // will stop before doing its assigned task.
  usleep(100000);  //100ms

  return 0; // will wait for the worker to complete the task
}
