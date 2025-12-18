CC = gcc
CFLAGS = -Wall -Wextra

all: rwlock

rwlock: rwlock.c
	gcc -Wall -pthread -o rwlock rwlock.c

run: rwlock
	./rwlock

clean:
	rm -f rwlock *.o

.PHONY: all clean
