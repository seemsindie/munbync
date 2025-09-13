
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -O2
LDFLAGS = 

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -lm
endif
ifeq ($(UNAME_S),Darwin)
    LDFLAGS += 
endif

# Windows (MinGW)
ifeq ($(OS),Windows_NT)
    LDFLAGS += -lws2_32
endif

# Directory structure
SRC_DIR = src
EXAMPLES_DIR = examples
BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib
OBJ_DIR = $(BUILD_DIR)/obj
EXAMPLES_BUILD_DIR = $(BUILD_DIR)/examples

# Source files
SOURCES = $(SRC_DIR)/munbyn_printer.c
HEADERS = $(SRC_DIR)/munbyn_printer.h
OBJECTS = $(OBJ_DIR)/munbyn_printer.o

# Library
LIBRARY = $(LIB_DIR)/libmunbyn.a
SHARED_LIB = $(LIB_DIR)/libmunbyn.so

# Example programs
EXAMPLE = $(EXAMPLES_BUILD_DIR)/example
EXAMPLE_SOURCES = $(EXAMPLES_DIR)/example.c
FEED_TEST = $(EXAMPLES_BUILD_DIR)/feed_test
FEED_TEST_SOURCES = $(EXAMPLES_DIR)/feed_test.c
CHARSET_EXAMPLE = $(EXAMPLES_BUILD_DIR)/charset_example
CHARSET_EXAMPLE_SOURCES = $(EXAMPLES_DIR)/charset_example.c
CODEPAGE_TABLE_EXAMPLE = $(EXAMPLES_BUILD_DIR)/codepage_table_example
CODEPAGE_TABLE_EXAMPLE_SOURCES = $(EXAMPLES_DIR)/codepage_table_example.c
TEXT_FORMATTING_EXAMPLE = $(EXAMPLES_BUILD_DIR)/text_formatting_example
TEXT_FORMATTING_EXAMPLE_SOURCES = $(EXAMPLES_DIR)/text_formatting_example.c
BARCODE_EXAMPLE = $(EXAMPLES_BUILD_DIR)/barcode_example
BARCODE_EXAMPLE_SOURCES = $(EXAMPLES_DIR)/barcode_example.c
COMPREHENSIVE_BARCODE_EXAMPLE = $(EXAMPLES_BUILD_DIR)/comprehensive_barcode_example
COMPREHENSIVE_BARCODE_EXAMPLE_SOURCES = $(EXAMPLES_DIR)/comprehensive_barcode_example.c
TAB_EXAMPLE = $(EXAMPLES_BUILD_DIR)/tab_example
TAB_EXAMPLE_SOURCES = $(EXAMPLES_DIR)/tab_example.c
RASTER_IMAGE_EXAMPLE = $(EXAMPLES_BUILD_DIR)/raster_image_example
RASTER_IMAGE_EXAMPLE_SOURCES = $(EXAMPLES_DIR)/raster_image_example.c
PNG_RASTER_EXAMPLE = $(EXAMPLES_BUILD_DIR)/png_raster_example
PNG_RASTER_EXAMPLE_SOURCES = $(EXAMPLES_DIR)/png_raster_example.c
NETWORK_EXAMPLE = $(EXAMPLES_BUILD_DIR)/network_example
NETWORK_EXAMPLE_SOURCES = $(EXAMPLES_DIR)/network_example.c

# Default target
all: $(LIBRARY) $(EXAMPLE) $(FEED_TEST) $(CHARSET_EXAMPLE) $(CODEPAGE_TABLE_EXAMPLE) $(TEXT_FORMATTING_EXAMPLE) $(BARCODE_EXAMPLE) $(COMPREHENSIVE_BARCODE_EXAMPLE) $(TAB_EXAMPLE) $(RASTER_IMAGE_EXAMPLE) $(PNG_RASTER_EXAMPLE) $(NETWORK_EXAMPLE)

# Create build directories
$(BUILD_DIR) $(LIB_DIR) $(OBJ_DIR) $(EXAMPLES_BUILD_DIR):
	mkdir -p $@

# Static library
$(LIBRARY): $(OBJECTS) | $(LIB_DIR)
	ar rcs $@ $^
	ranlib $@

# Shared library (Linux/Mac)
$(SHARED_LIB): $(OBJECTS) | $(LIB_DIR)
	$(CC) -shared -fPIC -o $@ $^ $(LDFLAGS)

# Example programs
$(EXAMPLE): $(EXAMPLE_SOURCES) $(LIBRARY) | $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $< -L$(LIB_DIR) -lmunbyn $(LDFLAGS)

$(FEED_TEST): $(FEED_TEST_SOURCES) $(LIBRARY) | $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $< -L$(LIB_DIR) -lmunbyn $(LDFLAGS)

$(CHARSET_EXAMPLE): $(CHARSET_EXAMPLE_SOURCES) $(LIBRARY) | $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $< -L$(LIB_DIR) -lmunbyn $(LDFLAGS)

$(CODEPAGE_TABLE_EXAMPLE): $(CODEPAGE_TABLE_EXAMPLE_SOURCES) $(LIBRARY) | $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $< -L$(LIB_DIR) -lmunbyn $(LDFLAGS)

$(TEXT_FORMATTING_EXAMPLE): $(TEXT_FORMATTING_EXAMPLE_SOURCES) $(LIBRARY) | $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $< -L$(LIB_DIR) -lmunbyn $(LDFLAGS)

$(BARCODE_EXAMPLE): $(BARCODE_EXAMPLE_SOURCES) $(LIBRARY) | $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $< -L$(LIB_DIR) -lmunbyn $(LDFLAGS)

$(COMPREHENSIVE_BARCODE_EXAMPLE): $(COMPREHENSIVE_BARCODE_EXAMPLE_SOURCES) $(LIBRARY) | $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $< -L$(LIB_DIR) -lmunbyn $(LDFLAGS)

$(TAB_EXAMPLE): $(TAB_EXAMPLE_SOURCES) $(LIBRARY) | $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $< -L$(LIB_DIR) -lmunbyn $(LDFLAGS)

$(RASTER_IMAGE_EXAMPLE): $(RASTER_IMAGE_EXAMPLE_SOURCES) $(LIBRARY) | $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $< -L$(LIB_DIR) -lmunbyn $(LDFLAGS)

$(PNG_RASTER_EXAMPLE): $(PNG_RASTER_EXAMPLE_SOURCES) $(LIBRARY) | $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(EXAMPLES_DIR) -o $@ $< -L$(LIB_DIR) -lmunbyn $(LDFLAGS)

$(NETWORK_EXAMPLE): $(NETWORK_EXAMPLE_SOURCES) $(LIBRARY) | $(EXAMPLES_BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $< -L$(LIB_DIR) -lmunbyn $(LDFLAGS)

# Object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Install (Linux/Mac)
install: $(LIBRARY) $(SHARED_LIB)
	sudo cp $(LIBRARY) /usr/local/lib/
	sudo cp $(SHARED_LIB) /usr/local/lib/
	sudo cp $(SRC_DIR)/munbyn_printer.h /usr/local/include/
	sudo ldconfig

# Uninstall (Linux/Mac)
uninstall:
	sudo rm -f /usr/local/lib/$(LIBRARY)
	sudo rm -f /usr/local/lib/$(SHARED_LIB)
	sudo rm -f /usr/local/include/munbyn_printer.h
	sudo ldconfig

# Help
help:
	@echo "Available targets:"
	@echo "  all         - Build library and example programs"
	@echo "  library     - Build static library ($(LIBRARY))"
	@echo "  shared      - Build shared library ($(SHARED_LIB))"
	@echo "  examples    - Build all example programs"
	@echo "  clean       - Remove build directory"
	@echo "  install     - Install library system-wide (Linux/Mac)"
	@echo "  uninstall   - Remove installed library (Linux/Mac)"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Example programs are built to: $(EXAMPLES_BUILD_DIR)/"
	@echo "Libraries are built to: $(LIB_DIR)/"

# Convenience targets
library: $(LIBRARY)
shared: $(SHARED_LIB)
examples: $(EXAMPLE) $(FEED_TEST) $(CHARSET_EXAMPLE) $(CODEPAGE_TABLE_EXAMPLE) $(TEXT_FORMATTING_EXAMPLE) $(BARCODE_EXAMPLE) $(COMPREHENSIVE_BARCODE_EXAMPLE) $(TAB_EXAMPLE) $(RASTER_IMAGE_EXAMPLE) $(PNG_RASTER_EXAMPLE) $(NETWORK_EXAMPLE)

.PHONY: all library shared examples clean install uninstall help