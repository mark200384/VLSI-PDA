CC = g++
LIBS = -std=c++11 -O3
OBJS = main.o
hw3: $(OBJS)
	$(CC) $(OBJS) -o ../bin/hw4
main.o: main.cpp
	$(CC) -c main.cpp $(LIBS)
clean:
	rm *.o
	rm ../bin/*