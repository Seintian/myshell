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
TEST_MODULES = $(wildcard $(TESTDIR)/*_unity.c)
TEST_MODULE_OBJECTS = $(TEST_MODULES:$(TESTDIR)/%.c=$(BUILDDIR)/%.o)
TEST_RUNNER_SRC = $(TESTDIR)/test_runner.c
TEST_RUNNER_OBJ = $(BUILDDIR)/test_runner.o
UNITY_SRC = $(TESTDIR)/unity/unity.c
UNITY_OBJ = $(BUILDDIR)/unity.o
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

# Tests (Unity framework)
tests: $(TEST_TARGET)

$(UNITY_OBJ): $(UNITY_SRC) $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(UNITY_SRC) -o $(UNITY_OBJ)

$(TEST_TARGET): $(UNITY_OBJ) $(TEST_RUNNER_OBJ) $(TEST_MODULE_OBJECTS) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS))
	$(CC) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS)) $(UNITY_OBJ) $(TEST_RUNNER_OBJ) $(TEST_MODULE_OBJECTS) -o $(TEST_TARGET) $(LDFLAGS)

$(BUILDDIR)/%.o: $(TESTDIR)/%.c
	$(CC) $(CFLAGS) -I$(TESTDIR) -I$(TESTDIR)/unity -c $< -o $@

# Run the shell
run: $(TARGET)
	./$(TARGET)

# Run tests
test: tests
	@if [ -f $(TEST_TARGET) ]; then \
		echo "Running Unity test suite..."; \
		$(TEST_TARGET); \
	else \
		echo "No tests found. Create test files in $(TESTDIR)/ with _unity.c suffix to enable testing."; \
	fi

# Run individual test modules
test-lexer: $(BUILDDIR)/test_lexer_unity.o $(UNITY_OBJ) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS))
	$(CC) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS)) $(UNITY_OBJ) $(BUILDDIR)/test_lexer_unity.o -o $(BUILDDIR)/test_lexer $(LDFLAGS)
	$(BUILDDIR)/test_lexer

test-parser: $(BUILDDIR)/test_parser_unity.o $(UNITY_OBJ) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS))
	$(CC) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS)) $(UNITY_OBJ) $(BUILDDIR)/test_parser_unity.o -o $(BUILDDIR)/test_parser $(LDFLAGS)
	$(BUILDDIR)/test_parser

test-env: $(BUILDDIR)/test_env_unity.o $(UNITY_OBJ) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS))
	$(CC) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS)) $(UNITY_OBJ) $(BUILDDIR)/test_env_unity.o -o $(BUILDDIR)/test_env $(LDFLAGS)
	$(BUILDDIR)/test_env

test-util: $(BUILDDIR)/test_util_unity.o $(UNITY_OBJ) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS))
	$(CC) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS)) $(UNITY_OBJ) $(BUILDDIR)/test_util_unity.o -o $(BUILDDIR)/test_util $(LDFLAGS)
	$(BUILDDIR)/test_util

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

# Create a Unity test template
create-test:
	@mkdir -p $(TESTDIR)
	@if [ ! -f $(TESTDIR)/test_example_unity.c ]; then \
		echo "Creating Unity test template..."; \
		echo '#include "unity.h"' > $(TESTDIR)/test_example_unity.c; \
		echo '#include "your_module.h"' >> $(TESTDIR)/test_example_unity.c; \
		echo '' >> $(TESTDIR)/test_example_unity.c; \
		echo 'void setUp(void) {' >> $(TESTDIR)/test_example_unity.c; \
		echo '    // Set up before each test' >> $(TESTDIR)/test_example_unity.c; \
		echo '}' >> $(TESTDIR)/test_example_unity.c; \
		echo '' >> $(TESTDIR)/test_example_unity.c; \
		echo 'void tearDown(void) {' >> $(TESTDIR)/test_example_unity.c; \
		echo '    // Clean up after each test' >> $(TESTDIR)/test_example_unity.c; \
		echo '}' >> $(TESTDIR)/test_example_unity.c; \
		echo '' >> $(TESTDIR)/test_example_unity.c; \
		echo 'void test_example_function(void) {' >> $(TESTDIR)/test_example_unity.c; \
		echo '    TEST_ASSERT_EQUAL(1, 1);' >> $(TESTDIR)/test_example_unity.c; \
		echo '}' >> $(TESTDIR)/test_example_unity.c; \
		echo '' >> $(TESTDIR)/test_example_unity.c; \
		echo 'int main(void) {' >> $(TESTDIR)/test_example_unity.c; \
		echo '    UNITY_BEGIN();' >> $(TESTDIR)/test_example_unity.c; \
		echo '    RUN_TEST(test_example_function);' >> $(TESTDIR)/test_example_unity.c; \
		echo '    return UNITY_END();' >> $(TESTDIR)/test_example_unity.c; \
		echo '}' >> $(TESTDIR)/test_example_unity.c; \
		echo "Unity test template created in $(TESTDIR)/test_example_unity.c"; \
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
	@echo "  test        - Build and run Unity test suite"
	@echo "  test-lexer  - Run lexer tests only"
	@echo "  test-parser - Run parser tests only"
	@echo "  test-env    - Run environment tests only"
	@echo "  test-util   - Run utility tests only"
	@echo "  debug       - Build with debug symbols and no optimization"
	@echo "  release     - Build optimized release version"
	@echo "  install     - Install shell to /usr/local/bin (requires sudo)"
	@echo "  uninstall   - Remove shell from /usr/local/bin (requires sudo)"
	@echo "  clean       - Remove build files"
	@echo "  distclean   - Remove build files and temporary files"
	@echo "  create-test - Create sample test files"
	@echo "  memcheck    - Run valgrind memory check (requires debug symbols)"
	@echo "  memcheck-simple - Basic memory allocation analysis"
	@echo "  asan        - Build with AddressSanitizer (alternative to valgrind)"
	@echo "  format      - Format code with clang-format"
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
	@echo "Test modules: $(words $(TEST_MODULES))"
	@echo "Unity tests: $(if $(wildcard $(TESTDIR)/unity/unity.c),1,0)"
	@echo ""
	@ls -la $(SRCDIR)/ | grep "\.c$$" | wc -l | xargs echo "C source files:"
	@ls -la $(INCDIR)/ | grep "\.h$$" | wc -l | xargs echo "Header files:"
	@ls -la $(TESTDIR)/ | grep "_unity\.c$$" | wc -l | xargs echo "Unity test modules:"
	@if [ -f $(TARGET) ]; then \
		echo "Executable: $(TARGET) (built)"; \
		ls -lh $(TARGET); \
	else \
		echo "Executable: $(TARGET) (not built)"; \
	fi
	@if [ -f $(TEST_TARGET) ]; then \
		echo "Test suite: $(TEST_TARGET) (built)"; \
		echo "Run 'make test' to execute $(words $(TEST_MODULES)) test modules"; \
	else \
		echo "Test suite: $(TEST_TARGET) (not built)"; \
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
		echo "exit" | valgrind --leak-check=full --show-leak-kinds=all --error-limit=no ./$(TARGET) 2>&1 || \
		(echo ""; echo "Standard valgrind failed. Trying with minimal flags..."; \
		 echo "exit" | valgrind --tool=memcheck --leak-check=yes ./$(TARGET) 2>&1 || \
		 (echo ""; echo "Valgrind failed completely. This may be due to:"; \
		  echo "1. Missing debug symbols - try: export DEBUGINFOD_URLS=\"https://debuginfod.archlinux.org\""; \
		  echo "2. Valgrind compatibility issues with your kernel/glibc version"; \
		  echo "3. Try installing: sudo pacman -S gdb"; \
		  echo ""; \
		  echo "Alternative solutions:"; \
		  echo "- Use 'make memcheck-simple' for basic static analysis"; \
		  echo "- Use 'make debug' and run with gdb for manual debugging"; \
		  echo "- Try AddressSanitizer: make clean && make CFLAGS=\"-fsanitize=address -g\" && ./bin/myshell"; \
		  false)); \
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

# Simple memory check without valgrind (basic static analysis)
memcheck-simple: $(TARGET)
	@echo "Running basic memory checks..."
	@echo "Checking malloc/free balance:"
	@grep -rn "malloc\|calloc\|realloc" $(SRCDIR)/ | wc -l | xargs echo "  Memory allocation calls:"
	@grep -rn "free" $(SRCDIR)/ | wc -l | xargs echo "  Free calls:"
	@echo ""
	@echo "Checking for potential issues:"
	@grep -rn "strdup\|strndup" $(SRCDIR)/ | wc -l | xargs echo "  strdup calls (require free):"
	@grep -rn "return.*malloc\|return.*calloc" $(SRCDIR)/ | wc -l | xargs echo "  Functions returning malloc'd pointers:"
	@echo ""
	@echo "For detailed memory analysis, install debug symbols and use 'make memcheck'"

# AddressSanitizer build (alternative to valgrind)
asan: CFLAGS += -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -O1
asan: LDFLAGS += -fsanitize=address -fsanitize=undefined
asan: clean all
	@echo "Built with AddressSanitizer. Run with: ./$(TARGET)"
	@echo "AddressSanitizer will detect memory errors at runtime."
	@echo "Set ASAN_OPTIONS=abort_on_error=1 to stop on first error."

.PHONY: all clean distclean run test debug release install uninstall plugins tests help status check create-test memcheck memcheck-simple asan format
