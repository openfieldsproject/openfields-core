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
BUILD_SUBDIR = $(BUILD_DIR)/$(BUILD_MODE)

OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_SUBDIR)/%.o, $(SRCS))

TARGET = set_astro_daily

# Compiler flags
CFLAGS_RELEASE = -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 \
                 -Wconversion -Wsign-conversion -Werror \
                 -I$(INCLUDE_DIR) -O2

CFLAGS_DEBUG   = -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 \
                 -Wconversion -Wsign-conversion -Werror \
                 -I$(INCLUDE_DIR) -O0 -g

ifeq ($(BUILD_MODE),debug)
	CFLAGS = $(CFLAGS_DEBUG)
else
	CFLAGS = $(CFLAGS_RELEASE)
endif

LDFLAGS = -lgps -lofp

.PHONY: all clean debug release cppcheck scan-build valgrind fullcheck

############################################################
# Default target: run full pipeline (cppcheck → build → valgrind)
############################################################
all: fullcheck


############################################################
# Full check pipeline
############################################################
fullcheck: cppcheck $(TARGET) valgrind


############################################################
# Build rules
############################################################

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(BUILD_SUBDIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_SUBDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_SUBDIR):
	mkdir -p $(BUILD_SUBDIR)


############################################################
# Cleaning
############################################################
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm -f cppcheck.log


############################################################
# Build mode helpers
############################################################
debug:
	$(MAKE) BUILD_MODE=debug

release:
	$(MAKE) BUILD_MODE=release


############################################################
# Static Analysis: Cppcheck
############################################################
cppcheck:
	@echo "Running Cppcheck..."
	@$(CPPCHECK) --enable=all --inconclusive --std=c11 \
		--suppress=missingIncludeSystem \
		--error-exitcode=1 \
		-I$(INCLUDE_DIR) $(SRC_DIR) 2>&1 | tee cppcheck.log


############################################################
# Optional: Clang Static Analyzer
############################################################
scan-build:
	$(SCANBUILD) $(MAKE) clean all


############################################################
# Valgrind memory check
############################################################
valgrind: $(TARGET)
	@echo "Running Valgrind..."
	timeout 10s $(VALGRIND) --leak-check=full --track-origins=yes ./$(TARGET)
