include kaldi.mk
SUBDIRS = online test
OBJ = Config.o DecodeWorker.o protocol.o SpeechServer.o RemoteAudioSource.o
OUT = speechsvr speechclient

LIBS += ../threaded-server.a online-decoder/online-decoder.a \

CCFLAGS += -I../src -Ionline-decoder \

.PHONY: all  $(SUBDIRS)

all: $(SUBDIRS) speechsvr speechclient

speechsvr: $(OBJ) speechsvr.o
	$(CC) $(CCFLAGS) -o $@ $(OBJ) speechsvr.o $(LIBS)

speechclient: $(OBJ) speechclient.o
	$(CC) $(CCFLAGS) -o $@ $(OBJ) speechclient.o $(LIBS)

%.o: %.cc
	$(CC) $(CCFLAGS) -c $< -o $@

$(SUBDIR):
	$(MAKE) -C $@

clean:
	-rm -f $(OBJ) $(OUT) speechsvr.o speechclient.o
	-for x in $(SUBDIRS); do $(MAKE) -C $x clean; done
