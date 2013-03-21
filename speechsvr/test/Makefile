include ../../make.mk
OBJ = ConfigTest.o tests.o ../Config.o
OUT = tests

CCFLAGS += -I$(GTEST_ROOT)/include
LIBS = -L$(GTEST_ROOT)/lib -lgtest

INCLUDES = -I../

all: $(OUT) test

$(OUT): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBS)

%.o: %.cc
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: test

test:
	./$(OUT)

clean:
	-rm -f $(OBJ)
