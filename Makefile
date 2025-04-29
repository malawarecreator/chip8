CC       = gcc
CFLAGS   = -Wall -Wextra -O2 $(shell sdl2-config --cflags)
LDFLAGS  = $(shell sdl2-config --libs)
TARGET   = bin/chip8
SRC      = src/chip8.c

$(TARGET): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	rm -rf bin
