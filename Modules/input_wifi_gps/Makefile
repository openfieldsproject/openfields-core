# Compiler and tools
CC = gcc
SPLINT = splint
VALGRIND = valgrind
CPPCHECK = cppcheck
CLANG_TIDY = clang-tidy

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Sources and objects
SRCS = $(wildcard $(SRC_DIR)/*.c)
BUILD_MODE ?= release
BUILD_SUBDIR = $(BUILD_DIR)/$(BUILD_MODE)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_SUBDIR)/%.o, $(SRCS))
TARGET = input_wifi_gps

# Compiler flags
CFLAGS_RELEASE = -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wconversion -Wsign-conversion -Werror -I$(INCLUDE_DIR) -O2
CFLAGS_DEBUG   = -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wconversion -Wsign-conversion -Werror -I$(INCLUDE_DIR) -O0 -g

LDFLAGS = -lmosquitto -lm -lssl -lcrypto

ifeq ($(BUILD_MODE),debug)
	CFLAGS = $(CFLAGS_DEBUG)
else
	CFLAGS = $(CFLAGS_RELEASE)
endif

.PHONY: all clean debug release splint cppcheck clang valgrind

# Default build (Splint bypassed)
all: $(BUILD_SUBDIR) $(TARGET)

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

# Optional static analysis targets

splint:
	@echo "Running Splint static analysis (optional)..."
	@$(SPLINT) $(SRCS) || echo "Splint detected issues (ignored for now)."

cppcheck:
	@echo "Running cppcheck static analysis..."
	@$(CPPCHECK) --enable=all --inconclusive --force $(SRC_DIR)

clang:
	@echo "Running clang-tidy static analysis..."
	@$(CLANG_TIDY) $(SRCS) -- -I$(INCLUDE_DIR)

# Memory checking target with valgrind
valgrind: $(TARGET)
	$(VALGRIND) --leak-check=full --track-origins=yes ./$(TARGET)


