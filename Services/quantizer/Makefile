# Makefile for Timing Quantizer Module

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../..
LDFLAGS =

# Source files
SRC = quantizer.c quantizer_example.c
OBJ = $(SRC:.c=.o)
TARGET = quantizer_example

# Default target
all: $(TARGET)

# Build example executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Run example
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(OBJ) *.o

# Rebuild everything
rebuild: clean all

.PHONY: all run clean rebuild
