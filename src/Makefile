
TARGET=libnasio.a
OBJS=nlist.o npool.o

CFLAGS=-Wall

$(TARGET):$(OBJS)
	$(AR) rc $(TARGET) $(OBJS)

clean:
	rm -rf *.o $(TARGET)
