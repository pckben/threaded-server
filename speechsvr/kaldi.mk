CC = g++
CCFLAGS = -g -O2 -Wall \
					-Wno-sign-compare -Winit-self \

FSTROOT = /home/scespeech/kaldi/tools/openfst
KALDIROOT = /home/scespeech/kaldi/src
ATLASINC = /home/scespeech/kaldi/tools/ATLAS/include
ATLASLIBS = /home/scespeech/kaldi/src/../tools/ATLAS/build/install/lib//liblapack.a /home/scespeech/kaldi/src/../tools/ATLAS/build/install/lib//libcblas.a /home/scespeech/kaldi/src/../tools/ATLAS/build/install/lib//libatlas.a /home/scespeech/kaldi/src/../tools/ATLAS/build/install/lib//libf77blas.a

LIBS += $(KALDIROOT)/lat/kaldi-lat.a \
				$(KALDIROOT)/decoder/kaldi-decoder.a \
				$(KALDIROOT)/feat/kaldi-feature.a \
				$(KALDIROOT)/transform/kaldi-transform.a \
				$(KALDIROOT)/gmm/kaldi-gmm.a \
				$(KALDIROOT)/hmm/kaldi-hmm.a \
				$(KALDIROOT)/tree/kaldi-tree.a \
				$(KALDIROOT)/matrix/kaldi-matrix.a \
				$(KALDIROOT)/util/kaldi-util.a \
				$(KALDIROOT)/base/kaldi-base.a \
				$(FSTROOT)/lib/libfst.a \
				-lm -ldl -lpthread \
				$(ATLASLIBS) \
				#-framework Accelerate \

CCFLAGS += -DKALDI_DOUBLEPRECISION=0 -DHAVE_POSIX_MEMALIGN \
					 -DHAVE_EXECINFO_H=1 -DHAVE_CXXABI_H -rdynamic \
					 -DHAVE_ATLAS -I$(ATLASINC) \
					 -I$(KALDIROOT) \
					 -I$(FSTROOT)/include
