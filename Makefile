# Makefile for MyShell (rebuilt)

# Compiler and base flags
CC = gcc
CFLAGS = -Wall -Wextra -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L -g -Iinclude
LDFLAGS = -ldl -lpthread

# Sanitizers for debug/testing
SANITIZERS ?= address,undefined
SAN_CFLAGS = -fsanitize=$(SANITIZERS) -fno-omit-frame-pointer -O1
SAN_LDFLAGS = -fsanitize=$(SANITIZERS)

# Strict AddressSanitizer runtime (used when running tests or debug binaries)
ASAN_STRICT_OPTS ?= detect_leaks=1:abort_on_error=1

# Hardened release toggles
FORTIFY_LEVEL ?= 2
ENABLE_LTO ?= 1
STRIP ?= strip
HARDEN_CFLAGS = -O2 -DNDEBUG -D_FORTIFY_SOURCE=$(FORTIFY_LEVEL) -fstack-protector-strong -fPIE
HARDEN_LDFLAGS = -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-z,defs -Wl,-z,separate-code -pie
ifeq ($(ENABLE_LTO),1)
HARDEN_CFLAGS += -flto
HARDEN_LDFLAGS += -flto
endif

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build
BINDIR = bin
TESTDIR = tests
PLUGINDIR = plugins

# Targets
TARGET = $(BINDIR)/myshell

# Sources/objects
SOURCES = $(wildcard $(SRCDIR)/*.c)
SOURCES := $(filter-out $(SRCDIR)/evloop_epoll.c, $(SOURCES))
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

# Plugins
PLUGIN_HELLO_SRC = $(PLUGINDIR)/hello/hello.c
PLUGIN_HELLO_SO = $(BUILDDIR)/hello.so

# Tests
TEST_MODULES = $(wildcard $(TESTDIR)/*_unity.c)
TEST_MODULE_OBJECTS = $(TEST_MODULES:$(TESTDIR)/%.c=$(BUILDDIR)/tests/%.o)
TEST_RUNNER_SRC = $(TESTDIR)/test_runner.c
TEST_RUNNER_OBJ = $(BUILDDIR)/tests/test_runner.o
UNITY_SRC = $(TESTDIR)/unity/unity.c
UNITY_OBJ = $(BUILDDIR)/tests/unity.o
TEST_TARGET = $(BUILDDIR)/run_tests

# Default
all: $(TARGET) plugins

# Ensure directories exist (parallel-safe)
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Separate subdir for test objects (avoids pattern-rule conflicts)
$(BUILDDIR)/tests:
	mkdir -p $(BUILDDIR)/tests

# Main executable
$(TARGET): $(BUILDDIR) $(BINDIR) $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Object files from sources
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Object files from tests (compile test code with sanitizers)
$(BUILDDIR)/tests/%.o: $(TESTDIR)/%.c | $(BUILDDIR)/tests
	$(CC) $(CFLAGS) $(SAN_CFLAGS) -I$(TESTDIR) -I$(TESTDIR)/unity -c $< -o $@

# Plugins
plugins: $(PLUGIN_HELLO_SO)

$(PLUGIN_HELLO_SO): $(PLUGIN_HELLO_SRC) $(BUILDDIR)
	$(CC) $(CFLAGS) -fPIC -shared $(PLUGIN_HELLO_SRC) -o $(PLUGIN_HELLO_SO)

# Tests (Unity)
tests: $(TEST_TARGET)

$(UNITY_OBJ): $(UNITY_SRC) $(BUILDDIR)/tests
	$(CC) $(CFLAGS) $(SAN_CFLAGS) -c $(UNITY_SRC) -o $(UNITY_OBJ)

$(TEST_TARGET): $(UNITY_OBJ) $(TEST_RUNNER_OBJ) $(TEST_MODULE_OBJECTS) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS))
	$(CC) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS)) $(UNITY_OBJ) $(TEST_RUNNER_OBJ) $(TEST_MODULE_OBJECTS) -o $(TEST_TARGET) $(LDFLAGS) $(SAN_LDFLAGS)

# Run the shell
run: $(TARGET)
	ASAN_OPTIONS=$(ASAN_STRICT_OPTS) ./$(TARGET)

# Run full test suite
test: tests
	@if [ -f $(TEST_TARGET) ]; then \
		echo "Running Unity test suite..."; \
		ASAN_OPTIONS=$(ASAN_STRICT_OPTS) $(TEST_TARGET); \
	else \
		echo "No tests found. Create test files in $(TESTDIR)/ with _unity.c suffix to enable testing."; \
	fi

# Run individual test modules (link with sanitizers)
test-lexer: $(BUILDDIR)/tests/test_lexer_unity.o $(UNITY_OBJ) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS))
	$(CC) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS)) $(UNITY_OBJ) $(BUILDDIR)/tests/test_lexer_unity.o -o $(BUILDDIR)/test_lexer $(LDFLAGS) $(SAN_LDFLAGS)
	ASAN_OPTIONS=$(ASAN_STRICT_OPTS) $(BUILDDIR)/test_lexer

test-parser: $(BUILDDIR)/tests/test_parser_unity.o $(UNITY_OBJ) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS))
	$(CC) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS)) $(UNITY_OBJ) $(BUILDDIR)/tests/test_parser_unity.o -o $(BUILDDIR)/test_parser $(LDFLAGS) $(SAN_LDFLAGS)
	ASAN_OPTIONS=$(ASAN_STRICT_OPTS) $(BUILDDIR)/test_parser

test-env: $(BUILDDIR)/tests/test_env_unity.o $(UNITY_OBJ) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS))
	$(CC) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS)) $(UNITY_OBJ) $(BUILDDIR)/tests/test_env_unity.o -o $(BUILDDIR)/test_env $(LDFLAGS) $(SAN_LDFLAGS)
	ASAN_OPTIONS=$(ASAN_STRICT_OPTS) $(BUILDDIR)/test_env

test-util: $(BUILDDIR)/tests/test_util_unity.o $(UNITY_OBJ) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS))
	$(CC) $(filter-out $(BUILDDIR)/main.o, $(OBJECTS)) $(UNITY_OBJ) $(BUILDDIR)/tests/test_util_unity.o -o $(BUILDDIR)/test_util $(LDFLAGS) $(SAN_LDFLAGS)
	ASAN_OPTIONS=$(ASAN_STRICT_OPTS) $(BUILDDIR)/test_util

# Debug build (includes sanitizers for deep diagnostics)
debug: CFLAGS += -DDEBUG $(SAN_CFLAGS)
debug: LDFLAGS += $(SAN_LDFLAGS)
debug:
	$(MAKE) clean
	$(MAKE) $(TARGET)
	@echo "Built debug with sanitizers ($(SANITIZERS))."

# Single hardened release build (alternative to debug)
release: CFLAGS := $(filter-out -g,$(CFLAGS)) $(HARDEN_CFLAGS)
release: LDFLAGS := $(LDFLAGS) $(HARDEN_LDFLAGS)
release:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" $(TARGET)
	@echo "Stripping symbols for release..." 
	@$(STRIP) $(TARGET) || true
	@echo "Built hardened release: $(TARGET)"

# Install / Uninstall
install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin/"
	@sudo cp $(TARGET) /usr/local/bin/
	@sudo chmod +x /usr/local/bin/$(TARGET)
	@echo "Installation complete"

uninstall:
	@echo "Removing $(TARGET) from /usr/local/bin/"
	@sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstall complete"

# Clean
clean:
	rm -rf $(BUILDDIR)
	rm -rf $(BINDIR)

distclean: clean
	find . -name "*~" -delete
	find . -name "*.bak" -delete
	find . -name ".*.swp" -delete

# Project status
status:
	@echo "Project Status:"
	@echo "==============="
	@echo "Source files: $(words $(SOURCES))"
	@echo "Header files: $(words $(wildcard $(INCDIR)/*.h))"
	@echo "Plugin files: $(words $(wildcard $(PLUGINDIR)/*/*.c))"
	@echo "Test modules: $(words $(TEST_MODULES))"
	@echo "Unity tests: $(if $(wildcard $(TESTDIR)/unity/unity.c),1,0)"
	@echo ""
	@echo "C source files (built): $(words $(SOURCES))"
	@echo "Header files: $(words $(wildcard $(INCDIR)/*.h))"
	@echo "Unity test modules: $(words $(TEST_MODULES))"
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

# Basic checks
check:
	@echo "Running basic checks..."
	@echo "Checking for missing includes..."
	@grep -n "#include" $(SRCDIR)/*.c | grep -v "$(INCDIR)" | grep -v "<" || echo "  ✓ All includes look good"
	@echo "Checking for TODO comments..."
	@grep -n "TODO\|FIXME\|XXX" $(SRCDIR)/*.c $(INCDIR)/*.h || echo "  ✓ No TODO comments found"
	@echo "Checking for potential memory leaks (basic)..."
	@grep -n "malloc\|calloc\|strdup" $(SRCDIR)/*.c | wc -l | xargs echo "  malloc calls:"
	@grep -n "free" $(SRCDIR)/*.c | wc -l | xargs echo "  free calls:"

# Dependency tracking
-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.d: $(SRCDIR)/%.c | $(BUILDDIR)
	@$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< > $@

# Valgrind memory check
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
		  echo "- Use 'make debug' and run with gdb or sanitizers"; \
		  echo "- Try AddressSanitizer: make clean && make debug && ./bin/myshell"; \
		  false)); \
	else \
		echo "Valgrind not found. Install valgrind to run memory checks."; \
	fi

# Formatting and docs
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "Formatting code with clang-format..."; \
		find $(SRCDIR) $(INCDIR) $(TESTDIR) $(PLUGINDIR) \( -name "*.c" -o -name "*.h" \) -print0 | xargs -0 clang-format -i; \
	else \
		echo "clang-format not found. Skipping clang-format step."; \
	fi
	@echo "Normalizing blank lines (removing whitespace-only lines)..."; \
	find $(SRCDIR) $(INCDIR) $(TESTDIR) $(PLUGINDIR) -type f \( -name "*.c" -o -name "*.h" \) -print0 \
		| xargs -0 sed -i -E 's/^[[:space:]]+$$//'; \
	if [ -f README.md ]; then sed -i -E 's/^[[:space:]]+$$//' README.md; fi; \
	echo "Whitespace cleanup complete"

docs:
	@if command -v doxygen >/dev/null 2>&1; then \
		echo "Generating Doxygen documentation..."; \
		doxygen Doxyfile; \
		echo "Docs generated in docs/html"; \
	else \
		echo "Doxygen not found. Install doxygen to generate docs."; \
	fi

# Help
help:
	@echo "MyShell Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all         - Build the shell and plugins (default)"
	@echo "  $(TARGET)   - Build the main shell executable"
	@echo "  plugins     - Build all plugins"
	@echo "  tests       - Build test suite"
	@echo "  run         - Run the shell"
	@echo "  test        - Build and run Unity test suite"
	@echo "  test-lexer  - Run lexer tests only"
	@echo "  test-parser - Run parser tests only"
	@echo "  test-env    - Run environment tests only"
	@echo "  test-util   - Run utility tests only"
	@echo "  debug       - Build with sanitizers ($(SANITIZERS)) and debug symbols"
	@echo "  release     - Build hardened release (PIE, RELRO, SSP, FORTIFY, LTO, stripped)"
	@echo "  install     - Install shell to /usr/local/bin (requires sudo)"
	@echo "  uninstall   - Remove shell from /usr/local/bin (requires sudo)"
	@echo "  clean       - Remove build files"
	@echo "  distclean   - Remove build files and temporary files"
	@echo "  status      - Show project status"
	@echo "  check       - Basic repository checks"
	@echo "  memcheck    - Run valgrind memory check"
	@echo "  format      - Format code with clang-format and normalize whitespace"
	@echo "  docs        - Generate Doxygen documentation"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Environment variables:"
	@echo "  CC            - C compiler (default: gcc)"
	@echo "  CFLAGS        - Additional compiler flags"
	@echo "  LDFLAGS       - Additional linker flags"
	@echo "  SANITIZERS    - Comma-separated list for -fsanitize= (default: address,undefined)"
	@echo "  FORTIFY_LEVEL - _FORTIFY_SOURCE level for release (default: 2)"
	@echo "  ENABLE_LTO    - Set to 0 to disable LTO in release (default: 1)"

.PHONY: all clean distclean run test debug release install uninstall plugins tests help status check memcheck format docs
