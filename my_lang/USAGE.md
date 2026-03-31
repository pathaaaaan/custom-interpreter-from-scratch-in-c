# LiteLogic Usage Guide

This guide explains how to build, run, and test the interpreter in `my_lang/`.

## 1. Working flow of the interpreter

You do **not** run `main.c`, `lexer.c`, `parser.c`, and `eval.c` separately.
They work together as one program:

1. `make` compiles all C source files into a single executable named `litelogic`
2. `main.c` reads your source code from a file or from the REPL
3. `lexer.c` converts the source text into tokens
4. `parser.c` turns those tokens into an AST (abstract syntax tree)
5. `eval.c` walks the AST and executes it
6. `table.c` stores variables in the symbol table

So the normal workflow is:

```sh
cd my_lang
make
./litelogic tests/test.ll
```

---

## 2. How to test it

### Option A: Run the existing sample test

From the `my_lang` folder:

```sh
make
./litelogic tests/test.ll
```

Expected output:

```text
20
```

You can also use the Makefile shortcut:

```sh
make run
```

### Option B: Test your own `.ll` file

Create a new LiteLogic program, for example `tests/arithmetic.ll`:

```ll
x = 8;
y = x + 4;
print(y);
```

Run it with:

```sh
./litelogic tests/arithmetic.ll
```

Expected output:

```text
12
```

### Option C: Use the REPL for quick testing

Start interactive mode:

```sh
./litelogic
```

Then type code like this:

```ll
x = 7;
y = x * 3;
print(y);
```

Output:

```text
21
```

Type `quit` or `exit` to leave the REPL.

---

## 3. Can you test individual files?

### The C source files (`main.c`, `lexer.c`, `parser.c`, `eval.c`, `table.c`)

- **Not directly as separate runnable programs**
- They are modules of the same interpreter
- You compile them **all together** using `make`

So for normal usage, you should **not** try to execute each C file on its own.

### The LiteLogic source files (`.ll` files)

- **Yes, these can be tested individually**
- Each `.ll` file is a separate input program for the interpreter
- You run them one at a time like this:

```sh
./litelogic path/to/file.ll
```

That is the best way to test features one by one.

---

## 4. Recommended testing workflow

A simple workflow for development is:

1. Build the interpreter
   ```sh
   make
   ```
2. Run the built-in sample
   ```sh
   ./litelogic tests/test.ll
   ```
3. Create small `.ll` files for each feature you want to check
4. Run each file individually through `./litelogic`
5. Use the REPL when you want very quick experiments

Example feature-by-feature testing:

- arithmetic test → `tests/arithmetic.ll`
- comparison test → `tests/compare.ll`
- logic test → `tests/logic.ll`
- `if/else` test → `tests/if_else.ll`

---

## 5. Supported language features

LiteLogic currently supports:

- integer values
- variables and assignment
- arithmetic: `+`, `-`, `*`, `/`
- comparisons: `==`, `!=`, `<`, `>`
- logical operators: `AND`, `OR`
- `if / else`
- `print(...)`
- file execution
- REPL mode

---

## 6. Proper examples

### Example 1: Arithmetic and variables

```ll
x = 10;
y = x * 2 + 5;
print(y);
```

Output:

```text
25
```

### Example 2: Comparison with `if / else`

```ll
score = 18;

if (score > 10) {
    print(1);
} else {
    print(0);
}
```

Output:

```text
1
```

### Example 3: Logical operators

```ll
x = 8;
y = 12;

if ((x < 10) AND (y > 10)) {
    print(1);
} else {
    print(0);
}
```

Output:

```text
1
```

### Example 4: Equality and inequality

```ll
x = 5;
print(x == 5);
print(x != 2);
```

Output:

```text
1
1
```

---

## 7. Important syntax rules

Keep these in mind while testing:

- every statement should end with `;`
- `if` and `else` blocks must use `{}`
- `AND` and `OR` are uppercase in this implementation
- only integers are supported right now
- using an undefined variable will cause an evaluation error
- dividing by zero will cause an evaluation error

Example of valid syntax:

```ll
x = 4;
if (x > 2) {
    print(x);
} else {
    print(0);
}
```

---

## 8. Fast command reference

```sh
# build
cd my_lang
make

# run sample file
./litelogic tests/test.ll

# run your own file
./litelogic tests/arithmetic.ll

# interactive mode
./litelogic

# clean build output
make clean
```
