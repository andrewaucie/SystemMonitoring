CC = gcc
CFLAGS = -Wall
HEADER = main.h

all: b09a3

b09a3: main.o stats_functions.o
	$(CC) $(CFLAGS) -o $@ $^

.%o : %.c $(HEADER)
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o