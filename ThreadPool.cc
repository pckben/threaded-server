#include "ThreadPool.h"
#include "Thread.h"

using namespace ntu;

ThreadPool::ThreadPool(int size) {
	size_ = 0;
	sem_init(&semaphore_, 0, size); 
}

ThreadPool::~ThreadPool() {
	sem_destroy(&semaphore_);
}

Thread* ThreadPool::GetThread() {
	// wait until at least a thread is free
	sem_wait(&semaphore_);

	// find the first free thread
	for (int i=0; i<size_; i++)
		if (thread_[i]->IsFree()) {
			thread_[i]->Use();
			return thread_[i];
		}
}

void ThreadPool::AddThread(Thread* thread) {
	thread_[size_++] = thread;
}

void ThreadPool::ThreadFinished(Thread* thread) {
	sem_post(&semaphore_);
}
