$(CXX)   = g++ -g
BINDIR   = bin
BINFILES = $(BINDIR)/test_server \
		   $(BINDIR)/test_client \
		   $(BINDIR)/test_worker

all: $(BINDIR) test_server test_client test_worker

$(BINDIR):
	mkdir $(BINDIR)

test_server: $(BINDIR) *.cc *.h
	$(CXX) -o $(BINDIR)/test_server test_server.cc Worker.cc TaskDispatcher.cc Server.cc Socket.cc ThreadedServer.cc

test_client: $(BINDIR) test_client.cc Socket.cc Socket.h
	$(CXX) -o $(BINDIR)/test_client test_client.cc Socket.cc

test_worker: $(BINDIR) test_worker.cc Task.h Worker.cc Worker.h 
	$(CXX) -o $(BINDIR)/test_worker test_worker.cc Worker.cc 

clean:
	-rm -f $(BINFILES)
	
