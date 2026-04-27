#ifndef VALUE_H
#define VALUE_H

#include "common.h"

typedef enum {
    VAL_INT,
    VAL_FLOAT,
    VAL_BOOL,
    VAL_CHAR,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_NIL,
    VAL_RETURN_FLAG // Used internally to signal that a function has returned
} ValueType;

typedef struct Value {
    ValueType type;
    union {
        int integer;
        double float_val;
        int boolean;
        char character;
        char *string;
        struct {
            char **params;
            int param_count;
            struct Node *body;
        } func;
    } as;
} Value;

// For tracking heap-allocated objects like strings
typedef struct ObjectNode {
    void *ptr; 
    struct ObjectNode *next;
} ObjectNode;

typedef struct {
    ObjectNode *head;
} ObjectTracker;

void init_tracker(ObjectTracker *tracker);
void track_object(ObjectTracker *tracker, void *ptr);
void free_tracker(ObjectTracker *tracker);

void print_value(Value val);
Value val_nil();

#endif
