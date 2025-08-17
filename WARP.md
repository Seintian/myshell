# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Quick Development Commands

### Building

```bash
make all          # Build shell and all plugins (default)
make myshell      # Build main shell executable only
make plugins      # Build plugins only
make debug        # Debug build with symbols
make release      # Optimized release build
```

### Testing

```bash
make test         # Run full Unity test suite
make test-lexer   # Run lexer tests only
make test-parser  # Run parser tests only
make test-env     # Run environment variable tests only
make test-util    # Run utility function tests only
```

### Running and Development

```bash
make run          # Build and run shell interactively
./bin/myshell     # Run built shell directly
make clean        # Clean build artifacts
make help         # Show all available targets
```

### Code Quality and Analysis

```bash
make check        # Run basic code checks
make memcheck     # Run valgrind memory analysis
make asan         # Build with AddressSanitizer
make format       # Format code with clang-format
```

### Plugin Development

```bash
# Build custom plugin
gcc -fPIC -shared -Iinclude mycmd.c -o mycmd.so

# Example plugin structure (see plugins/hello/hello.c):
static plugin_info_t plugin_info = {
    .name = "mycmd",
    .execute = my_execute
};
```

## Architecture Overview

### Pipeline Flow

The shell follows a classical interpreter pipeline:

1. **Lexer** (`lexer.c`) - Tokenizes input into TOKEN_WORD, TOKEN_PIPE, etc.
2. **Parser** (`parser.c`) - Builds AST from token stream
3. **Executor** (`exec.c`) - Executes AST nodes (commands, pipelines, sequences)

### Core Components

**AST System**

- `ast.h` defines node types: AST_COMMAND, AST_PIPELINE, AST_SEQUENCE, AST_BACKGROUND
- AST nodes are opaque handles with type-specific operations
- Memory management handled through `ast_free()`

**Execution Model**

- Commands can be built-ins, external programs, or plugins
- Pipeline execution supports chaining commands with pipes
- Job control framework for background processes

**Plugin Architecture**
- Dynamic loading via `dlopen()` from shared libraries
- Plugin API defined in `plugin.h` with standardized interface
- Plugins register via `get_plugin_info()` function export

**Built-in Commands**
Core built-ins in `builtin_core.c`: cd, pwd, exit, export, unset, jobs, fg, bg, type

### Key Data Structures

**Lexer Token Flow**

```txt
Input string → lexer_create() → lexer_next_token() → token_t stream
```

**Parser AST Construction**

```txt
token_t stream → parser_create() → parser_parse() → ast_node_t tree
```

**Execution Context**

```txt
ast_node_t → exec_ast() → command resolution → process creation/built-in execution
```

### Memory Management Patterns

- Consistent create/free pairing for all components
- AST nodes manage their own child cleanup
- Plugins handle their own lifecycle through init/cleanup callbacks
- Token values are heap-allocated strings requiring explicit cleanup

### Event System

- Two event loop implementations: `evloop_select.c` (default) and `evloop_epoll.c` (Linux)
- Abstracted through `evloop.h` interface
- Used for job control and signal handling

### Testing Strategy

Uses Unity framework with comprehensive coverage:

- **24 tests** across lexer, parser, environment, and utilities
- Individual module testing capability
- Memory leak detection integration with valgrind
- Test-driven development patterns with `setUp()/tearDown()` hooks

### File Organization

- `include/` - Public APIs and type definitions
- `src/` - Core implementation files
- `plugins/` - Dynamically loadable command extensions
- `tests/` - Unity-based test suite with module separation
- `build/` - Generated object files and binaries (created during build)
