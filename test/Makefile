TARGET=unit_test

GTEST_DIR=/Users/supergui/Codes/packages/gtest-1.7.0/
INCLUDE=-I../src/ -I$(GTEST_DIR)/include/
LIBS=../src/libnasio.a $(GTEST_DIR)/lib/libgtest.a
CFLAGS=-g -Wall -std=c99 $(INCLUDE)
CXXFLAGS=-g -Wall $(INCLUDE)

$(TARGET): unit_test.o
	$(CXX) $^ -o $@ $(LIBS)

clean:
	rm -rf *.o $(TARGET)
