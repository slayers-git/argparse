CC=gcc
LIB=argparse.a
SHARED_LIB=libargparse.so
SOURCE=argparse.c
INCLUDE=argparse.h
EXAMPLE=example
EXAMPLE_S=example.c
CFLAGS=-fPIC -O2

all: shared static

object:
	$(CC) -c -o $(SOURCE:.c=.o) $(CFLAGS) $(SOURCE)

static: object
	ar rcs $(LIB) $(SOURCE:.c=.o)
shared: object
	gcc -shared -o $(SHARED_LIB) $(SOURCE:.c=.o)

example:
	gcc -o $(EXAMPLE) $(EXAMPLE_S) $(SOURCE)

clean:
	rm -f $(SHARED_LIB) $(LIB) $(SOURCE:.c=.o) $(EXAMPLE)

install:
	cp $(INCLUDE) /usr/include
	cp $(SHARED_LIB) /usr/lib/$(SHARED_LIB) 2>/dev/null || :
	cp $(LIB) /usr/lib/$(LIB) 2>/dev/null || :
	chmod 0755 /usr/lib/$(LIB) $(SHARED_LIB)
	ldconfig
