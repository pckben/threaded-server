#include "DecodeWorker.h"

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

#include <SocketTask.h>
#include <Socket.h>

using namespace speechsvr;
using namespace pckben;
using namespace std;
using namespace kaldi;
using namespace fst;

string SymbolsToWords(vector<int32> symbols, SymbolTable *symbol_table) {
  string words = "";
  for (vector<int32>::iterator it = symbols.begin();
       it != symbols.end();
       ++it) {
    string word = symbol_table->Find(*it);
    assert(word != "");
    words += word + " ";
  }
  return words;
}


void LdaTransform(Matrix<BaseFloat> &cmvn,
		Matrix<BaseFloat> &output,
		Matrix<BaseFloat> &lda_transform,
    kaldi::uint32 left_context, kaldi::uint32 right_context) {

  uint32 window_size = left_context + right_context + 1;
  uint32 window_center = left_context;
  int32 cur_feat = window_size - window_center - 1;
  uint32 trans_rows = lda_transform.NumRows();

  int32 feat_dim = cmvn.NumCols();
  Matrix<BaseFloat> feat_window;
  feat_window.Resize(window_size, feat_dim, kUndefined);
  int32 i;
  for (i = 0; i <= window_center; ++i)
    feat_window.CopyRowFromVec(cmvn.Row(0), i);
  for (; i < window_size; ++i)
    feat_window.CopyRowFromVec(cmvn.Row(i - window_center - 1), i);

  int32 window_pos = 0;

  Matrix<BaseFloat> spliced_feats;
  spliced_feats.Resize(cmvn.NumRows() - cur_feat,
      feat_dim * window_size,
      kUndefined);
  output.Resize(cmvn.NumRows() - cur_feat, trans_rows, kUndefined);

  int32 sfi = 0; // index of the current row in spliced_feats_
  for (; cur_feat < cmvn.NumRows(); ++cur_feat, ++sfi) {
    // append at the tail of the window and advance the effective window position
    feat_window.CopyRowFromVec(cmvn.Row(cur_feat), window_pos);
    window_pos = (window_pos + 1) % window_size;
    // splice the feature vectors in the current feature window
    int32 fwi = window_pos; // the oldest vector in feature window
    SubVector<BaseFloat> spliced_row(spliced_feats, sfi);
    for (int32 i = 0; i < window_size; ++i) {
      SubVector<BaseFloat> dst(spliced_row, i*feat_dim, feat_dim);
      dst.CopyRowFromMat(feat_window, fwi);
      fwi = (fwi + 1) % window_size;
    }
  }
  KALDI_VLOG(3) << "OnlineLdaInput::Compute AddMatMat";
  output.AddMatMat(1.0, spliced_feats, kNoTrans, lda_transform, kTrans, 0.0);
}

DecodeWorker::DecodeWorker(LatticeFasterDecoder *decoder,
                           AmDiagGmm *gmm,
                           TransitionModel *trans_model,
                           Matrix<BaseFloat> *lda_transform,
                           SymbolTable *symbol_table,
                           float acoustic_scale,
                           float left_context,
                           float right_context)
: decoder_(decoder),
  gmm_(gmm),
  trans_model_(trans_model),
  lda_transform_(lda_transform),
  symbol_table_(symbol_table),
  acoustic_scale_(acoustic_scale),
  left_context_(left_context),
  right_context_(right_context)
{
  MfccOptions mfcc_opts;
  mfcc_opts.use_energy = false;
  mfcc_opts.vtln_warp = 1.0;
  mfcc_opts.frame_opts.frame_length_ms = 25;
  mfcc_opts.frame_opts.frame_shift_ms = 10;

  mfcc_ = new Mfcc(mfcc_opts);
  const int32 feat_dim = mfcc_opts.num_ceps;
  const int32 cmvn_window = 600;
  cmvn_ = new OnlineCMN(feat_dim,cmvn_window);
}

DecodeWorker::~DecodeWorker() {
  delete decoder_;
  delete mfcc_;
  delete cmvn_;
}

void DecodeWorker::Work() {
  Socket *sock = ((SocketTask *)GetTask())->GetSocket();

  // Receive header
  Packet packet;
  sock->Receive((char *)&packet.header, sizeof(PacketHeader));

  cout << "Packet received: type=" << packet.header.type
       << ", length=" << packet.header.payload_length << endl;

  Vector<BaseFloat> wave;  // raw wave data
  Matrix<BaseFloat> mfcc_output;
  Matrix<BaseFloat> cmvn_output;
  Matrix<BaseFloat> lda_output;
  string words;

  PacketHeader response_header;

  switch (packet.header.type) {
    case DATA_WAVE:
      // Receive wave
      wave.Resize(packet.header.payload_length / sizeof(float));
      cout << "Waiting for " << packet.header.payload_length << " bytes ("
           << packet.header.payload_length / sizeof(float) << " floats)\n";
      sock->Receive((char *)wave.Data(), packet.header.payload_length);
      cout << packet.header.payload_length << " bytes received.\n";
      // Feature extraction
      cout << "Feature extraction...\n";
      mfcc_->Compute(wave, &mfcc_output, NULL);
      cout << "CMVN...\n";
      cmvn_->ApplyCmvn2(mfcc_output, &cmvn_output);
      cout << "LDA transform...\n";
      LdaTransform(cmvn_output, lda_output, *lda_transform_, left_context_, right_context_);
      cout << "Decode...\n";
      words = Decode(lda_output);
      cout << "Output: " << words << endl;

      // Send back result
      response_header.payload_length = words.length();
      response_header.type = DECODE_OUTPUT;
      sock->Send((char *)&response_header, sizeof(PacketHeader));
      sock->Send(words.c_str(), words.length());
      break;

    case DATA_FEATURE:
      break;

    default:
      cerr << "Invalid request received.\n";
      break;
  }
}

string DecodeWorker::Decode(Matrix<BaseFloat> &features) {
  VectorFst<LatticeArc> decoded;
  LatticeWeight weight;
  vector<int32> alignment;
  vector<int32> symbols;
  string words;

  // Decode
  decodable_ = new DecodableAmDiagGmmScaled(*gmm_, *trans_model_, features,
                                            acoustic_scale_);
  decoder_->Decode(decodable_);
  // Traceback
  cout << "Traceback...\n";
  decoder_->GetBestPath(&decoded);
  // Get output
  cout << "Get output...\n";
  GetLinearSymbolSequence(decoded, &alignment, &symbols, &weight);
  words = SymbolsToWords(symbols, symbol_table_);
  delete decodable_;
  return words;
}
