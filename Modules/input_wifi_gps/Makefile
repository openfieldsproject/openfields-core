# Compiler and tools
CC = gcc
VALGRIND = valgrind
CPPCHECK = cppcheck
SCANBUILD = scan-build

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
TARGET = input_wifi_gps

# Compiler flags (without linker libs)
CFLAGS_RELEASE = -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wconversion -Wsign-conversion -Werror -I$(INCLUDE_DIR) -O2
CFLAGS_DEBUG   = -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wconversion -Wsign-conversion -Werror -I$(INCLUDE_DIR) -O0 -g

# Linker flags (libraries)
LDFLAGS = -lm -lssl -lcrypto

# Select flags based on build mode
ifeq ($(BUILD_MODE),debug)
	CFLAGS = $(CFLAGS_DEBUG)
else
	CFLAGS = $(CFLAGS_RELEASE)
endif

.PHONY: all clean debug release cppcheck scan-build valgrind static

# Default build target
all: static $(BUILD_SUBDIR) $(TARGET)

# Ensure static analysis passes before building
static: cppcheck

# Build target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# Compile object files
$(BUILD_SUBDIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_SUBDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build subdirectory
$(BUILD_SUBDIR):
	mkdir -p $(BUILD_SUBDIR)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm cppcheck.log

# Debug/release helpers
debug:
	$(MAKE) BUILD_MODE=debug

release:
	$(MAKE) BUILD_MODE=release

# === Static Analysis Targets ===

# Cppcheck (fails build on errors or warnings)
cppcheck:
	@echo "Running Cppcheck static analysis (strict mode)..."
	@$(CPPCHECK) --enable=all --inconclusive --std=c11 --suppress=missingIncludeSystem -I$(INCLUDE_DIR) $(SRC_DIR) 2>&1 | tee cppcheck.log
	@if grep -E -q "(error|warning)" cppcheck.log; then \
		echo "Cppcheck detected errors or warnings, stopping build."; \
		exit 1; \
	fi

# Clang Static Analyzer (optional)
scan-build:
	@echo "Running Clang Static Analyzer..."
	@$(SCANBUILD) $(CC) $(CFLAGS) -c $(SRCS)

# Memory checking target with Valgrind (runs executable)
valgrind: $(TARGET)
	timeout 10s $(VALGRIND) --leak-check=full --track-origins=yes ./$(TARGET)
