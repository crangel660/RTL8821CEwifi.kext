# Makefile for RTL8821CEwifi.kext
# Build the RTL8821CE PCIe WiFi driver for macOS
#
# Requirements:
# - macOS Sequoia (15.0+)
# - Xcode 15.0+ with command line tools
# - Kernel development headers
#
# Usage:
#   make          - Build the kext
#   make clean    - Clean build artifacts
#   make load     - Load the kext (requires sudo)
#   make unload   - Unload the kext (requires sudo)
#   make install  - Install to /Library/Extensions
#   make logs     - View kernel logs

# Configuration
KEXT_NAME = RTL8821CEwifi
KEXT_DIR = RTL8821CEwifi
BUILD_DIR = build
KERNEL_VERSION = $(shell uname -r)

# Compiler settings
CC = clang++
CFLAGS = -x c++ -std=c++17 -Wall -Wextra -fno-exceptions -fno-rtti
CFLAGS += -I. -I$(KEXT_DIR) -Iinclude
CFLAGS += -I$(KEXT_DIR)/HAL
CFLAGS += -I$(KEXT_DIR)/../RTL80211
CFLAGS += -I$(KEXT_DIR)/../RTL8821CE
CFLAGS += -I$(KEXT_DIR)/../RTL80211/linux
CFLAGS += -I$(KEXT_DIR)/../RTL80211/openbsd

# Kernel includes
KERNEL_INCLUDES = -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/Kernel.framework/Headers
CFLAGS += $(KERNEL_INCLUDES)

# Linker settings
LDFLAGS = -nostdlib -Wl,-kext -Wl,-bundle
LDFLAGS += -Wl,-exported_symbol,_kmod_start
LDFLAGS += -Wl,-exported_symbol,_kmod_end
LDFLAGS += -Wl,-force_load

# Frameworks
FRAMEWORKS = -framework IOKit -framework Kernel -framework Libkern

# Source files
CPP_SOURCES = \
    $(KEXT_DIR)/RTL8821CEwifi.cpp \
    $(KEXT_DIR)/HAL/RtlHalService.cpp \
    $(KEXT_DIR)/../RTL80211/compat.cpp \
    $(KEXT_DIR)/../RTL80211/rtw_bridge.cpp

# Linux driver C files (compiled as C++)
C_SOURCES = \
    $(KEXT_DIR)/../RTL8821CE/rtw8821c.c \
    $(KEXT_DIR)/../RTL8821CE/pci.c \
    $(KEXT_DIR)/../RTL8821CE/tx.c \
    $(KEXT_DIR)/../RTL8821CE/rx.c \
    $(KEXT_DIR)/../RTL8821CE/fw.c \
    $(KEXT_DIR)/../RTL8821CE/phy.c \
    $(KEXT_DIR)/../RTL8821CE/mac.c \
    $(KEXT_DIR)/../RTL8821CE/util.c \
    $(KEXT_DIR)/../RTL8821CE/debug.c \
    $(KEXT_DIR)/../RTL8821CE/efuse.c \
    $(KEXT_DIR)/../RTL8821CE/sec.c \
    $(KEXT_DIR)/../RTL8821CE/ps.c \
    $(KEXT_DIR)/../RTL8821CE/led.c \
    $(KEXT_DIR)/../RTL8821CE/wow.c \
    $(KEXT_DIR)/../RTL8821CE/coex.c \
    $(KEXT_DIR)/../RTL8821CE/regd.c \
    $(KEXT_DIR)/../RTL8821CE/sar.c \
    $(KEXT_DIR)/../RTL8821CE/bf.c \
    $(KEXT_DIR)/../RTL8821CE/main.c

# Object files
OBJECTS = $(CPP_SOURCES:.cpp=.o) $(C_SOURCES:.c=.o)

# Output
OUTPUT = $(BUILD_DIR)/$(KEXT_NAME).kext/Contents/MacOS/$(KEXT_NAME)

# Colors for output
COLOR_RESET = \033[0m
COLOR_GREEN = \033[32m
COLOR_YELLOW = \033[33m
COLOR_RED = \033[31m

.PHONY: all clean load unload install logs help

all: $(BUILD_DIR) $(OUTPUT)
	@echo "$(COLOR_GREEN)[SUCCESS]$(COLOR_RESET) Built $(KEXT_NAME).kext"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)/$(KEXT_NAME).kext/Contents/MacOS
	@cp $(KEXT_DIR)/Info.plist $(BUILD_DIR)/$(KEXT_NAME).kext/Contents/

# Compile C++ files
$(KEXT_DIR)/%.o: $(KEXT_DIR)/%.cpp
	@echo "$(COLOR_YELLOW)[C++]$(COLOR_RESET) Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(KEXT_DIR)/HAL/%.o: $(KEXT_DIR)/HAL/%.cpp
	@echo "$(COLOR_YELLOW)[C++]$(COLOR_RESET) Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(KEXT_DIR)/../RTL80211/%.o: $(KEXT_DIR)/../RTL80211/%.cpp
	@echo "$(COLOR_YELLOW)[C++]$(COLOR_RESET) Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile C files (as C++)
$(KEXT_DIR)/../RTL8821CE/%.o: $(KEXT_DIR)/../RTL8821CE/%.c
	@echo "$(COLOR_YELLOW)[CC]$(COLOR_RESET) Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Link
$(OUTPUT): $(OBJECTS)
	@echo "$(COLOR_YELLOW)[LINK]$(COLOR_RESET) Linking $(KEXT_NAME).kext"
	@$(CC) $(LDFLAGS) $(FRAMEWORKS) -o $@ $(OBJECTS)
	@echo "$(COLOR_GREEN)[INFO]$(COLOR_RESET) Setting kext permissions"
	@chmod -R 755 $(BUILD_DIR)/$(KEXT_NAME).kext
	@echo "$(COLOR_GREEN)[INFO]$(COLOR_RESET) Setting kext ownership"
	@sudo chown -R root:wheel $(BUILD_DIR)/$(KEXT_NAME).kext

clean:
	@echo "$(COLOR_YELLOW)[CLEAN]$(COLOR_RESET) Removing build artifacts"
	@rm -rf $(BUILD_DIR)
	@find . -name "*.o" -delete
	@echo "$(COLOR_GREEN)[DONE]$(COLOR_RESET) Clean complete"

load:
	@echo "$(COLOR_YELLOW)[LOAD]$(COLOR_RESET) Loading $(KEXT_NAME).kext"
	@sudo kextutil -v $(BUILD_DIR)/$(KEXT_NAME).kext
	@echo "$(COLOR_GREEN)[DONE]$(COLOR_RESET) Load complete"

unload:
	@echo "$(COLOR_YELLOW)[UNLOAD]$(COLOR_RESET) Unloading $(KEXT_NAME).kext"
	@sudo kextunload -v com.crangel.RTL8821CEwifi
	@echo "$(COLOR_GREEN)[DONE]$(COLOR_RESET) Unload complete"

install:
	@echo "$(COLOR_YELLOW)[INSTALL]$(COLOR_RESET) Installing to /Library/Extensions"
	@sudo cp -R $(BUILD_DIR)/$(KEXT_NAME).kext /Library/Extensions/
	@sudo chown -R root:wheel /Library/Extensions/$(KEXT_NAME).kext
	@sudo chmod -R 755 /Library/Extensions/$(KEXT_NAME).kext
	@sudo kextcache -i /
	@echo "$(COLOR_GREEN)[DONE]$(COLOR_RESET) Installation complete. Please reboot."

logs:
	@echo "$(COLOR_YELLOW)[LOGS]$(COLOR_RESET) Showing kernel logs for $(KEXT_NAME)"
	@log show --predicate 'process == "kernel"' --last 5m | grep -i "RTL8821CE"

help:
	@echo "RTL8821CEwifi.kext Build System"
	@echo ""
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build the kext (default)"
	@echo "  clean     - Remove build artifacts"
	@echo "  load      - Load kext (requires sudo)"
	@echo "  unload    - Unload kext (requires sudo)"
	@echo "  install   - Install to /Library/Extensions"
	@echo "  logs      - View kernel logs"
	@echo "  help      - Show this help"
	@echo ""
	@echo "Quick Start:"
	@echo "  1. make clean && make"
	@echo "  2. sudo make load"
	@echo "  3. Check logs: make logs"
	@echo ""
	@echo "Note: Requires Xcode command line tools and kernel headers"

# Debug target
debug: CFLAGS += -DDEBUG -g -O0
debug: clean all
	@echo "$(COLOR_GREEN)[DEBUG]$(COLOR_RESET) Debug build complete"

# Release target
release: CFLAGS += -O2 -DNDEBUG
release: clean all
	@echo "$(COLOR_GREEN)[RELEASE]$(COLOR_RESET) Release build complete"
