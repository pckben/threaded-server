#ifndef KALDI_ONLINE_AUDIO_SOURCE_H_
#define KALDI_ONLINE_AUDIO_SOURCE_H_

#include "matrix/kaldi-vector.h"

namespace kaldi {

  // There is in fact no real hierarchy of classes w/ virtual methods etc.,
  // as C++ templates are used instead. The class below is given just
  // to document the interface.
  class OnlineAudioSource {
   public:
    // Reads from the audio source, and writes the samples converted to BaseFloat
    // into the vector pointed by "data". The function assumes, that "data" already
    // has the right size - i.e. its length is equal to the count of samples
    // requested. The original contents will be overwritten.
    // The function blocks until data->Dim() samples are read, unless
    // no more data is available for some reason(EOF?), or "*timeout" (in ms)
    // expires. In each case the function returns the number of samples actually
    // read. If the returned number is less than data->Dim(), the contents of the
    // remainder of the vector are undefined. If timer expires "data" contains
    // the samples read until timeout occured and *timeout contains zero.
    // In case timeout is not reached the contents of "*timeout" are left unchanged.
    // The timeout is considered to be a hint only and one should not rely on it
    // to be completely accurate. The "timeout" is not considered if this pointer
    // is zero and the function can block for indefinitely long period.
    // In case an unexpected and unrecoverable error occurs the function throws
    // an exception of type std::runtime_error (e.g. by using KALDI_ERR macro).
    virtual int32 Read(VectorBase<BaseFloat> *data, uint32 *timeout = 0) = 0;
    virtual ~OnlineAudioSource() { }
  };

}

#endif // KALDI_ONLINE_AUDIO_SOURCE_H_
