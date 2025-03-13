CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = bin/chip8.exe
SRC = src/chip8.c

$(TARGET): $(SRC)
	@if not exist bin (mkdir bin)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	del $(TARGET)