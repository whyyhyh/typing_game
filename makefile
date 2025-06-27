# Makefile
CC = gcc
CFLAGS = -Wall -O2
TARGET = typing_game

all: $(TARGET)

$(TARGET): typing_game.c
	$(CC) $(CFLAGS) -o $(TARGET) typing_game.c

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
