# Cross compiler prefix
CROSS_COMPILE ?= aarch64-linux-gnu-

# Compiler and tools
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

# Application name
TARGET = gmsl_tool

# Source files
SRCS = \
    src/args.c \
    src/deserializer.c \
    src/i2c_func.c \
    src/max9295d.c \
    src/max96712.c \
    src/max96714.c \
    src/max96717.c \
    src/max96792.c \
    src/serdes_setup.c \
    src/serializer.c

# Object files
OBJS = $(SRCS:.c=.o)

# C flags
CFLAGS = -Wall -O2 -march=armv8.2-a -Iinclude

# Header dependencies
HEADERS = serdes_head.h version.h

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile rule
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(TARGET)

