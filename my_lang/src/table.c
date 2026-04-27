#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/common.h"
#include "../include/table.h"

void init_tracker(ObjectTracker *tracker) {
    tracker->head = NULL;
}

void track_object(ObjectTracker *tracker, void *ptr) {
    if (!tracker || !ptr) return;
    ObjectNode *node = (ObjectNode *)malloc(sizeof(ObjectNode));
    node->ptr = ptr;
    node->next = tracker->head;
    tracker->head = node;
}

void free_tracker(ObjectTracker *tracker) {
    ObjectNode *current = tracker->head;
    while (current != NULL) {
        ObjectNode *next = current->next;
        free(current->ptr);  // Free the actual pointer (e.g. string)
        free(current);
        current = next;
    }
    tracker->head = NULL;
}

Value val_nil() {
    Value v;
    v.type = VAL_NIL;
    return v;
}

void print_value(Value val) {
    switch (val.type) {
        case VAL_INT:
            printf("%d", val.as.integer);
            break;
        case VAL_FLOAT:
            printf("%g", val.as.float_val);
            break;
        case VAL_BOOL:
            printf(val.as.boolean ? "true" : "false");
            break;
        case VAL_CHAR:
            printf("'%c'", val.as.character);
            break;
        case VAL_STRING:
            printf("\"%s\"", val.as.string);
            break;
        case VAL_FUNCTION:
            printf("<fn>");
            break;
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_RETURN_FLAG:
            printf("<return flag>");
            break;
    }
}

static void env_error(const char *message) {
    fprintf(stderr, "Environment error: %s\n", message);
    exit(EXIT_FAILURE);
}

void init_env(Environment *env, Environment *enclosing) {
    env->items = NULL;
    env->count = 0;
    env->capacity = 0;
    env->enclosing = enclosing;
}

void free_env(Environment *env) {
    size_t i;
    for (i = 0; i < env->count; i++) {
        free(env->items[i].name);
    }
    free(env->items);
    env->items = NULL;
    env->count = 0;
    env->capacity = 0;
}

static void ensure_capacity(Environment *env) {
    size_t new_capacity;
    Symbol *resized;
    if (env->count < env->capacity) {
        return;
    }
    new_capacity = env->capacity == 0 ? 16 : env->capacity * 2;
    resized = (Symbol *)realloc(env->items, new_capacity * sizeof(Symbol));
    if (resized == NULL) {
        env_error("Unable to grow environment table.");
    }
    env->items = resized;
    env->capacity = new_capacity;
}

void set_variable(Environment *env, const char *name, Value value) {
    size_t i;
    // Walk up the chain to see if it's already defined
    Environment *current = env;
    while (current != NULL) {
        for (i = 0; i < current->count; i++) {
            if (strcmp(current->items[i].name, name) == 0) {
                current->items[i].value = value;
                return;
            }
        }
        current = current->enclosing;
    }

    // Not found, declare in the current (local) environment
    ensure_capacity(env);
    env->items[env->count].name = ll_strdup(name);
    env->items[env->count].value = value;
    env->count++;
}

int get_variable(const Environment *env, const char *name, Value *out_value) {
    size_t i;
    while (env != NULL) {
        for (i = 0; i < env->count; i++) {
            if (strcmp(env->items[i].name, name) == 0) {
                *out_value = env->items[i].value;
                return 1;
            }
        }
        env = env->enclosing;
    }
    return 0;
}
