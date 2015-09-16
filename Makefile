
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
UNITTEST=./bin/unit_test
ECHO_SERVER=./bin/echo_server
ECHO_CLIENT=./bin/echo_client

CFLAGS=-g -MD -Wall -O2 -std=c99 $(INCLUDES)
CXXFLAGS=-g -MD -Wall -O2 $(INCLUDES)

.PHONY: all test lib

TARGETS=$(LIBNASIO) $(UNITTEST) $(ECHO_SERVER) $(ECHO_CLIENT)

all:$(TARGETS)
test:$(UNITTEST) $(ECHO_SERVER) $(ECHO_CLIENT)
lib:$(LIBNASIO)

$(LIBNASIO):$(OBJNASIO)
	$(AR) rc $@ $^

$(UNITTEST): test/unit_test.o $(OBJNASIO)
	$(CXX) $^ -o $@ $(LIBS)

$(ECHO_SERVER): test/echo_server.o $(LIBNASIO)
	$(CXX) $^ -o $@ $(LIBNASIO) $(LIBS)

$(ECHO_CLIENT): test/echo_client.o $(LIBNASIO)
	$(CXX) $^ -o $@ $(LIBNASIO) $(LIBS)

clean:
	rm -fv $(TARGETS) src/*.o src/*.d test/*.o test/*.d
