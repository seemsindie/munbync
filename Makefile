
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -O2
LDFLAGS = 

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS += 
endif
ifeq ($(UNAME_S),Darwin)
    LDFLAGS += 
endif

# Windows (MinGW)
ifeq ($(OS),Windows_NT)
    LDFLAGS += -lws2_32
endif

# Source files
SOURCES = munbyn_printer.c
HEADERS = munbyn_printer.h
OBJECTS = $(SOURCES:.c=.o)

# Library
LIBRARY = libmunbyn.a
SHARED_LIB = libmunbyn.so

# Example programs
EXAMPLE = example
EXAMPLE_SOURCES = example.c
FEED_TEST = feed_test
FEED_TEST_SOURCES = feed_test.c
CHARSET_EXAMPLE = charset_example
CHARSET_EXAMPLE_SOURCES = charset_example.c
CODEPAGE_TABLE_EXAMPLE = codepage_table_example
CODEPAGE_TABLE_EXAMPLE_SOURCES = codepage_table_example.c
TEXT_FORMATTING_EXAMPLE = text_formatting_example
TEXT_FORMATTING_EXAMPLE_SOURCES = text_formatting_example.c
BARCODE_EXAMPLE = barcode_example
BARCODE_EXAMPLE_SOURCES = barcode_example.c
COMPREHENSIVE_BARCODE_EXAMPLE = comprehensive_barcode_example
COMPREHENSIVE_BARCODE_EXAMPLE_SOURCES = comprehensive_barcode_example.c

# Default target
all: $(LIBRARY) $(EXAMPLE) $(FEED_TEST) $(CHARSET_EXAMPLE) $(CODEPAGE_TABLE_EXAMPLE) $(TEXT_FORMATTING_EXAMPLE) $(BARCODE_EXAMPLE) $(COMPREHENSIVE_BARCODE_EXAMPLE)

# Static library
$(LIBRARY): $(OBJECTS)
	ar rcs $@ $^
	ranlib $@

# Shared library (Linux/Mac)
$(SHARED_LIB): $(OBJECTS)
	$(CC) -shared -fPIC -o $@ $^ $(LDFLAGS)

# Example programs
$(EXAMPLE): $(EXAMPLE_SOURCES) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lmunbyn $(LDFLAGS)

$(FEED_TEST): $(FEED_TEST_SOURCES) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lmunbyn $(LDFLAGS)

$(CHARSET_EXAMPLE): $(CHARSET_EXAMPLE_SOURCES) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lmunbyn $(LDFLAGS)

$(CODEPAGE_TABLE_EXAMPLE): $(CODEPAGE_TABLE_EXAMPLE_SOURCES) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lmunbyn $(LDFLAGS)

$(TEXT_FORMATTING_EXAMPLE): $(TEXT_FORMATTING_EXAMPLE_SOURCES) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lmunbyn $(LDFLAGS)

$(BARCODE_EXAMPLE): $(BARCODE_EXAMPLE_SOURCES) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lmunbyn $(LDFLAGS)

$(COMPREHENSIVE_BARCODE_EXAMPLE): $(COMPREHENSIVE_BARCODE_EXAMPLE_SOURCES) $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lmunbyn $(LDFLAGS)

# Object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(LIBRARY) $(SHARED_LIB) $(EXAMPLE) $(FEED_TEST) $(CHARSET_EXAMPLE) $(CODEPAGE_TABLE_EXAMPLE) $(TEXT_FORMATTING_EXAMPLE) $(BARCODE_EXAMPLE) $(COMPREHENSIVE_BARCODE_EXAMPLE)

# Install (Linux/Mac)
install: $(LIBRARY) $(SHARED_LIB)
	sudo cp $(LIBRARY) /usr/local/lib/
	sudo cp $(SHARED_LIB) /usr/local/lib/
	sudo cp munbyn_printer.h /usr/local/include/
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
	@echo "  $(LIBRARY)      - Build static library"
	@echo "  $(SHARED_LIB)      - Build shared library"
	@echo "  $(EXAMPLE)        - Build basic example program"
	@echo "  $(FEED_TEST)   - Build feed line test program"
	@echo "  $(CHARSET_EXAMPLE) - Build character set example program"
	@echo "  $(CODEPAGE_TABLE_EXAMPLE) - Build codepage table generator"
	@echo "  $(TEXT_FORMATTING_EXAMPLE) - Build text formatting demo program"
	@echo "  $(BARCODE_EXAMPLE) - Build barcode demo program"
	@echo "  $(COMPREHENSIVE_BARCODE_EXAMPLE) - Build comprehensive barcode test"
	@echo "  clean       - Remove build files"
	@echo "  install     - Install library system-wide (Linux/Mac)"
	@echo "  uninstall   - Remove installed library (Linux/Mac)"
	@echo "  help        - Show this help message"

.PHONY: all clean install uninstall help