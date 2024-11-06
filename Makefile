# Makefile

# Compiler and flags
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -g
LDFLAGS =
LIBS = -lpthread  # Add other libraries if needed

# Directories
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin
INCLUDE_DIR = include

# Find all .c files in src and its subdirectories
SOURCES = $(shell find $(SRC_DIR) -name '*.c') 

# Replace .c with .o and adjust paths for object files
OBJECTS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Target executable
TARGET = $(BIN_DIR)/distributor

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)

# Compile source files to object files
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@find $(OBJ_DIR) -type f -name '*.o' -delete
	@rm -rf $(BIN_DIR)

.PHONY: all clean
