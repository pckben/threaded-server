include ../kaldi.mk
OBJ = online-cmn.o online-decodable.o online-faster-decoder.o onlinebin-util.o \
			online-feat-input.o online-vad.o
OUT = online-decoder.a

CCFLAGS += -I../src \
					 -DKALDI_DOUBLEPRECISION=0 -DHAVE_POSIX_MEMALIGN \
					 -I$(KALDIROOT) \

all: $(OUT)

$(OUT): $(OBJ)
	ar -cru $(OUT) $(OBJ)
	ranlib $(OUT)

%.o: %.cc %.h
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	-rm -f $(OBJ) $(OUT)
