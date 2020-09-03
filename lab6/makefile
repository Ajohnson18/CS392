CC     = gcc
CFLAGS = -g -Wall -Werror -pedantic-errors

all: main libsum.so

# TODO: build main
main: main.c 
	$(CC) $(CFLAGS) -o main main.c -ldl

# TODO: build libsum.so
libsum.so: sum.c
	$(CC) $(CFLAGS) -shared -fPIC -o libsum.so sum.c

clean:
	rm -f libsum.so main main.exe
