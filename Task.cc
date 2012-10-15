#include "Task.h"

using namespace pckben;

void Task::Done() {
	if (observer_)
		observer_->OnTaskDone(this);
}
