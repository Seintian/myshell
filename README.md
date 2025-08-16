# My Shell

```tree
shell/
    include/
        shell.h              // public entry points
        ast.h                // (public) AST nodes (typedefs + opaque handles)
        jobs.h               // job control API
        lexer.h              // token stream API
        parser.h             // parse API
        exec.h               // executor API
        redir.h              // redirection helpers
        builtin.h            // builtin registry
        plugin.h             // dynamic cmd ABI
        env.h                // env/vars API
        term.h               // terminal control
        evloop.h             // select()/epoll() abstraction
        util.h
    src/
        main.c
        shell.c
        lexer.c
        parser.c
        expand.c
        exec.c
        pipeline.c
        redir.c
        jobs.c
        term.c
        env.c
        builtin_core.c       // cd, exit, export, unset, pwd, jobs, fg, bg, type
        plugin.c
        evloop_select.c      // default
        evloop_epoll.c       // optional Linux impl
        util.c
    plugins/
        hello/hello.c        // example plugin command
    tests/
        ...
```
