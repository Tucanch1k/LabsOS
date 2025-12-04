CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = pipe fifo

all: $(TARGETS)

pipe: pipe.c
	$(CC) $(CFLAGS) -o pipe pipe.c

fifo: fifo1.c  fifo2.c
	$(CC) $(CFLAGS) -o process1 fifo1.c
	$(CC) $(CFLAGS) -o process2 fifo2.c
clean:
	rm -f $(TARGETS)

.PHONY: all clean
