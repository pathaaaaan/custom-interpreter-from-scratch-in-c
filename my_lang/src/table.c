#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/common.h"
#include "../include/table.h"

static void table_error(const char *message) {
    fprintf(stderr, "Symbol table error: %s\n", message);
    exit(EXIT_FAILURE);
}

void init_table(SymbolTable *table) {
    table->items = NULL;
    table->count = 0;
    table->capacity = 0;
}

void free_table(SymbolTable *table) {
    size_t i;

    for (i = 0; i < table->count; i++) {
        free(table->items[i].name);
    }

    free(table->items);
    table->items = NULL;
    table->count = 0;
    table->capacity = 0;
}

static void ensure_capacity(SymbolTable *table) {
    size_t new_capacity;
    Symbol *resized;

    if (table->count < table->capacity) {
        return;
    }

    new_capacity = table->capacity == 0 ? 16 : table->capacity * 2;
    resized = (Symbol *)realloc(table->items, new_capacity * sizeof(Symbol));
    if (resized == NULL) {
        table_error("Unable to grow symbol table.");
    }

    table->items = resized;
    table->capacity = new_capacity;
}

void set_variable(SymbolTable *table, const char *name, int value) {
    size_t i;

    for (i = 0; i < table->count; i++) {
        if (strcmp(table->items[i].name, name) == 0) {
            table->items[i].value = value;
            return;
        }
    }

    ensure_capacity(table);
    table->items[table->count].name = ll_strdup(name);
    table->items[table->count].value = value;
    table->count++;
}

int get_variable(const SymbolTable *table, const char *name, int *out_value) {
    size_t i;

    for (i = 0; i < table->count; i++) {
        if (strcmp(table->items[i].name, name) == 0) {
            *out_value = table->items[i].value;
            return 1;
        }
    }

    return 0;
}
