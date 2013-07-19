// online/online-faster-decoder.cc

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

#include "util/timer.h"
#include "online-faster-decoder.h"
#include "fstext/fstext-utils.h"
#include "hmm/hmm-utils.h"

namespace kaldi {

void OnlineFasterDecoder::ResetDecoder(bool full) {
  ClearToks(toks_.Clear());
  StateId start_state = fst_.Start();
  KALDI_ASSERT(start_state != fst::kNoStateId);
  Arc dummy_arc(0, 0, Weight::One(), start_state);
  Token *dummy_token = new Token(dummy_arc, NULL);
  toks_.Insert(start_state, dummy_token);
  prev_immortal_tok_ = immortal_tok_ = dummy_token;
  utt_frames_ = 0;
  if (full)
    frame_ = 0;
}


void
OnlineFasterDecoder::MakeLattice(const Token *start,
                                 const Token *end,
                                 fst::MutableFst<LatticeArc> *out_fst) const {
  out_fst->DeleteStates();
  if (start == NULL) return;
  bool is_final = false;
  Weight this_weight = Times(start->weight_,
                             fst_.Final(start->arc_.nextstate));
  if (this_weight != Weight::Zero())
    is_final = true;
  std::vector<LatticeArc> arcs_reverse;  // arcs in reverse order.
  for (const Token *tok = start; tok != end; tok = tok->prev_) {
    BaseFloat tot_cost = tok->weight_.Value() -
        (tok->prev_ ? tok->prev_->weight_.Value() : 0.0),
        graph_cost = tok->arc_.weight.Value(),
        ac_cost = tot_cost - graph_cost;
    LatticeArc l_arc(tok->arc_.ilabel,
                     tok->arc_.olabel,
                     LatticeWeight(graph_cost, ac_cost),
                     tok->arc_.nextstate);
    arcs_reverse.push_back(l_arc);
  }
  if(arcs_reverse.back().nextstate == fst_.Start()) {
    arcs_reverse.pop_back();  // that was a "fake" token... gives no info.
  }
  StateId cur_state = out_fst->AddState();
  out_fst->SetStart(cur_state);
  for (ssize_t i = static_cast<ssize_t>(arcs_reverse.size())-1; i >= 0; i--) {
    LatticeArc arc = arcs_reverse[i];
    arc.nextstate = out_fst->AddState();
    out_fst->AddArc(cur_state, arc);
    cur_state = arc.nextstate;
  }
  if (is_final) {
    Weight final_weight = fst_.Final(start->arc_.nextstate);
    out_fst->SetFinal(cur_state, LatticeWeight(final_weight.Value(), 0.0));
  } else {
    out_fst->SetFinal(cur_state, LatticeWeight::One());
  }
  RemoveEpsLocal(out_fst);
}


void OnlineFasterDecoder::UpdateImmortalToken() {
  unordered_set<Token*> emitting;
  for (Elem *e = toks_.GetList(); e != NULL; e = e->tail) {
    Token* tok = e->val;
    while (tok->arc_.ilabel == 0) //deal with non-emitting ones ...
      tok = tok->prev_;
    if (tok != NULL)
      emitting.insert(tok);
  }
  Token* the_one = NULL;
  while (1) {
    if (emitting.size() == 1) {
      the_one = *(emitting.begin());
      break;
    }
    if (emitting.size() == 0)
      break;
    unordered_set<Token*> prev_emitting;
    unordered_set<Token*>::iterator it;
    for (it = emitting.begin(); it != emitting.end(); ++it) {
      Token* tok = *it;
      Token* prev_token = tok->prev_;
      while ((prev_token != NULL) && (prev_token->arc_.ilabel == 0))
        prev_token = prev_token->prev_; //deal with non-emitting ones
      if (prev_token == NULL)
        continue;
      prev_emitting.insert(prev_token);
    } // for
    emitting = prev_emitting;
  } // while
  if (the_one != NULL) {
    prev_immortal_tok_ = immortal_tok_;
    immortal_tok_ = the_one;
    return;
  }
}


bool
OnlineFasterDecoder::PartialTraceback(fst::MutableFst<LatticeArc> *out_fst) {
  UpdateImmortalToken();
  if(immortal_tok_ == prev_immortal_tok_)
    return false; //no partial traceback at that point of time
  MakeLattice(immortal_tok_, prev_immortal_tok_, out_fst);
  return true;
}


void
OnlineFasterDecoder::FinishTraceBack(fst::MutableFst<LatticeArc> *out_fst) {
  Token *best_tok = NULL;
  bool is_final = ReachedFinal();
  if (!is_final) {
    for (Elem *e = toks_.GetList(); e != NULL; e = e->tail)
      if (best_tok == NULL || *best_tok < *(e->val) )
        best_tok = e->val;
  } else {
    Weight best_weight = Weight::Zero();
    for (Elem *e = toks_.GetList(); e != NULL; e = e->tail) {
      Weight this_weight = Times(e->val->weight_, fst_.Final(e->key));
      if (this_weight != Weight::Zero() &&
          this_weight.Value() < best_weight.Value()) {
        best_weight = this_weight;
        best_tok = e->val;
      }
    }
  }
  MakeLattice(best_tok, immortal_tok_, out_fst);
}


void
OnlineFasterDecoder::TracebackNFrames(int32 nframes,
                                      fst::MutableFst<LatticeArc> *out_fst) {
  Token *best_tok = NULL;
  for (Elem *e = toks_.GetList(); e != NULL; e = e->tail)
    if (best_tok == NULL || *best_tok < *(e->val) )
      best_tok = e->val;
  if (best_tok == NULL) {
    out_fst->DeleteStates();
    return;
  }

  bool is_final = false;
  Weight this_weight = Times(best_tok->weight_,
                             fst_.Final(best_tok->arc_.nextstate));
  if (this_weight != Weight::Zero())
    is_final = true;
  std::vector<LatticeArc> arcs_reverse;  // arcs in reverse order.
  for (Token *tok = best_tok; (tok != NULL) && (nframes > 0); tok = tok->prev_) {
    if (tok->arc_.ilabel != 0) // count only the non-epsilon arcs
      --nframes;
    BaseFloat tot_cost = tok->weight_.Value() -
                             (tok->prev_ ? tok->prev_->weight_.Value() : 0.0);
    BaseFloat graph_cost = tok->arc_.weight.Value();
    BaseFloat ac_cost = tot_cost - graph_cost;
    LatticeArc larc(tok->arc_.ilabel,
                     tok->arc_.olabel,
                     LatticeWeight(graph_cost, ac_cost),
                     tok->arc_.nextstate);
    arcs_reverse.push_back(larc);
  }
  if(arcs_reverse.back().nextstate == fst_.Start())
    arcs_reverse.pop_back();  // that was a "fake" token... gives no info.
  StateId cur_state = out_fst->AddState();
  out_fst->SetStart(cur_state);
  for (ssize_t i = static_cast<ssize_t>(arcs_reverse.size())-1; i >= 0; i--) {
    LatticeArc arc = arcs_reverse[i];
    arc.nextstate = out_fst->AddState();
    out_fst->AddArc(cur_state, arc);
    cur_state = arc.nextstate;
  }
  if (is_final) {
    Weight final_weight = fst_.Final(best_tok->arc_.nextstate);
    out_fst->SetFinal(cur_state, LatticeWeight(final_weight.Value(), 0.0));
  } else {
    out_fst->SetFinal(cur_state, LatticeWeight::One());
  }
  RemoveEpsLocal(out_fst);
}


// IsReordered returns true if the transitions were possibly reordered.  This reordering
// can happen in AddSelfLoops, if the "reorder" option was true.
// This makes the out-transition occur before the self-loop transition.
// The function returns false (no reordering) if there is not enough information in
// the alignment to tell (i.e. no self-loop were taken), and in this case the calling
// code doesn't care what the answer is.
// The "alignment" vector contains a sequence of TransitionIds.


static bool IsReordered(const TransitionModel &trans_model,
                        const std::vector<int32> &alignment) {
  for (size_t i = 0; i+1 < alignment.size(); i++) {
    int32 tstate1 = trans_model.TransitionIdToTransitionState(alignment[i]),
        tstate2 = trans_model.TransitionIdToTransitionState(alignment[i+1]);
    if (tstate1 != tstate2) {
      bool is_loop_1 = trans_model.IsSelfLoop(alignment[i]),
          is_loop_2 = trans_model.IsSelfLoop(alignment[i+1]);
      KALDI_ASSERT(!(is_loop_1 && is_loop_2));  // Invalid.
      if (is_loop_1) return true;  // Reordered. self-loop is last.
      if (is_loop_2) return false;  // Not reordered.  self-loop is first.
    }
  }

  // Just one trans-state in whole sequence.
  if (alignment.empty()) return false;
  else {
    bool is_loop_front = trans_model.IsSelfLoop(alignment.front()),
        is_loop_back = trans_model.IsSelfLoop(alignment.back());
    if (is_loop_front) return false;  // Not reordered.  Self-loop is first.
    if (is_loop_back) return true;  // Reordered.  Self-loop is last.
    return false;  // We really don't know in this case but calling code should
    // not care.
  }
}


// SplitToPhonesInternal takes as input the "alignment" vector containing
// a sequence of transition-ids, and appends a single vector to
// "split_output" for each instance of a phone that occurs in the
// output.
// Returns true if the alignment passes some non-exhaustive consistency
// checks (if the input does not start at the start of a phone or does not
// end at the end of a phone, we should expect that false will be returned).

static bool SplitToPhonesInternal(const TransitionModel &trans_model,
                                  const std::vector<int32> &alignment,
                                  bool reordered,
                                  std::vector<std::vector<int32> > *split_output) {
  if (alignment.empty()) return true;  // nothing to split.
  std::vector<size_t> end_points;  // points at which phones end [in an
  // stl iterator sense, i.e. actually one past the last transition-id within
  // each phone]..

  bool was_ok = true;
  for (size_t i = 0; i < alignment.size(); i++) {
    int32 trans_id = alignment[i];
    if (trans_model.IsFinal(trans_id)) {  // is final-prob
      if (!reordered) end_points.push_back(i+1);
      else {  // reordered.
        while (i+1 < alignment.size() &&
              trans_model.IsSelfLoop(alignment[i+1])) {
          assert(trans_model.TransitionIdToTransitionState(alignment[i]) == 
                 trans_model.TransitionIdToTransitionState(alignment[i+1]));
          i++;
        }
        end_points.push_back(i+1);
      }
    } else if (i+1 == alignment.size()) {
      // need to have an end-point at the actual end.
      // but this is an error- should have been detected already.
      was_ok = false;
      end_points.push_back(i+1);
    } else {
      int32 this_state = trans_model.TransitionIdToTransitionState(alignment[i]),
          next_state = trans_model.TransitionIdToTransitionState(alignment[i+1]);
      if (this_state == next_state) continue;  // optimization.
      int32 this_phone = trans_model.TransitionStateToPhone(this_state),
          next_phone = trans_model.TransitionStateToPhone(next_state);
      if (this_phone != next_phone) {
        // The phone changed, but this is an error-- we should have detected this via the
        // IsFinal check.
        was_ok = false;
        end_points.push_back(i+1);
      }
    }
  }

  size_t cur_point = 0;
  for (size_t i = 0; i < end_points.size(); i++) {
    split_output->push_back(std::vector<int32>());
    // The next if-statement just checks that the initial trans-id
    // of the alignment is an initial-state of a phone (a cursory check
    // that the alignment is plausible).
    if (trans_model.TransitionStateToHmmState
       (trans_model.TransitionIdToTransitionState
        (alignment[cur_point])) != 0) was_ok= false;
    for (size_t j = cur_point; j < end_points[i]; j++)
      split_output->back().push_back(alignment[j]);
    cur_point = end_points[i];
  }
  return was_ok;
}


bool SplitToPhones(const TransitionModel &trans_model,
                   const std::vector<int32> &alignment,
                   std::vector<std::vector<int32> > *split_alignment) {
  assert(split_alignment != NULL);
  split_alignment->clear();

  bool is_reordered = IsReordered(trans_model, alignment);
  return SplitToPhonesInternal(trans_model, alignment,
                               is_reordered, split_alignment);
}


bool OnlineFasterDecoder::EndOfUtterance() {
  fst::VectorFst<LatticeArc> trace;
  int32 sil_frm = opts_.inter_utt_sil / (1 + utt_frames_ / opts_.max_utt_len_);
  TracebackNFrames(sil_frm, &trace);
  std::vector<int32> isymbols;
  fst::GetLinearSymbolSequence(trace, &isymbols,
                               static_cast<std::vector<int32>* >(0),
                               static_cast<LatticeArc::Weight*>(0));
  std::vector<std::vector<int32> > split;
  SplitToPhones(trans_model_, isymbols, &split);
  for (size_t i = 0; i < split.size(); i++) {
    int32 tid = split[i][0];
    int32 phone = trans_model_.TransitionIdToPhone(tid);
    if (silence_set_.count(phone) == 0)
      return false;
  }
  return true;
}

void OnlineFasterDecoder::ForceEndUtterance() {
	forcingEndUtt_ = true;
}

OnlineFasterDecoder::DecodeState
OnlineFasterDecoder::Decode(DecodableInterface *decodable) {
  if (state_ == kEndFeats || state_ == kEndUtt) // new utterance
    ResetDecoder(state_ == kEndFeats);
  ProcessNonemitting(std::numeric_limits<float>::max());
  int32 batch_frame = 0;
  Timer timer;
  double64 tstart = timer.Elapsed(), tstart_batch = tstart;
  BaseFloat factor = -1;
  for (; !decodable->IsLastFrame(frame_ - 1) && batch_frame < opts_.batch_size;
       ++frame_, ++utt_frames_, ++batch_frame) {
    if (batch_frame != 0 && (batch_frame % opts_.update_interval) == 0) {
      // adjust the beam if needed
      BaseFloat tend = timer.Elapsed();
      BaseFloat elapsed = (tend - tstart) * 1000;
      // warning: hardcoded 10ms frames assumption!
      factor = elapsed / (opts_.rt_max * opts_.update_interval * 10);
      BaseFloat min_factor = (opts_.rt_min / opts_.rt_max);
      if (factor > 1 || factor < min_factor) {
        BaseFloat update_factor = (factor > 1)?
            -std::min(opts_.beam_update * factor, opts_.max_beam_update):
             std::min(opts_.beam_update / factor, opts_.max_beam_update);
        effective_beam_ += effective_beam_ * update_factor;
        effective_beam_ = std::min(effective_beam_, max_beam_);
      }
      tstart = tend;
    }
    if (batch_frame != 0 && (frame_ % 200) == 0)
      // one log message at every 2 seconds assuming 10ms frames
      KALDI_VLOG(3) << "Beam: " << effective_beam_
          << "; Speed: "
          << ((timer.Elapsed() - tstart_batch) * 1000) / (batch_frame*10)
          << " xRT";
    BaseFloat weight_cutoff = ProcessEmitting(decodable, frame_);
    ProcessNonemitting(weight_cutoff);
    if (forcingEndUtt_) {
    	state_ = kEndUtt;
    	forcingEndUtt_ = false;
    	return state_;
    }
  }
  if (batch_frame == opts_.batch_size && !decodable->IsLastFrame(frame_ - 1)) {
//    if (EndOfUtterance())
//      state_ = kEndUtt;
//    else
	  state_ = kEndBatch;
  } else {
    state_ = kEndFeats;
  }
  return state_;
}

} // namespace kaldi
