#include "Thread.h"
#include "ThreadListener.h"

using namespace ntu;

Thread::Thread() {
	isFree_ = true;
}

Thread::~Thread() {
}

void Thread::Run(RunFunction* func, void* args) {
	pthread_create(&thread_, NULL, func, args);
}

void Thread::Finish() { 
	isFree_ = true;
	listener_->ThreadFinished(this); 
}
