CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = mtx

all: $(TARGETS)

mtx: mtx.c
	$(CC) $(CFLAGS) -o mtx mtx.c

clean:
	rm -f $(TARGETS)

.PHOMY: all clean
