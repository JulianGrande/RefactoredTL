CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib

all: RefactoredTL.a

RefactoredTL.a: RefactoredTL.o
	$(AR) libRefactoredTL.a RefactoredTL.o
	$(RANLIB) libRefactoredTL.a

RefactoredTL.o: RefactoredTL.h

clean:
	rm -rf testfile *.o *.a