include ../make.mk
OBJ = test_server.o test_client.o test_worker.o
OUT = test_server test_client test_worker
LINK = ../threaded-server.a -lpthread
INCLUDES = -I ../src

all: $(OUT)

test_server: test_server.o
	$(CC) -o $@ $< $(LINK)

test_client: test_client.o
	$(CC) -o $@ $< $(LINK)

test_worker: test_worker.o
	$(CC) -o $@ $< $(LINK)


SUBDIRS = unit
.PHONY: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

%.o: %.cc
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $@

clean:
	-rm -f $(OBJ) $(OUT)
