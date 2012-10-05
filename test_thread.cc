#include "Thread.h"
#include "ThreadPool.h"
#include "pthread.h"
#include <iostream>

using namespace std;
using namespace ntu;

struct thread_info {
	string name;
	int interval;
};

static void* do_decode(void* args) {
	thread_info* info = (thread_info*)args;
	cout << "Thread " << info->name << ": Decoding..." << endl;
	sleep(info->interval);
	cout << "Thread " << info->name << ": Decode finished." << endl;
}

static void* do_postProcessing(void* args) {
	thread_info* info = (thread_info*)args;
	cout << "Thread " << info->name << ": Post processing..." << endl;
	sleep(info->interval);
	cout << "Thread " << info->name << ": Post processing finished." << endl;
}

int main() {
	static int N = 2;
	ThreadPool pool(N);
	Thread th[N];
	thread_info info[N];

	for (int i=0; i<N; i++)
		pool.AddThread(&th[i]);

	Thread* decoderThread = pool.GetThread();
	info[0].name = "Decoder";
	info[0].interval = 2;

	Thread* postProcessingThread = pool.GetThread();
	info[1].name = "Post Processing";
	info[1].interval = 3;

	decoderThread->Run(&do_decode, &info[0]);
	sleep(1);
	postProcessingThread->Run(&do_postProcessing, &info[1]);

	cin.get();

	return 0;
}
