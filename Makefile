output: main.o read_functions.o
	g++ main.o read_functions.o -o sniffer

main.o: main.cpp
	g++ -c main.cpp 

read_functions.o: read_functions.cpp read_functions.h
	g++ -c read_functions.cpp

clean:
	rm *.o *.out *.txt *.pipe sniffer
