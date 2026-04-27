#ifndef LEXER_H
#define LEXER_H

#include "common.h"

TokenArray lex_source(const char *source);  // takes the source code and returns an array of tokens
void free_tokens(TokenArray *tokens);

#endif
