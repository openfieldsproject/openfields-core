# Compiler
CC = gcc

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Sources and objects
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Default build mode
BUILD_MODE ?= release

# Build mode subdirectory
BUILD_SUBDIR = $(BUILD_DIR)/$(BUILD_MODE)

# Object files path inside build subdir
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_SUBDIR)/%.o, $(SRCS))

# Output executable path inside build subdir
TARGET = transform_astro

# Flags
CFLAGS_RELEASE = -lmosquitto -lm -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wconversion -Wsign-conversion -Werror -I$(INCLUDE_DIR) -O2
CFLAGS_DEBUG = -lmosquitto -lm -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wconversion -Wsign-conversion -Werror -I$(INCLUDE_DIR) -O0 -g

# Select flags based on build mode
ifeq ($(BUILD_MODE),debug)
	CFLAGS = $(CFLAGS_DEBUG)
else
	CFLAGS = $(CFLAGS_RELEASE)
endif

.PHONY: all clean debug release

all: $(BUILD_SUBDIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_SUBDIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_SUBDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_SUBDIR):
	mkdir -p $(BUILD_SUBDIR)

clean:
	rm -rf $(BUILD_DIR)
	rm $(TARGET)

debug:
	$(MAKE) BUILD_MODE=debug

release:
	$(MAKE) BUILD_MODE=release
