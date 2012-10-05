#include "DecodeServer.h"
#include "ThreadPool.h"
#include "DecoderRunner.h"
#include "DecodeThread.h"

using namespace ntu;

DecodeServer::DecodeServer(int nThreads) {
	nThreads_ = nThreads;
	pool_ = new ThreadPool(nThreads);

	for (int i=0; i<nThreads; i++) {
		decoderRunner_[i] = new DecoderRunner();
		thread_[i] = new DecodeThread(decoderRunner_[i]);
		pool_->AddThread(thread_[i]);
	}
}

DecodeServer::~DecodeServer() {
	for (int i=0; i<nThreads_; i++) {
		delete decoderRunner_[i];
		delete thread_[i];
	}
}

void DecodeServer::HandleClient(int sock) {
	// get available thread,
	// blocked if no thread available
	Thread* thread = pool_->GetThread();

	// Get the corresponding decoder runner
	DecoderRunner* runner = ((DecodeThread*)thread)->GetRunner();

	// pass sock to decoder
	runner->SetSocket(sock);

	// run on a the given thread
	runner->Run(thread);
}
