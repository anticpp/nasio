TARGET=unit_test

INCLUDE=-I../src/
LIBS=../src/libnasio.a
CFLAGS=-g -Wall -std=c99 $(INCLUDE)

$(TARGET): unit_test.o
	$(CC) $^ -o $@ $(LIBS)

clean:
	rm -rf *.o $(TARGET)
