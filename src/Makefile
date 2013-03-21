include ../make.mk
OBJ = Semaphore.o \
			Server.o \
			Socket.o \
			Task.o \
			TaskDispatcher.o \
			ThreadedServer.o \
			Worker.o \
			SocketTask.o
OUT = ../threaded-server.a

all: $(OUT)

$(OUT): $(OBJ)
	ar -cru $(OUT) $(OBJ)
	ranlib $(OUT)

%.o: %.cc %.h
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	-rm -f $(OBJ) $(OUT)
