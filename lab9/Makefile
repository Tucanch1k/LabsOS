CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = lab_semaphore sender receiver

all: $(TARGETS)

lab_semaphore: lab_semaphore.c
	$(CC) $(CFLAGS) -o lab_semaphore lab_semaphore.c -lpthread

sender: sender.c
	$(CC) $(CFLAGS) -o sender sender.c

receiver: receiver.c
	$(CC) $(CFLAGS) -o receiver receiver.c

clean:
	rm -f $(TARGETS)

.PHOMY: all clean
