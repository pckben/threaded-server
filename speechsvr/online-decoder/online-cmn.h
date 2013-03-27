// runon/run-on-cmvn.h

// Copyright 2012 Cisco Systems (author: Matthias Paulik)

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.


#ifndef KALDI_ONLINE_CMN_H_
#define KALDI_ONLINE_CMN_H_

#include <vector>

#include "base/kaldi-common.h"
#include "matrix/matrix-lib.h"
#include "hmm/transition-model.h"

namespace kaldi {

//support for online cepstral mean normalization (variance normalization not 
//supported here) -- it's just doing a simple moving average over a history 
//of n preceding frames ...
class OnlineCMN {

 public:
  OnlineCMN(int32 dim, int32 history);


  void ApplyCmvn(const MatrixBase<BaseFloat> &feats,
                 Matrix<BaseFloat> *norm_feats);

  // do exact cmvn, not just mean substraction
  void ApplyCmvn2(const MatrixBase<BaseFloat> &feats,
                 Matrix<BaseFloat> *norm_feats);

 private:
  void ComputeMean(const MatrixBase<BaseFloat> &feats,
                   Matrix<BaseFloat> *norm_feats);
// for cmvn
  void Compute(const MatrixBase<BaseFloat> &feats,
               Matrix<BaseFloat> *norm_feats);

  int32 oldest_row_;
  int32 dim_;
  int32 hist_;
  Matrix<BaseFloat> stats_;
  Vector<double> norm_;
  int32 features_cached_;
};  // class OnlineCMN

}  // namespace kaldi

#endif // KALDI_ONLINE_CMN_H_
