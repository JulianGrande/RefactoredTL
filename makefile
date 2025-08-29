CC = gcc
CFLAGS = -g -c -Wall
AR = ar -rc
RANLIB = ranlib

TARGET_LIB = libRefactoredTL.a
OBJECTS = RefactoredTL.o

all: $(TARGET_LIB)

$(TARGET_LIB): $(OBJECTS)
	$(AR) $@ $(OBJECTS)
	$(RANLIB) $@

RefactoredTL.o: RefactoredTL.c RefactoredTL.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	# Remove object files and the static library
	rm -f $(OBJECTS) $(TARGET_LIB)