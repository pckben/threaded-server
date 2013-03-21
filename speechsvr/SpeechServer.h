#ifndef SPEECHSVR_SPEECHSERVER_H
#define SPEECHSVR_SPEECHSERVER_H

#include <ThreadedServer.h>
#include "Config.h"

#include <matrix/kaldi-matrix.h>
#include <fst/vector-fst.h>

namespace kaldi {
  class LatticeFasterDecoder;
  class AmDiagGmm;
  class TransitionModel;
  class DecodableInterface;
}

namespace fst {
  class SymbolTable;
}

namespace pckben {
  class Task;
  class Worker;
}

namespace speechsvr {
  class SpeechServer : public pckben::ThreadedServer {
   public:
    SpeechServer(int num_threads, std::string config_file);
    ~SpeechServer();

   protected:
    pckben::Task* CreateTask(int sock);
    pckben::Worker* CreateWorker();

   private:
    Config config_;
    kaldi::AmDiagGmm *gmm_;
    kaldi::TransitionModel *trans_model_;
    kaldi::Matrix<kaldi::BaseFloat> *lda_transform_;
    fst::VectorFst<fst::StdArc> *fst_;
    fst::SymbolTable *symbol_table_;
    float acoustic_scale_;
    float left_context_;
    float right_context_;
  };
}
#endif  // SPEECHSVR_SPEECHSERVER_H
