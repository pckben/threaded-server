#include "DecodeWorker.h"
#include "RemoteAudioSource.h"

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

//#include "online/online-feat-input.h"
#include "online-decoder/online-decodable.h"
#include "online-decoder/online-faster-decoder.h"
#include "online-decoder/onlinebin-util.h"
#include "online-decoder/online-cmn.h"
#include "online-decoder/online-vad.h"

#include <SocketTask.h>
#include <Socket.h>

using namespace speechsvr;
using namespace pckben;
using namespace std;
using namespace kaldi;
using namespace fst;

void SendOutput(Socket *socket, bool end_utterance, string str) {
  PacketHeader header;
  header.payload_length = sizeof(bool) + str.length();
  header.type = DECODE_OUTPUT;
  socket->Send(&header, sizeof(PacketHeader));
  socket->Send(&end_utterance, sizeof(bool));
  socket->Send(str.c_str(), str.length());
}

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

string GetPartialResult(const std::vector<int32>& words,
                      const fst::SymbolTable *word_syms) {
  KALDI_ASSERT(word_syms != NULL);
  string result = "";
  for (size_t i = 0; i < words.size(); i++) {
    string word = word_syms->Find(words[i]);
    if (word == "")
      KALDI_ERR << "Word-id " << words[i] <<" not in symbol table.";
    result += word + " ";
  }
  return result;
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
                           float right_context,
                           VectorFst<StdArc> *decode_fst)
: decoder_(decoder),
  gmm_(gmm),
  trans_model_(trans_model),
  lda_transform_(lda_transform),
  symbol_table_(symbol_table),
  acoustic_scale_(acoustic_scale),
  left_context_(left_context),
  right_context_(right_context),
  decode_fst_(decode_fst)
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
  PacketHeader header;
  sock->Receive((char *)&header, sizeof(PacketHeader));

  if (header.type != DECODE_REQUEST)
    exit(1);

  DecodeRequest req;
  sock->Receive((char *)&req, header.payload_length);

  cout << "Decode request: "
       << (req.online_mode ? "online mode" : "file mode") << endl;

  DecodeResponse res = { true };
  SendPacket(sock, DECODE_ACCEPT, sizeof(DecodeResponse), &res);

  Vector<BaseFloat> wave;  // raw wave data
  Matrix<BaseFloat> mfcc_output;
  Matrix<BaseFloat> cmvn_output;
  Matrix<BaseFloat> lda_output;
  string words;

  if (req.online_mode) {
    OnlineDecode(sock);
  }
  else {
    sock->Receive((char *)&header, sizeof(PacketHeader));
    if (header.type == DATA_WAVE) {
      // Receive wave
      wave.Resize(header.payload_length / sizeof(float));
      cout << "Waiting for " << header.payload_length << " bytes ("
           << header.payload_length / sizeof(float) << " floats)\n";
      sock->Receive((char *)wave.Data(), header.payload_length);
      cout << header.payload_length << " bytes received.\n";
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
      SendOutput(sock, true, words);
    }
    else if (header.type == DATA_FEATURE) {
    }
    else {
      cerr << "Invalid request received: " << header.type << endl;
    }
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

void DecodeWorker::OnlineDecode(Socket *sock) {

  const int32 kSampleFreq = 16000;
  int32 vad_buffer_length_ms = 500;
  int32 vad_hangover_ms = 500;
  int32 batch_size = 27;
  int32 frame_length_ms = 25;
  int32 frame_shift_ms = 10;
  float vad_onset_threshold = 400;
  float vad_offset_threshold = 50;
  float vad_recover_threshold = 100;
  int frame_length_samples = frame_length_ms * (kSampleFreq/1000);
  int frame_shift_samples = frame_shift_ms * (kSampleFreq/1000);
  int vad_hangover_samples = vad_hangover_ms * (kSampleFreq/1000);
  int vad_hangover_frames = (vad_hangover_samples - frame_length_samples) / frame_shift_samples + 1;
  MfccOptions mfcc_opts;

  OnlineFasterDecoderOpts config;
  string silence_phones_str = "1:2:3:4:5:6:7:8:9:10:11:12:13:14:15";
  std::vector<int32> silence_phones;
  if (!SplitStringToIntegers(silence_phones_str, ":", false, &silence_phones))
    KALDI_ERR << "Invalid silence-phones string " << silence_phones_str;
  if (silence_phones.empty())
    KALDI_ERR << "No silence phones given!";
  VectorFst<LatticeArc> out_fst;

  OnlineFasterDecoder decoder(*decode_fst_, config,
      silence_phones, *trans_model_);

  RemoteAudioSource audioSource(sock);

  SimpleEnergyVad vad(vad_onset_threshold, vad_offset_threshold, 
      vad_recover_threshold, vad_hangover_frames);

  OnlineVadFeInput vadMfccInput(&audioSource, mfcc_, &vad,
      frame_length_samples,
      frame_shift_samples,
      vad_buffer_length_ms * (kSampleFreq / 1000));

  OnlineCmvnInput cmvn_input(&vadMfccInput, mfcc_opts.num_ceps, 600);
  OnlineLdaInput lda_input(&cmvn_input, mfcc_opts.num_ceps, *lda_transform_,
      left_context_, right_context_);
  int32 feat_dim = lda_transform_->NumRows();
  OnlineDecodableDiagGmmScaled decodable(&lda_input, *gmm_, *trans_model_,
      acoustic_scale_, batch_size,
      feat_dim, -1);

  // setup VAD-->Decoder event notification
  OnlineFasterDecoderVadListener vadListener(&decoder);
  vadMfccInput.AddVadListener(&vadListener);

  OnlineFasterDecoder::DecodeState dstate;
  string utterance = "";

  while (true) {
    dstate = decoder.Decode(&decodable);

    if (dstate & (decoder.kEndUtt | decoder.kEndFeats)) {
      std::vector<int32> word_ids;
      decoder.FinishTraceBack(&out_fst);
      fst::GetLinearSymbolSequence(out_fst,
          static_cast<vector<int32> *>(0), &word_ids,
          static_cast<LatticeArc::Weight*>(0));
      string result = GetPartialResult(word_ids, symbol_table_);
      utterance += result;
      utterance = "";

      SendOutput(sock, true, result);
      cerr << result << endl;

    } else if (dstate & (decoder.kEndBatch)) {
      std::vector<int32> word_ids;
      if (decoder.PartialTraceback(&out_fst)) {
        fst::GetLinearSymbolSequence(out_fst,
            static_cast<vector<int32> *>(0), &word_ids,
            static_cast<LatticeArc::Weight*>(0));
        string result = GetPartialResult(word_ids, symbol_table_);
        utterance += result;

        SendOutput(sock, false, result);

        cerr << result;
      }
    }

    /*
    if (dstate == decoder.kEndFeats)
      std::cerr << "*";
    else if (dstate == decoder.kEndBatch) {
    } else if (dstate == decoder.kEndUtt) {
      std::cerr << "\n";
    } else
      std::cerr << "?";
    */

    if (dstate == decoder.kEndFeats)
      break;
  }
}
