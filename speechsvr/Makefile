include ../make.mk
OBJ = Config.o DecodeWorker.o SpeechServer.o
OUT = speechsvr speechclient

LIBS = ../threaded-server.a \
			 $(KALDIROOT)/lat/kaldi-lat.a \
			 $(KALDIROOT)/decoder/kaldi-decoder.a \
			 $(KALDIROOT)/feat/kaldi-feature.a \
			 $(KALDIROOT)/transform/kaldi-transform.a \
			 $(KALDIROOT)/gmm/kaldi-gmm.a \
			 $(KALDIROOT)/hmm/kaldi-hmm.a \
			 $(KALDIROOT)/tree/kaldi-tree.a \
			 $(KALDIROOT)/matrix/kaldi-matrix.a \
			 $(KALDIROOT)/util/kaldi-util.a \
			 $(KALDIROOT)/base/kaldi-base.a \
			 $(KALDIROOT)/online/kaldi-online.a \
			 $(FSTROOT)/lib/libfst.a \
			 -framework Accelerate \
			 -lm -ldl

CCFLAGS += -I../src \
					 -DKALDI_DOUBLEPRECISION=0 -DHAVE_POSIX_MEMALIGN \
					 -Wno-sign-compare -Winit-self \
					 -DHAVE_EXECINFO_H=1 -DHAVE_CXXABI_H -rdynamic \
					 -DHAVE_CLAPACK \
					 -I$(KALDIROOT) \
					 -I$(FSTROOT)/include \

.PHONY: all test

all: test speechsvr speechclient

speechsvr: $(OBJ) speechsvr.o
	$(CC) $(CCFLAGS) -o $@ $(OBJ) speechsvr.o $(LIBS)

speechclient: $(OBJ) speechclient.o
	$(CC) $(CCFLAGS) -o $@ $(OBJ) speechclient.o $(LIBS)

%.o: %.cc
	$(CC) $(CCFLAGS) -c $< -o $@

test:
	$(MAKE) -C $@

clean:
	-rm -f $(OBJ) $(OUT) speechsvr.o speechclient.o
	$(MAKE) -C test clean
