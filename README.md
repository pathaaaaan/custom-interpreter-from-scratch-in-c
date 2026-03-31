# LiteLogic

LiteLogic is a lightweight tree-walking interpreter written in C99. It supports integers, booleans encoded as `0` and `1`, arithmetic with precedence, variables, comparisons, logical `AND`/`OR`, `if/else`, `print(...)`, file execution, and a small REPL.

## Project Layout

```text
my_lang/
├── src/
│   ├── main.c
│   ├── lexer.c
│   ├── parser.c
│   ├── eval.c
│   └── table.c
├── include/
│   ├── common.h
│   ├── lexer.h
│   ├── parser.h
│   ├── eval.h
│   └── table.h
├── tests/
│   └── test.ll
├── Makefile
└── README.md
```

## Build

```sh
cd my_lang
make
```

## Run

Execute a source file:

```sh
./litelogic tests/test.ll
```

Start the REPL:

```sh
./litelogic
```

Type `exit` or `quit` to leave the REPL.

## Detailed Usage Guide

For a fuller explanation of the interpreter workflow, testing options, and example programs, see:

- [`my_lang/USAGE.md`](my_lang/USAGE.md)

## Language Examples

```ll
x = 10;
y = x * 2;
print(y);

if ((x > 5) AND (y < 30)) {
    print(1);
} else {
    print(0);
}
```

## Implementation Notes

- Lexer returns a dynamic token array.
- Parser uses recursive descent and builds an AST.
- Evaluator walks the tree against a persistent symbol table.
- AST nodes and token data are freed after execution.
