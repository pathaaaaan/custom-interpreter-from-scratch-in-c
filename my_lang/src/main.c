#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/eval.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/table.h"

static char *read_file(const char *path) {
    FILE *file;
    long length;
    size_t read_count;
    char *buffer;

    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", path);
        exit(EXIT_FAILURE);
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        fprintf(stderr, "Unable to seek file: %s\n", path);
        exit(EXIT_FAILURE);
    }

    length = ftell(file);
    if (length < 0) {
        fclose(file);
        fprintf(stderr, "Unable to read file size: %s\n", path);
        exit(EXIT_FAILURE);
    }

    rewind(file);
    buffer = (char *)malloc((size_t)length + 1);
    if (buffer == NULL) {
        fclose(file);
        fprintf(stderr, "Unable to allocate file buffer.\n");
        exit(EXIT_FAILURE);
    }

    read_count = fread(buffer, 1, (size_t)length, file);
    buffer[read_count] = '\0';
    fclose(file);
    return buffer;
}

static void run_source(const char *source, SymbolTable *table) {
    TokenArray tokens;
    Node *program;

    tokens = lex_source(source);
    program = parse_tokens(&tokens);
    eval_program(program, table);
    free_ast(program);
    free_tokens(&tokens);
}

static void append_text(char **buffer, size_t *length, size_t *capacity, const char *text) {
    size_t text_len;
    size_t required;
    char *resized;

    text_len = strlen(text);
    required = *length + text_len + 1;
    if (required > *capacity) {
        size_t new_capacity = *capacity == 0 ? 128 : *capacity;
        while (new_capacity < required) {
            new_capacity *= 2;
        }
        resized = (char *)realloc(*buffer, new_capacity);
        if (resized == NULL) {
            fprintf(stderr, "Unable to grow REPL buffer.\n");
            free(*buffer);
            exit(EXIT_FAILURE);
        }
        *buffer = resized;
        *capacity = new_capacity;
    }

    memcpy(*buffer + *length, text, text_len + 1);
    *length += text_len;
}

static int should_continue_input(const char *buffer) {
    int braces = 0;
    int parens = 0;
    size_t i;
    char last = '\0';

    for (i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] == '{') {
            braces++;
        } else if (buffer[i] == '}') {
            braces--;
        } else if (buffer[i] == '(') {
            parens++;
        } else if (buffer[i] == ')') {
            parens--;
        }

        if (buffer[i] != ' ' && buffer[i] != '\t' && buffer[i] != '\n' && buffer[i] != '\r') {
            last = buffer[i];
        }
    }

    if (braces > 0 || parens > 0) {
        return 1;
    }

    if (last == '{') {
        return 1;
    }

    if (last == '\0') {
        return 1;
    }

    return last != ';' && last != '}';
}

static void repl(void) {
    SymbolTable table;
    char line[1024];
    char *buffer = NULL;
    size_t length = 0;
    size_t capacity = 0;

    init_table(&table);

    while (1) {
        printf("%s", length == 0 ? ">>> " : "... ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }

        if (length == 0 &&
            (strcmp(line, "exit\n") == 0 || strcmp(line, "quit\n") == 0)) {
            break;
        }

        append_text(&buffer, &length, &capacity, line);
        if (should_continue_input(buffer)) {
            continue;
        }

        run_source(buffer, &table);
        buffer[0] = '\0';
        length = 0;
    }

    free(buffer);
    free_table(&table);
}

int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [file]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 2) {
        SymbolTable table;
        char *source = read_file(argv[1]);

        init_table(&table);
        run_source(source, &table);
        free(source);
        free_table(&table);
        return EXIT_SUCCESS;
    }

    repl();
    return EXIT_SUCCESS;
}
