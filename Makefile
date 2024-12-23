CC = g++
CXXFLAGS = -std=c++11 -Wall -O2
OBJS = main.o

main.o: main.c
	$(CC) $(CXXFLAGS) -c main.c

main: $(OBJS)
	$(CC) $(CXXFLAGS) $(OBJS) -o main.out

clean:
	rm -r $(OBJS) main

