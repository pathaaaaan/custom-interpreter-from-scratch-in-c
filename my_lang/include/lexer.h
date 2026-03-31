#ifndef LEXER_H
#define LEXER_H

#include "common.h"

TokenArray lex_source(const char *source);
void free_tokens(TokenArray *tokens);

#endif
