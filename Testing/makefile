CC = gcc
CFLAGS = -g -w

all:: test

test:
	$(CC) $(CFLAGS) -o test test.c -L../ -lRefactoredTL

clean:
	rm -rf test