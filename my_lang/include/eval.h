#ifndef EVAL_H
#define EVAL_H

#include "common.h"
#include "value.h"
#include "table.h"

Value eval(Node *node, Environment *env, ObjectTracker *tracker);
void eval_program(Node *program, Environment *env, ObjectTracker *tracker);
void print_ast(Node *node, int depth);

#endif
