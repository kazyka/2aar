.PHONY: clean

CC=gcc-5
CFLAGS=-Werror -Wextra -pedantic -std=c11 -g

all: calc

calc: stack.o calc.c
	$(CC) $(CFLAGS) -o calc stack.o calc.c

stack.o: stack.c
	$(CC) $(CFLAGS) -c stack.c

clean:
	rm -f *.o
	rm -f calc
