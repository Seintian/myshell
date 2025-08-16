# MyShell Build and Usage Guide

## Prerequisites

- GCC compiler with C99 support
- GNU Make
- Standard POSIX development libraries
- `libdl` for dynamic loading (usually included with glibc)

## Building the Project

### Quick Start

```bash
# Build everything (shell + plugins)
make all

# Or just build the shell
make myshell

# Build with debug symbols
make debug

# Build optimized release version
make release
```

### Available Make Targets

- `make all` - Build the shell and all plugins (default)
- `make myshell` - Build the main shell executable
- `make plugins` - Build all plugins
- `make run` - Build and run the shell interactively
- `make test` - Build and run the test suite
- `make clean` - Remove build files
- `make help` - Show all available targets

## Running the Shell

### Interactive Mode

```bash
./myshell
```

### Testing Commands

```bash
# Test basic commands
echo -e "pwd\nls\nexit" | ./myshell

# Test built-in commands
echo -e "export TEST=hello\necho \$TEST\nexit" | ./myshell
```

## Project Structure

```text
myshell/
├── Makefile              # Build configuration
├── README.md            # This file
├── build/               # Compiled objects and binaries
├── include/             # Header files
│   ├── shell.h         # Main shell interface
│   ├── lexer.h         # Tokenization
│   ├── parser.h        # Command parsing
│   ├── exec.h          # Command execution
│   ├── ast.h           # Abstract syntax tree
│   ├── jobs.h          # Job control
│   ├── builtin.h       # Built-in commands
│   ├── plugin.h        # Plugin system
│   ├── env.h           # Environment variables
│   ├── term.h          # Terminal control
│   ├── evloop.h        # Event loop abstraction
│   ├── redir.h         # I/O redirection
│   └── util.h          # Utility functions
├── src/                 # Source files
│   ├── main.c          # Entry point
│   ├── shell.c         # Main shell loop
│   ├── lexer.c         # Tokenizer implementation
│   ├── parser.c        # Parser implementation
│   ├── exec.c          # Command executor
│   ├── expand.c        # Variable expansion
│   ├── pipeline.c      # Pipeline handling
│   ├── jobs.c          # Job control
│   ├── builtin_core.c  # Core built-in commands
│   ├── plugin.c        # Plugin system
│   ├── term.c          # Terminal management
│   ├── redir.c         # I/O redirection
│   ├── evloop_select.c # Select-based event loop
│   ├── evloop_epoll.c  # Linux epoll event loop
│   └── util.c          # Utility functions
├── plugins/             # Plugin modules
│   └── hello/          # Example plugin
│       └── hello.c     # Hello world plugin
└── tests/               # Test files
    └── test_lexer.c    # Example test
```

## Features Implemented

### Core Shell Features

- ✅ Command line parsing and tokenization
- ✅ Basic command execution
- ✅ Built-in commands (cd, pwd, exit, export, unset, jobs, fg, bg, type)
- ✅ Environment variable support
- ✅ Job control framework
- ✅ Plugin system for extensible commands
- ✅ Pipeline support (framework)
- ✅ I/O redirection (framework)

### Built-in Commands

- `cd [directory]` - Change directory
- `pwd` - Print working directory
- `exit [code]` - Exit the shell
- `export VAR=value` - Set environment variables
- `unset VAR` - Unset environment variables
- `jobs` - List active jobs
- `fg [job]` - Bring job to foreground
- `bg [job]` - Put job in background
- `type command` - Show command type

### Plugin System

- Dynamic loading of shared library plugins
- Example "hello" plugin included
- Plugin API for adding custom commands

## Building Plugins

Plugins are shared libraries that implement the plugin API:

```c
#include "plugin.h"

static int my_execute(int argc, char **argv) {
    // Your command implementation
    return 0;
}

static plugin_info_t plugin_info = {
    .name = "mycmd",
    .version = "1.0.0", 
    .description = "My custom command",
    .execute = my_execute
};

plugin_info_t *get_plugin_info(void) {
    return &plugin_info;
}
```

Build plugins with:

```bash
gcc -fPIC -shared -Iinclude mycmd.c -o mycmd.so
```

## Development

### Adding Features

1. Add function declarations to appropriate header files in `include/`
2. Implement functions in corresponding source files in `src/`
3. Update Makefile if adding new source files
4. Write tests in `tests/` directory

### Testing

```bash
# Create test files
make create-test

# Run tests
make test

# Memory checking (if valgrind installed)
make memcheck
```

### Code Quality

```bash
# Check for common issues
make check

# Format code (if clang-format installed)
make format
```

## Installation

```bash
# Install to /usr/local/bin (requires sudo)
make install

# Uninstall
make uninstall
```

## Troubleshooting

### Build Issues

- Ensure GCC and Make are installed
- Check that all required development libraries are available
- Use `make clean` before rebuilding if you encounter linking issues

### Runtime Issues

- Check file permissions on the executable
- Ensure plugins are in the correct location
- Use debug build for more verbose output: `make debug`

### Environment

Set `SHELL_DEBUG=1` to enable debug output:

```bash
SHELL_DEBUG=1 ./myshell
```

## Contributing

1. Follow the existing code style
2. Add tests for new features
3. Update documentation
4. Ensure `make check` passes
5. Test thoroughly before submitting changes

## License

This project is provided as-is for educational purposes.
