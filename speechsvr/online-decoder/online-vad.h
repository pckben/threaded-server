/*
 * online-vad.h
 *
 *  Created on: Oct 19, 2012
 *      Author: Ben
 */

#ifndef ONLINE_VAD_H_
#define ONLINE_VAD_H_

#include "feat/feature-extractor.h"
#include "online-feat-input.h"

namespace kaldi {

	/**
	 * Represents an object that listens for VAD events, such
	 * as speech starting and ending.
	 */
	class VadEventListener {
		public:
			virtual void OnSpeechStart() = 0;
			virtual void OnSpeechEnd() = 0;
			virtual ~VadEventListener() { }
	};

	/**
	 * Interface for Online VAD objects
	 */
	class OnlineVad {
		public:
			/**
			 * Initializes the VAD with the given non-speech sample segment
			 */
			virtual void Initialize(VectorBase<BaseFloat> &nonSpeechSample) = 0;
			/**
			 * Processes the given frame and returns VAD detection value
			 */
			virtual bool Process(VectorBase<BaseFloat> &frame) = 0;
			virtual ~OnlineVad() { }
	};

	class SimpleEnergyVad : public OnlineVad {
		public:
			enum VadState {
					kSilence = 1,
					kEndingSpeech = 2,
					kSpeech = 4
				};

			SimpleEnergyVad(float onset_threshold, float offset_threshold,
                      float recover_threshold, int hangover_frames);
			~SimpleEnergyVad() { fclose(vad_file); }
			void Initialize(VectorBase<BaseFloat> &nonSpeechSample);
			bool Process(VectorBase<BaseFloat> &frame);
		private:
			float onset_threshold_;
			float offset_threshold_;
			float recover_threshold_;
			bool lastVad_;
			FILE* vad_file;
			int hangover_counter_;
			int hangover_frames_;
			VadState state_;
	};

	class Ramirez2004Vad : public OnlineVad {
		public:
			void Initialize(VectorBase<BaseFloat> &nonSpeechSample);
			bool Process(VectorBase<BaseFloat> &frame);
	};

	/**
	 * Wraps an audio source and queue up N frames
	 * then performs VAD on the big buffer.
	 */
	class OnlineVadFeInput : public OnlineFeatInputItf {
		public:
			OnlineVadFeInput(OnlineAudioSource *au_src,
					FeatureExtractor* extractor,
					OnlineVad* vad,
					int frame_size,
					int frame_shift,
					int buffer_length);

			virtual ~OnlineVadFeInput();

			void AddVadListener(VadEventListener* listener) { vadListener_[nVadListeners_++] = listener; }

			virtual bool Compute(Matrix<BaseFloat> *output, uint32 *timeout);

			bool DoVad(VectorBase<BaseFloat> &batch);

		private:
			void ShiftBuffer(VectorBase<BaseFloat> &buffer, int overlap);

			int frame_size_;
			int frame_shift_;
			OnlineAudioSource *source_;	// audio source
			FeatureExtractor *extractor_;
			OnlineVad *vad_;
			int buffer_length_;	// in #samples
			bool buffer_init_; // so we can wait to fill up the buffer
			Vector<BaseFloat> buffer_; // the longer buffer, acts as a queue
			bool batchVad_;
			int hangoverCount_;

			VadEventListener* vadListener_[128];
			int nVadListeners_;

			// DEBUG
			FILE* vad_input_wav;
			FILE* vad_output_wav;

			int cursorPos;
			std::string cursorChars;

			KALDI_DISALLOW_COPY_AND_ASSIGN(OnlineVadFeInput);
	};

	class OnlineFasterDecoder;

	/**
	 * Bridges OnlineFasterDecoder and VadListener
	 */
	class OnlineFasterDecoderVadListener : public VadEventListener {
		public:
			OnlineFasterDecoderVadListener(OnlineFasterDecoder* decoder);
			void OnSpeechStart();
			void OnSpeechEnd();
		private:
			OnlineFasterDecoder* decoder_;
	};
}

#endif /* ONLINE_VAD_H_ */
