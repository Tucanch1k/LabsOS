CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2

TARGET = myls
SRC = myls.c

all: $(TARGET)

$(TARGET): $(SRC) 
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

rebuild: clean all

