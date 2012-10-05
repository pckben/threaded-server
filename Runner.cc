#include "Runner.h"
#include "Thread.h"

using namespace ntu;

void Runner::Run(Thread* thread) {
	thread->Run(&Runner::run_function, this);
}
