
all: main

CPPFLAGS=-MD -O3

main: main.o
	$(CXX) $^ -o $@


clean:
	rm -f *.o *.d main
