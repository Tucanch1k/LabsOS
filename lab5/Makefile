# ===== Makefile for archiver =====

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = archiver
SRC = archiver.c

# Основная цель (по умолчанию)
all: $(TARGET)

# Правило сборки программы
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Очистка скомпилированных файлов
clean:
	rm -f $(TARGET) *.o *.arc

# Тест: быстрая проверка работы
run:
	./$(TARGET) $(ARGS)


