#include "Semaphore.h"
#include "stddef.h"

using namespace pckben;

Semaphore::Semaphore(int count) {
  counter_ = count;
  pthread_mutex_init(&lock_, NULL);
  pthread_cond_init(&counter_cond_, NULL);
}

Semaphore::~Semaphore() {
  pthread_mutex_destroy(&lock_);
  pthread_cond_destroy(&counter_cond_);
}

void Semaphore::Wait() {
  pthread_mutex_lock(&lock_);
  while(counter_ <= 0)
    pthread_cond_wait(&counter_cond_, &lock_);
  counter_--;
  pthread_mutex_unlock(&lock_);
}

void Semaphore::Signal() {
  pthread_mutex_lock(&lock_);
  counter_++;
  pthread_cond_signal(&counter_cond_);
  pthread_mutex_unlock(&lock_);
}

int Semaphore::GetValue() {
  pthread_mutex_lock(&lock_);
  int value = counter_;
  pthread_mutex_unlock(&lock_);
  return value;
}
