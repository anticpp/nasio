
TARGET=libnasio.a
OBJS=nlist.o npool.o nasio.o nasio_net.o nbuffer.o

CFLAGS=-Wall -Wunused-function -std=c99

$(TARGET):$(OBJS)
	$(AR) rc $(TARGET) $(OBJS)

clean:
	rm -rf *.o $(TARGET)
