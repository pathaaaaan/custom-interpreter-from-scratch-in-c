#ifndef EVAL_H
#define EVAL_H

#include "common.h"
#include "table.h"

int eval(Node *node, SymbolTable *table);
void eval_program(Node *program, SymbolTable *table);

#endif
