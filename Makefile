# Makefile for MyShell

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -Iinclude
LDFLAGS = -ldl

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build
BINDIR = bin
TESTDIR = tests
PLUGINDIR = plugins

# Target executable
TARGET = $(BINDIR)/myshell

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
# Exclude one of the evloop implementations to avoid multiple definition errors
SOURCES := $(filter-out $(SRCDIR)/evloop_epoll.c, $(SOURCES))
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

# Plugin sources
PLUGIN_HELLO_SRC = $(PLUGINDIR)/hello/hello.c
PLUGIN_HELLO_SO = $(BUILDDIR)/hello.so

# Test sources (if any exist)
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_OBJECTS = $(TEST_SOURCES:$(TESTDIR)/%.c=$(BUILDDIR)/test_%.o)
TEST_TARGET = $(BUILDDIR)/run_tests

# Default target
all: $(TARGET) plugins

# Create build directories
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Main executable
$(TARGET): $(BUILDDIR) $(BINDIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Plugins
plugins: $(PLUGIN_HELLO_SO)

$(PLUGIN_HELLO_SO): $(PLUGIN_HELLO_SRC) $(BUILDDIR)
	$(CC) $(CFLAGS) -fPIC -shared $(PLUGIN_HELLO_SRC) -o $(PLUGIN_HELLO_SO)

# Tests (if test files exist)
tests: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS))
	$(CC) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS)) $(TEST_OBJECTS) -o $(TEST_TARGET) $(LDFLAGS)

$(BUILDDIR)/test_%.o: $(TESTDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the shell
run: $(TARGET)
	./$(TARGET)

# Run tests
test: tests
	@if [ -f $(TEST_TARGET) ]; then \
		echo "Running tests..."; \
		$(TEST_TARGET); \
	else \
		echo "No tests found. Create test files in $(TESTDIR)/ to enable testing."; \
	fi

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: clean all

# Release build
release: CFLAGS += -O2 -DNDEBUG
release: clean all

# Install (optional)
install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin/"
	@sudo cp $(TARGET) /usr/local/bin/
	@sudo chmod +x /usr/local/bin/$(TARGET)
	@echo "Installation complete"

# Uninstall
uninstall:
	@echo "Removing $(TARGET) from /usr/local/bin/"
	@sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstall complete"

# Clean build files
clean:
	rm -rf $(BUILDDIR)
	rm -f $(TARGET)

# Clean everything including backups
distclean: clean
	find . -name "*~" -delete
	find . -name "*.bak" -delete
	find . -name ".*.swp" -delete

# Create a simple test framework
create-test:
	@mkdir -p $(TESTDIR)
	@if [ ! -f $(TESTDIR)/test_lexer.c ]; then \
		echo "Creating sample test file..."; \
		echo '#include <stdio.h>' > $(TESTDIR)/test_lexer.c; \
		echo '#include <assert.h>' >> $(TESTDIR)/test_lexer.c; \
		echo '#include "lexer.h"' >> $(TESTDIR)/test_lexer.c; \
		echo '' >> $(TESTDIR)/test_lexer.c; \
		echo 'void test_lexer_basic() {' >> $(TESTDIR)/test_lexer.c; \
		echo '    printf("Testing lexer...\n");' >> $(TESTDIR)/test_lexer.c; \
		echo '    // Add your tests here' >> $(TESTDIR)/test_lexer.c; \
		echo '}' >> $(TESTDIR)/test_lexer.c; \
		echo '' >> $(TESTDIR)/test_lexer.c; \
		echo 'int main() {' >> $(TESTDIR)/test_lexer.c; \
		echo '    test_lexer_basic();' >> $(TESTDIR)/test_lexer.c; \
		echo '    printf("All tests passed!\n");' >> $(TESTDIR)/test_lexer.c; \
		echo '    return 0;' >> $(TESTDIR)/test_lexer.c; \
		echo '}' >> $(TESTDIR)/test_lexer.c; \
		echo "Sample test created in $(TESTDIR)/test_lexer.c"; \
	fi

# Show help
help:
	@echo "MyShell Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all         - Build the shell and plugins (default)"
	@echo "  $(TARGET)   - Build the main shell executable"
	@echo "  plugins     - Build all plugins"
	@echo "  tests       - Build test suite"
	@echo "  run         - Build and run the shell"
	@echo "  test        - Build and run tests"
	@echo "  debug       - Build with debug symbols and no optimization"
	@echo "  release     - Build optimized release version"
	@echo "  install     - Install shell to /usr/local/bin (requires sudo)"
	@echo "  uninstall   - Remove shell from /usr/local/bin (requires sudo)"
	@echo "  clean       - Remove build files"
	@echo "  distclean   - Remove build files and temporary files"
	@echo "  create-test - Create sample test files"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Environment variables:"
	@echo "  CC          - C compiler (default: gcc)"
	@echo "  CFLAGS      - Additional compiler flags"
	@echo "  LDFLAGS     - Additional linker flags"

# Show project status
status:
	@echo "Project Status:"
	@echo "==============="
	@echo "Source files: $(words $(SOURCES))"
	@echo "Header files: $(words $(wildcard $(INCDIR)/*.h))"
	@echo "Plugin files: $(words $(wildcard $(PLUGINDIR)/*/*.c))"
	@echo "Test files: $(words $(TEST_SOURCES))"
	@echo ""
	@ls -la $(SRCDIR)/ | grep "\.c$$" | wc -l | xargs echo "C source files:"
	@ls -la $(INCDIR)/ | grep "\.h$$" | wc -l | xargs echo "Header files:"
	@if [ -f $(TARGET) ]; then \
		echo "Executable: $(TARGET) (built)"; \
		ls -lh $(TARGET); \
	else \
		echo "Executable: $(TARGET) (not built)"; \
	fi

# Check for common issues
check:
	@echo "Running basic checks..."
	@echo "Checking for missing includes..."
	@grep -n "#include" $(SRCDIR)/*.c | grep -v "$(INCDIR)" | grep -v "<" || echo "  ✓ All includes look good"
	@echo "Checking for TODO comments..."
	@grep -n "TODO\|FIXME\|XXX" $(SRCDIR)/*.c $(INCDIR)/*.h || echo "  ✓ No TODO comments found"
	@echo "Checking for potential memory leaks (basic)..."
	@grep -n "malloc\|calloc\|strdup" $(SRCDIR)/*.c | wc -l | xargs echo "  malloc calls:"
	@grep -n "free" $(SRCDIR)/*.c | wc -l | xargs echo "  free calls:"

# Dependencies (for automatic header dependency tracking)
-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.d: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	@$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< > $@

# Valgrind memory check (if valgrind is installed)
memcheck: $(TARGET)
	@if command -v valgrind >/dev/null 2>&1; then \
		echo "Running memory check with valgrind..."; \
		echo "exit" | valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET); \
	else \
		echo "Valgrind not found. Install valgrind to run memory checks."; \
	fi

# Code formatting (if clang-format is available)
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "Formatting code..."; \
		find $(SRCDIR) $(INCDIR) -name "*.c" -o -name "*.h" | xargs clang-format -i; \
		echo "Code formatting complete"; \
	else \
		echo "clang-format not found. Install clang-format to format code."; \
	fi

.PHONY: all clean distclean run test debug release install uninstall plugins tests help status check create-test memcheck format
