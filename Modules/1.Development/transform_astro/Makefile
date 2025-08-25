# Compiler and tools
CC = gcc
SPLINT = splint
VALGRIND = valgrind

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

# Compiler flags (without linker libs)
CFLAGS_RELEASE = -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wconversion -Wsign-conversion -Werror -I$(INCLUDE_DIR) -O2
CFLAGS_DEBUG = -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wconversion -Wsign-conversion -Werror -I$(INCLUDE_DIR) -O0 -g

# Linker flags (libraries)
LDFLAGS = -lmosquitto -lm

# Select flags based on build mode
ifeq ($(BUILD_MODE),debug)
	CFLAGS = $(CFLAGS_DEBUG)
else
	CFLAGS = $(CFLAGS_RELEASE)
endif

.PHONY: all clean debug release splint valgrind

all: splint $(BUILD_SUBDIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(BUILD_SUBDIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_SUBDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_SUBDIR):
	mkdir -p $(BUILD_SUBDIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)

debug:
	$(MAKE) BUILD_MODE=debug

release:
	$(MAKE) BUILD_MODE=release

# Static analysis target with splint
splint:
	@echo "Running Splint static analysis..."
	@$(SPLINT) $(SRCS) || (echo "Splint detected issues, stopping build." && exit 1)

# Memory checking target with valgrind (runs executable)
valgrind: $(TARGET)
	$(VALGRIND) --leak-check=full --track-origins=yes ./$(TARGET)

