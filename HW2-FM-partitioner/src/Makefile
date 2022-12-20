ALL: main exes

main: main.o 
	@g++ -o ../bin/hw2 main.o
main.o: main.cpp 
	@g++ -std=c++11 -c main.cpp
exes:
		@./../bin/hw2
clean:
	rm -rf main.o
	rm -rf ../bin/hw2