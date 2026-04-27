#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>
#include "value.h"

typedef struct {
    char *name;
    Value value;
} Symbol;

typedef struct Environment {
    Symbol *items;
    size_t count;
    size_t capacity;
    struct Environment *enclosing;
} Environment;

void init_env(Environment *env, Environment *enclosing);
void free_env(Environment *env);
void set_variable(Environment *env, const char *name, Value value);
int get_variable(const Environment *env, const char *name, Value *out_value);

#endif
