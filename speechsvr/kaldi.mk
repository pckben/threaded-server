CC = g++
CCFLAGS = -g -O2 -Wall \
					-Wno-sign-compare -Winit-self \

FSTROOT = /Users/Ben/projects/kaldi/tools/openfst
KALDIROOT = /Users/Ben/projects/kaldi/src

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
				-framework Accelerate \
				-lm -ldl

CCFLAGS += -DKALDI_DOUBLEPRECISION=0 -DHAVE_POSIX_MEMALIGN \
					 -DHAVE_EXECINFO_H=1 -DHAVE_CXXABI_H -rdynamic \
					 -DHAVE_CLAPACK \
					 -I$(KALDIROOT) \
					 -I$(FSTROOT)/include \
