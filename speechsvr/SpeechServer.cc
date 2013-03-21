#include "SpeechServer.h"
#include <SocketTask.h>
#include <Worker.h>

#include <string>
#include <fstream>
#include <iostream>

#include <base/kaldi-common.h>
#include <util/common-utils.h>
#include <gmm/am-diag-gmm.h>
#include <tree/context-dep.h>
#include <hmm/transition-model.h>
#include <fstext/fstext-lib.h>
#include <decoder/lattice-faster-decoder.h>
#include <decoder/decodable-am-diag-gmm.h>
#include <util/timer.h>
#include <feat/feature-functions.h>  // feature reversal
#include <feat/feature-mfcc.h>
#include <online/online-cmn.h>

#include "DecodeWorker.h"

using namespace pckben;
using namespace speechsvr;
using namespace std;
using namespace kaldi;
using namespace fst;

SpeechServer::SpeechServer(int num_threads, string config_file)
: ThreadedServer(num_threads), config_(config_file)
{
  acoustic_scale_ = config_.GetFloat("acoustic_scale");
  //float beam = config_.GetFloat("beam");
  //int max_active = config_.GetInt("max_active");
  left_context_ = config_.GetInt("left_context");
  right_context_ = config_.GetInt("right_context");

  // Load models
  bool binary;
  string model_file = config_.GetString("model_file");
  cout << "Loading models from '" << model_file << "'\n";
  Input ki(model_file, &binary);
  gmm_ = new AmDiagGmm();
  trans_model_ = new TransitionModel();
  trans_model_->Read(ki.Stream(), binary);
  gmm_->Read(ki.Stream(), binary);

  string lda_file = config_.GetString("lda_file");
  cout << "Loading LDA matrix from '" << lda_file << "'\n";
  Input lda_input(lda_file, &binary);
  lda_transform_ = new Matrix<BaseFloat>();
  lda_transform_->Read(lda_input.Stream(), binary);

  // Load symbol table
  string symbol_table_file = config_.GetString("symbol_file");
  cout << "Loading symbol table from '" << symbol_table_file << "'\n";
  symbol_table_ = SymbolTable::ReadText(symbol_table_file);

  // Load FST, this can take very long.
  string fst_file = config_.GetString("fst_file");
  cout << "Loading FST from '" << fst_file << "'\n";
  ifstream fst_in(fst_file.c_str(), ifstream::binary);
  if (!fst_in.good())
    cerr << "Could not open decoding-graph FST " << fst_file << endl;
  fst_ = VectorFst<StdArc>::Read(fst_in, FstReadOptions(fst_file));
  assert(fst_ != NULL); // fst code will warn.
}

SpeechServer::~SpeechServer() {
  delete gmm_;
  delete trans_model_;
  delete lda_transform_;
}

Task* SpeechServer::CreateTask(int sock) {
  return new SocketTask(sock);
}

// create a new DecodeWorker and pass in the pre-configured params
Worker* SpeechServer::CreateWorker() {
  LatticeFasterDecoderConfig config;
  config.beam = config_.GetFloat("beam");
  config.max_active = config_.GetFloat("max_active");
  config.lattice_beam = config_.GetFloat("lattice_beam");
  LatticeFasterDecoder *decoder = new LatticeFasterDecoder(*fst_, config);
  return new DecodeWorker(decoder, gmm_, trans_model_, lda_transform_,
                          symbol_table_,
                          acoustic_scale_,
                          left_context_, right_context_);
}
