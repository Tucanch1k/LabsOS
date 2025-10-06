TARGET = fork_demo
CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SRC = fork_demo.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o
	
run: $(TARGET)
	./$(TARGET)
