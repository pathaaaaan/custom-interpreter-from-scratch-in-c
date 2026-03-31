#ifndef PARSER_H
#define PARSER_H

#include "common.h"

Node *parse_tokens(const TokenArray *tokens);
void free_ast(Node *node);

#endif
