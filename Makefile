BINFILES = test_server test_client \
		   test_thread

all: test_server test_client test_thread

test_server: *.cc *.h
	g++ -o test_server test_server.cc \
			Thread.cc ThreadPool.cc Runner.cc Server.cc Socket.cc

test_client: test_client.cc Socket.cc Socket.h
	g++ -o test_client test_client.cc Socket.cc

test_thread: test_thread.cc Runner.cc Runner.h Thread.cc Thread.h ThreadPool.cc ThreadPool.h
	g++ -o test_thread test_thread.cc Runner.cc Thread.cc ThreadPool.cc

clean:
	-rm -f $(BINFILES)
	
