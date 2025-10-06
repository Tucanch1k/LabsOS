CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = mychmod
SRC = mychmod.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET) $(ARGS)

clean:
	rm -f $(TARGET)
