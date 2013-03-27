#ifndef SPEECHSVR_REMOTEAUDIOSOURCE_H_
#define SPEECHSVR_REMOTEAUDIOSOURCE_H_

#include "online-decoder/online-audio-source-interface.h"
#include "matrix/kaldi-matrix.h"

namespace pckben {
  class Socket;
}

namespace speechsvr {
  class RemoteAudioSource : public kaldi::OnlineAudioSource {
   public:
    RemoteAudioSource(pckben::Socket *socket);
    kaldi::int32 Read(kaldi::VectorBase<kaldi::BaseFloat> *data,
                      kaldi::uint32 *timeout=0);

   private:
    pckben::Socket* socket_;
  };
}
#endif  // SPEECHSVR_REMOTEAUDIOSOURCE_H_
