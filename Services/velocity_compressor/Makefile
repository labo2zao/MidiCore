# Makefile for Velocity Compressor Module

CC = gcc
CFLAGS = -Wall -Wextra -O2 -DSTANDALONE_TEST
LDFLAGS = -lm

# Source files
SRC = velocity_compressor.c velocity_compressor_test.c
OBJ = $(SRC:.c=.o)
TARGET = velocity_compressor_test

# Default target
all: $(TARGET)

# Build test executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Run tests
test: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(OBJ) *.o

# Rebuild everything
rebuild: clean all

.PHONY: all test clean rebuild
