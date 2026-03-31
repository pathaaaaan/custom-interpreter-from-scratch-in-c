#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>

typedef struct {
    char *name;
    int value;
} Symbol;

typedef struct {
    Symbol *items;
    size_t count;
    size_t capacity;
} SymbolTable;

void init_table(SymbolTable *table);
void free_table(SymbolTable *table);
void set_variable(SymbolTable *table, const char *name, int value);
int get_variable(const SymbolTable *table, const char *name, int *out_value);

#endif
