// online/online-feat-input.h

// Copyright 2012 Cisco Systems (author: Matthias Paulik)

//   Modifications to the original contribution by Cisco Systems made by:
//   Vassil Panayotov

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

#ifndef KALDI_ONLINE_FEAT_EXTRACT_H_
#define KALDI_ONLINE_FEAT_EXTRACT_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "feat/feature-functions.h"
#include "feat/feature-extractor.h"
#include "online-cmn.h"
#include "online-audio-source-interface.h"

namespace kaldi {

// Interface specification
class OnlineFeatInputItf {
 public:
  // Produces feature vectors in some way.
  // The features may be e.g. extracted from an audio samples, received and/or
  // transformed from another OnlineFeatInput class etc.
  //
  // "output" - a matrix to store the extracted feature vectors in its rows.
  //            The function will block until all rows of the output matrix are
  //            overwritten by new feature vectors, unless there is no more
  //            data in the underlying audio source or the timeout(if used)
  //            expires. The actual count of the computed vectors can be
  //            obtained by calling output->NumRows()
  //
  // "timeout" - points to a variable which contains a timeout(in ms).
  //             If the timeout expires, the referenced variable will
  //             contain 0 and the function can return fewer than the requested
  //             vectors. If the pointer is NULL, then no timeout is used and
  //             the function can block for indefinite time. This parameter
  //             should be considered only a hint and the user shouldn't assume
  //             the timeout will be fired with millisecond precision.
  //
  // Returns "false" if the underlying data source has no more data, and true
  // otherwise.
  virtual bool Compute(Matrix<BaseFloat> *output, uint32 *timeout) = 0;

  virtual ~OnlineFeatInputItf() {}
};


// Acts as a proxy to an underlying OnlineFeatInput.
// Applies cepstral mean and variance normalization
class OnlineCmvnInput : public OnlineFeatInputItf {
 public:
  // "input" - the underlying(unnormalized) feature source
  // "feat_dim" - dimensionality of the vectors
  // "cmn_window" - the count of the last vectors over which the average is
  //                calculated
  OnlineCmvnInput(OnlineFeatInputItf *input, int32 feat_dim, int32 cmn_window)
      : input_(input), cmvn_(feat_dim, cmn_window), in_matrix_() {}

  virtual bool Compute(Matrix<BaseFloat> *output, uint32 *timeout);

 private:
  OnlineFeatInputItf *input_;
  OnlineCMN cmvn_;
  Matrix<BaseFloat> in_matrix_; // the data received from the wrapped object

  KALDI_DISALLOW_COPY_AND_ASSIGN(OnlineCmvnInput);
};


// Accepts features over an UDP socket
class OnlineUdpInput : public OnlineFeatInputItf {
 public:
  OnlineUdpInput(int32 port);

  // The current implementation doesn't support "timeout" parameter
  virtual bool Compute(Matrix<BaseFloat> *output, uint32 *timeout);

  const sockaddr_in& client_addr() const { return client_addr_; }

  const int32 descriptor() const { return sock_desc_; }

 private:
  // various BSD sockets-related data structures
  int32 sock_desc_; // socket descriptor
  sockaddr_in server_addr_;
  sockaddr_in client_addr_;
};


// Splices the input features and applies a transformation matrix
class OnlineLdaInput : public OnlineFeatInputItf {
 public:
  OnlineLdaInput(OnlineFeatInputItf *input, const uint32 feat_dim,
                 const Matrix<BaseFloat> &transform,
                 const uint32 left_context, const uint32 right_context);

  virtual bool Compute(Matrix<BaseFloat> *output, uint32 *timeout);

 private:
  void InitFeatWindow();

  OnlineFeatInputItf *input_; // underlying/inferior input object
  const uint32 feat_dim_; // dimensionality of the feature vectors before xform
  const Matrix<BaseFloat> transform_; // transform matrix
  const uint32 window_size_; // the count of the feature vectors to be xformed
  const uint32 window_center_; // central feature vector offset
  uint32 window_pos_; // the position of the first vector in the current window
  const uint32 trans_rows_; // xform matrix rows == output vectors dimension
  Matrix<BaseFloat> feat_in_; // made a member in hope it will save us some memalloc time
  Matrix<BaseFloat> feat_window_; // matrix to hold features to be transformed
  Matrix<BaseFloat> spliced_feats_; // spliced features
  KALDI_DISALLOW_COPY_AND_ASSIGN(OnlineLdaInput);
}; // OnlineLdaInput


class OnlineDeltaInput : public OnlineFeatInputItf {
 public:
  OnlineDeltaInput(OnlineFeatInputItf *input, uint32 feat_dim,
                   uint32 order, uint32 window);

  bool Compute(Matrix<BaseFloat> *output, uint32 *timeout);

 private:
  void InitFeatWindow();

  OnlineFeatInputItf *input_; // underlying/inferior input object
  const uint32 feat_dim_; // feature vector dimensionality before transform
  const uint32 order_; // delta order
  const uint32 window_size_; // the number of features needed to compute deltas
  const uint32 window_center_; // index of the central feature (for convenience)
  Matrix<BaseFloat> feat_window_; // features needed to compute deltas
  Matrix<BaseFloat> feat_in_; // feature received from inferior object
  DeltaFeatures delta_; // computes deltas
  KALDI_DISALLOW_COPY_AND_ASSIGN(OnlineDeltaInput);
};


// Reads samples from the given audio source and write to
// the given output buffer
class OnlineAudioInput : public OnlineFeatInputItf {
 public:
  OnlineAudioInput(OnlineAudioSource *au_src, const int32 frame_size);

  virtual bool Compute(Matrix<BaseFloat> *output, uint32 *timeout);

 private:
  OnlineAudioSource *source_;
  const int32 frame_size_;
  Vector<BaseFloat> wave_;

  KALDI_DISALLOW_COPY_AND_ASSIGN(OnlineAudioInput);
};



// Implementation, that is meant to be used to read samples from an
// OnlineAudioSource and to extract MFCC/PLP features in the usual way
class OnlineFeInput : public OnlineFeatInputItf {
 public:
  // "au_src" - OnlineAudioSource object
  // "fe" - MFCC/PLP feature extraction class
  // "frame_size" - frame extraction window size in audio samples
  // "frame_shift" - feature frame width in audio samples
  OnlineFeInput(OnlineAudioSource *au_src, FeatureExtractor *fe,
                const int32 frame_size, const int32 frame_shift);

  virtual bool Compute(Matrix<BaseFloat> *output, uint32 *timeout);

 private:
  OnlineAudioSource *source_; // audio source
  FeatureExtractor *extractor_; // the actual feature extractor used
  const int32 frame_size_;
  const int32 frame_shift_;
  Vector<BaseFloat> wave_; // the samples to be passed for extraction
  Vector<BaseFloat> wave_remainder_; // the samples remained from the previous
                                     // feature batch

  KALDI_DISALLOW_COPY_AND_ASSIGN(OnlineFeInput);
};


} // namespace kaldi

#endif // KALDI_ONLINE_FEAT_EXTRACT_H_
