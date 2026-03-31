CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -Iinclude
SRC = src/main.c src/lexer.c src/parser.c src/eval.c src/table.c
TARGET = litelogic

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) tests/test.ll

clean:
	rm -f $(TARGET)

.PHONY: all run clean
