CC = gcc
CFLAGS = -std=c99 -Wall -O2
OBJS = main.o

build: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o a.out

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -r $(OBJS) a.out

