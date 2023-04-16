CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib

all: RefactoredTL.a

RefactoredTL.a: RefactoredTL.o
	$(AR) RefactoredTL.a RefactoredTL.o
	$(RANLIB) RefactoredTL.a

RefactoredTL.o: RefactoredTL.h

clean:
	rm -rf testfile *.o *.a