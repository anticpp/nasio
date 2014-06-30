
ALL: main

CPPFLAGS=-MD -o3

main: main.o
	$(CXX) $^ -o $@


clean:
	rm -f *.o *.d main
