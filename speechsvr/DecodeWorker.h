#ifndef SPEECHSVR_DECODEWORKER_H
#define SPEECHSVR_DECODEWORKER_H

#include "protocol.h"
#include <Worker.h>
#include <matrix/kaldi-matrix.h>

namespace kaldi {
  class LatticeFasterDecoder;
  class AmDiagGmm;
  class TransitionModel;
  class DecodableInterface;
  class Mfcc;
  class OnlineCMN;
}

namespace fst {
  class SymbolTable;
}

namespace speechsvr {
  class DecodeWorker : public pckben::BackgroundWorker {
   public:
    DecodeWorker(kaldi::LatticeFasterDecoder *decoder,
                 kaldi::AmDiagGmm *gmm,
                 kaldi::TransitionModel *trans_model,
                 kaldi::Matrix<kaldi::BaseFloat> *lda_transform,
                 fst::SymbolTable *symbol_table,
                 float acoustic_scale,
                 float left_context,
                 float right_context);
    virtual ~DecodeWorker();

   protected:
    void Work();
    std::string Decode(kaldi::Matrix<kaldi::BaseFloat> &features);

   private:
    kaldi::LatticeFasterDecoder *decoder_;
    kaldi::DecodableInterface *decodable_;
    kaldi::AmDiagGmm *gmm_;
    kaldi::TransitionModel *trans_model_;
    kaldi::Matrix<kaldi::BaseFloat> *lda_transform_;
    fst::SymbolTable *symbol_table_;
    float acoustic_scale_;
    kaldi::Mfcc *mfcc_;
    kaldi::OnlineCMN *cmvn_;
    float left_context_;
    float right_context_;
  };
}
#endif  // SPEECHSVR_DECODEWORKER_H
