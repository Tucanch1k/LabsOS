CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2

PROGS = cat grep

all: $(PROGS)

mycat: cat.c
	$(CC) $(CFLAGS) -o $@ $<

mygrep: grep.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS) *.o

rebuild: clean all
