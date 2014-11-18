TARGETS=unit_test echo_server

GTEST_DIR=/Users/supergui/Codes/packages/gtest-1.7.0/
INCLUDE=-I../src/ -I$(GTEST_DIR)/include/
LIBS=../src/libnasio.a $(GTEST_DIR)/lib/libgtest.a /usr/local/lib//libev.a
CFLAGS=-g -Wall -std=c99 $(INCLUDE)
CXXFLAGS=-g -Wall $(INCLUDE)

.PHONY:
	all

all: $(TARGETS)

unit_test: unit_test.o
	$(CXX) $^ -o $@ $(LIBS)

echo_server: echo_server.o
	$(CC) $^ -o $@ $(LIBS)


clean:
	rm -rf *.o $(TARGETS)
