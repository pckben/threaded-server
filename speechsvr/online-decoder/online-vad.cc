/*
 * online-vad.cc
 *
 *  Created on: Oct 19, 2012
 *      Author: Ben
 */

#include "online-vad.h"
#include "feat/feature-functions.h"
#include "online-audio-source.h"
#include "online-faster-decoder.h"
#include <iostream>

using namespace std;

namespace kaldi {

SimpleEnergyVad::SimpleEnergyVad(float onset_threshold, float offset_threshold,
                                float recover_threshold, int hangover_frames)
	: onset_threshold_(onset_threshold), offset_threshold_(offset_threshold),
    recover_threshold_(recover_threshold), hangover_counter_(0), 
    hangover_frames_(hangover_frames), state_(kSilence)
{}

void
SimpleEnergyVad::Initialize(VectorBase<BaseFloat> &sample) {
	FrameExtractionOptions frame_opts;
	frame_opts.window_type = "rectangular";
	FeatureWindowFunction feature_window_function_(frame_opts);
	Vector<BaseFloat> window;  // windowed waveform.
//	int32 num_frames = NumFrames(sample.Dim(), frame_opts);
	float avg_energy = 0;
	float max_energy = 0;
	for (int i=0; i<5; i++) {
		ExtractWindow(sample, i, frame_opts, feature_window_function_, &window, NULL);
		float energy = sqrt(VecVec(window, window)/window.Dim());
		avg_energy += energy;
		max_energy = max(max_energy, energy);
	}
	avg_energy /= 5;
	//onset_threshold_ = 400; //100; //400; // max_energy*1.3;
	//recover_threshold_ = 100;
	//offset_threshold_ = 50; //50; //200; // avg_energy*1.1;
	lastVad_ = false;
	vad_file = fopen("vad_file", "w");
	cerr << "SimpleEnergyVad initialization: \n\ton-set threshold=" << onset_threshold_
			<< "\n\toff-set threshold=" << offset_threshold_
			<< "\n\trecover threshold=" << recover_threshold_
			<< "\n\thangover=" << hangover_frames_ << " frames" << endl;
}

bool
SimpleEnergyVad::Process(VectorBase<BaseFloat> &frame) {
    float energy = sqrt(VecVec(frame, frame)/frame.Dim());

    if (state_ == kSilence && energy > onset_threshold_) {
		state_ = kSpeech;
	}

	else if (state_ == kSpeech && energy < offset_threshold_) {
		state_ = kEndingSpeech;
		hangover_counter_ = 0;
	}

	else if (state_ == kEndingSpeech) {
		if (energy > recover_threshold_) {
			state_ = kSpeech;
		}
		else if (hangover_counter_++ > hangover_frames_) {
    		state_ = kSilence;
    	}
    }
	bool vad = state_ == kSpeech || state_ == kEndingSpeech;
    fprintf(vad_file, "%.4f %.4f %.4f %.4f %d\n", energy, onset_threshold_, offset_threshold_, recover_threshold_, state_*200);
	return vad;
}

void
Ramirez2004Vad::Initialize(VectorBase<BaseFloat> &sample) {
}

bool
Ramirez2004Vad::Process(VectorBase<BaseFloat> &frame) {
	return true;
}

OnlineVadFeInput::OnlineVadFeInput(OnlineAudioSource *au_src,
									FeatureExtractor* extractor,
									OnlineVad* vad,
									int frame_size,
									int frame_shift,
									int buffer_length)
	:
	  frame_size_(frame_size),
	  frame_shift_(frame_shift),
	  source_(au_src), extractor_(extractor), vad_(vad),
	  buffer_length_(buffer_length),
	  buffer_init_(false),
	  batchVad_(false),
	  hangoverCount_(0),
	  nVadListeners_(0)
{
    vad_input_wav = fopen("vad_input", "wb");
    vad_output_wav = fopen("vad_output", "wb");
    cursorChars = "\\|/-\\|/-";
    cursorPos = 0;
}

OnlineVadFeInput::~OnlineVadFeInput() {
    fclose(vad_input_wav);
    fclose(vad_output_wav);
}

bool
OnlineVadFeInput::DoVad(VectorBase<BaseFloat> &batch) {

	FrameExtractionOptions frame_opts;
	frame_opts.window_type = "rectangular";
	FeatureWindowFunction feature_window_function_(frame_opts);
	Vector<BaseFloat> window;  // windowed waveform.
	int32 num_frames = NumFrames(batch.Dim(), frame_opts);
	int32 num_speech_frames = 0;
	for (int i=0; i<num_frames; i++) {
		ExtractWindow(batch, i, frame_opts, feature_window_function_, &window, NULL);
		if (vad_->Process(window))
			num_speech_frames++;
	}

	return num_speech_frames/float(num_frames) > .3;
}

void OnlineVadFeInput::ShiftBuffer(VectorBase<BaseFloat> &buffer, int shift_size) {
	SubVector<BaseFloat> moving_from_back(buffer, shift_size, buffer.Dim()-shift_size);
	SubVector<BaseFloat> moving_to_front(buffer, 0, buffer.Dim()-shift_size);
	Vector<BaseFloat> temp(buffer.Dim()-shift_size);
	temp.CopyFromVec(moving_from_back);
	moving_to_front.CopyFromVec(temp);
}

bool
OnlineVadFeInput::Compute(Matrix<BaseFloat> *output, uint32 *timeout) {
	KALDI_VLOG(3) << "OnlineVadFeInput::Compute " << output->NumRows() << " output vectors";
	MatrixIndexT nvec = output->NumRows(); // the number of output vectors
	if (nvec <= 0) {
		KALDI_WARN << "No feature vectors requested?!";
		return true;
	}

	int32 batch_size = frame_size_ + (nvec - 1) * frame_shift_;
	int32 frame_overlap = frame_size_ - frame_shift_;
	// number of new samples that we actually need to push
	int32 samples_req = batch_size - frame_overlap;
	// subvector to be pushed the new samples in

	if (!buffer_init_) {
		buffer_length_ += batch_size;
		buffer_.Resize(buffer_length_, kUndefined);
		source_->Read(&buffer_);
		buffer_init_ = true;
		vad_->Initialize(buffer_);
		fwrite(buffer_.Data(), sizeof(BaseFloat), buffer_length_, vad_input_wav);
	}

	SubVector<BaseFloat> buffer_back(buffer_, buffer_length_ - samples_req, samples_req);
	SubVector<BaseFloat> buffer_front(buffer_, 0, batch_size);

	int32 samples_rcv = 0;
	bool lastBatchVad;

	do {
		lastBatchVad = batchVad_;
		// perform VAD on the new frames, but send out the front frames
		batchVad_ = DoVad(buffer_back);

		// notify state changing events to the listeners
		if (batchVad_ != lastBatchVad) {
			for (int i=0; i<nVadListeners_; i++)
				if (!lastBatchVad && batchVad_) {
					vadListener_[i]->OnSpeechStart();
					hangoverCount_ = 0;
				}
				else if (lastBatchVad && !batchVad_) {
					hangoverCount_ += samples_req;
					if (hangoverCount_ > buffer_length_) {
						vadListener_[i]->OnSpeechEnd();
					}
					else
						batchVad_ = true;
				}
		}

		// extract feature if required:
		if (lastBatchVad || batchVad_) {
			// only write samples_req for continuous audio wave, but perform feature
			// extraction on the whole batch size.
			fwrite(buffer_front.Data(), sizeof(BaseFloat), samples_req, vad_output_wav);
			// Extract features for the front frames
			extractor_->Compute(buffer_front, output);
			if (output->NumRows() != nvec)
				KALDI_VLOG(3) << nvec << " feature vectors were requested, but only "
							  << output->NumRows() << " were received!";

			std::cerr << cursorChars[cursorPos++ % cursorChars.length()] << "\b";
		} else {
			std::cerr << "-\b";
		}

		// DEBUG, write some beep
		if (lastBatchVad && !batchVad_ && hangoverCount_ > buffer_length_) {
			float delim[1024];
			for (int i=0; i<1024; i++) delim[i] = 1e4;
			fwrite(delim, sizeof(float), 1024, vad_output_wav);
		}

		ShiftBuffer(buffer_, samples_req); // drop the front batch

		// read a new batch from the audio source
		samples_rcv = source_->Read(&buffer_back); // read a new batch to the back
		fwrite(buffer_back.Data(), sizeof(BaseFloat), samples_rcv, vad_input_wav);

		if (timeout != 0 && *timeout == 0)
			KALDI_WARN << "InputAudioSource::Read() timeout expired!";
		else if (samples_rcv != samples_req)
			KALDI_VLOG(3) << samples_req << " samples were requested, but only "
						  << samples_rcv << " were received!";

	} while (!(lastBatchVad || batchVad_) && samples_rcv==samples_req); // blocked until speech

	// if all requested samples are obtained or a timeout was triggered,
	// then we assume there is more data in the audio source
	return (samples_rcv == samples_req) || (timeout != 0 && *timeout == 0);
}

OnlineFasterDecoderVadListener::OnlineFasterDecoderVadListener(OnlineFasterDecoder* decoder)
: decoder_(decoder) { }

void OnlineFasterDecoderVadListener::OnSpeechStart() {
//	std::cerr << "[";
}

void OnlineFasterDecoderVadListener::OnSpeechEnd() {
	decoder_->ForceEndUtterance();
//	std::cerr << "]";
}



}
