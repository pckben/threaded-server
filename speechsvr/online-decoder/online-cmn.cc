#include "online-cmn.h"

using namespace kaldi;

OnlineCMN::OnlineCMN(int32 dim, int32 history) {
  dim_ = dim;
  hist_ = history;
  oldest_row_ = 0;

  stats_.Resize(history, dim);
  norm_.Resize(0);
  features_cached_ = 0;
}

void OnlineCMN::ApplyCmvn(const MatrixBase<BaseFloat> &feats,
    Matrix<BaseFloat> *norm_feats) {

  KALDI_ASSERT(feats.NumCols() == dim_ &&
      "Inconsistent feature vectors dimensionality");

  norm_feats->Resize(0, 0);
  //first, fill up the history
  int32 i = 0;
  while (features_cached_ < hist_) {
    if (i < feats.NumRows()) {
      stats_.CopyRowFromVec(feats.Row(i), features_cached_);
      features_cached_++;
      i++;
    } else
      break;
  }

  //could we fill the history?
  if (features_cached_ != hist_) {
    //well, let's just compute the mean based on what we have so far ...
    ComputeMean(feats, norm_feats);
    return;
  }

  //now that we have our cache of features filled, we need to compute
  //the full sum only once
  if (norm_.Dim() == 0) {
    norm_.Resize(dim_);

    Vector<BaseFloat> sum;
    sum.Resize(dim_);
    for(int32 r = 0; r < hist_; r++) {
      for(int32 d = 0; d < dim_; d++) {
        sum(d) += stats_(r, d);
      }
    }

    for(int32 d = 0; d < dim_; d++) {
      norm_(d) = sum(d) / hist_;
    }
  }

  //compute the rolling mean
  for( ; i < feats.NumRows(); i++) {

    //update rolling mean
    for(int32 d = 0; d < dim_; d++) {
      norm_(d) = norm_(d) - stats_(oldest_row_, d) / hist_ +
        feats(i,d) / hist_;
    }

    //replace the oldest row with the current feature vector
    stats_.CopyRowFromVec(feats.Row(i), oldest_row_);
    //update the oldestRow_ variable
    oldest_row_ = (oldest_row_ + 1) % hist_;
  }

  //ok,  now we are ready to spit out the mean normalized features
  norm_feats->Resize(feats.NumRows(), dim_);

  for(int32 j = 0; j < feats.NumRows(); j++) {
    //apply mean
    for(int32 d = 0; d < dim_; d++) {
      (*norm_feats)(j, d) = feats(j, d) - norm_(d);
    }
  }
}

void OnlineCMN::ApplyCmvn2(const MatrixBase<BaseFloat> &feats,
    Matrix<BaseFloat> *norm_feats) {
  KALDI_ASSERT(feats.NumCols() == dim_ &&
      "Inconsistent feature vectors dimensionality");

  // norm_feats->Resize(0, 0);
  //first, fill up the history
  int32 i = 0;
  while (features_cached_ < hist_) {
    if (i < feats.NumRows()) {
      stats_.CopyRowFromVec(feats.Row(i), features_cached_);
      features_cached_++;
      i++;
    } else {
      break;
    }
  }
  // update the cached feature buffer
  for(; i < feats.NumRows();  ++ i) {
    //replace the oldest row with the current feature vector
    stats_.CopyRowFromVec(feats.Row(i), oldest_row_);
    //update the oldestRow_ variable
    oldest_row_ = (oldest_row_ + 1) % hist_;
  }
  Compute(feats,norm_feats);
}

void OnlineCMN::ComputeMean(const MatrixBase<BaseFloat> &feats,
    Matrix<BaseFloat> *norm_feats) {

  Vector<BaseFloat> sum;
  Vector<BaseFloat> norm;
  sum.Resize(dim_);
  norm.Resize(dim_);
  for(int32 r = 0; r < features_cached_; r++) {
    for(int32 d = 0; d < dim_; d++) {
      sum(d) += stats_(r, d);
    }
  }

  for(int32 d = 0; d < dim_; d++) {
    norm(d) = sum(d) / features_cached_;
  }

  norm_feats->Resize(feats.NumRows(), dim_);
  for(int32 i = 0; i < feats.NumRows(); i++) {
    for(int32 d = 0; d < dim_; d++) {
      (*norm_feats)(i, d) = feats(i, d) - norm(d);
    }
  }
}

void OnlineCMN::Compute(const MatrixBase<BaseFloat> &feats, Matrix<BaseFloat> *norm_feats)
{
  Vector<BaseFloat> m, var;
  m.Resize(dim_);
  var.Resize(dim_);
  for(int32 rIdx = 0; rIdx < features_cached_; ++ rIdx) {
    Vector<BaseFloat> feature(stats_.Row(rIdx));
    m.AddVec(1.0,feature);
    var.AddVec2(1.0,feature);
  }
  BaseFloat scale = 1.0f/features_cached_;
  m.Scale(scale);  
  var.Scale(scale); 
  var.AddVec2(-1.0,m);
  var.InvertElements();
  var.ApplyPow(0.5);

  norm_feats->Resize(feats.NumRows(), dim_);
  for(int32 rIdx = 0; rIdx < feats.NumRows(); ++ rIdx) {
    norm_feats->Row(rIdx).CopyRowFromMat(feats,rIdx);
    norm_feats->Row(rIdx).AddVec(-1.0,m);
    norm_feats->Row(rIdx).MulElements(var);
  }
}
