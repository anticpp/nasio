
SRCNASIO=$(wildcard src/*.c)
OBJNASIO=$(patsubst %.c,%.o,$(SRCNASIO))
DEPNASIO=$(patsubst %c,%d,$(SRCNASIO))

SRCTEST=$(wildcard test/*.c)
OBJTEST=$(patsubst %.c,%.o,$(SRCTEST))
DEPTEST=$(patsubst %.c,%.d,$(SRCTEST))

GTEST_DIR=/Users/supergui/Codes/packages/gtest-1.7.0/
INCLUDES=-I./src/ -I$(GTEST_DIR)/include
LIBS=$(GTEST_DIR)/lib/libgtest.a /usr/local/lib//libev.a

LIBNASIO=./libs/libnasio.a
UNIT_TEST=./bin/unit_test
ECHO_SERVER=./bin/echo_server
TEST_CONNECT=./bin/test_connect

CFLAGS=-g -MD -Wall -O2 -std=c99 $(INCLUDES)
CXXFLAGS=-g -MD -Wall -O2 $(INCLUDES)

.PHONY: all test lib

TARGETS=$(LIBNASIO) $(UNIT_TEST) $(ECHO_SERVER) $(TEST_CONNECT)

all:$(TARGETS)
test:$(UNIT_TEST) $(ECHO_SERVER) $(TEST_CONNECT)
lib:$(LIBNASIO)

$(LIBNASIO):$(OBJNASIO)
	$(AR) rc $@ $^

$(UNIT_TEST): test/unit_test.o $(OBJNASIO)
	$(CXX) $^ -o $@ $(LIBS)

$(ECHO_SERVER): test/echo_server.o $(LIBNASIO)
	$(CXX) $^ -o $@ $(LIBNASIO) $(LIBS)

$(TEST_CONNECT): test/test_connect.o $(LIBNASIO)
	$(CXX) $^ -o $@ $(LIBNASIO) $(LIBS)

clean:
	rm -fv $(TARGETS) src/*.o src/*.d test/*.o test/*.d
