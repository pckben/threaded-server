#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <pthread.h>

namespace pckben {
  class Semaphore {
   public:
     Semaphore(int count);
     virtual ~Semaphore();
     void Signal();
     void Wait();
     int GetValue();

   private:
     pthread_mutex_t lock_;
     pthread_cond_t counter_cond_;
     int counter_;
  };
}

#endif  // SEMAPHORE_H_
