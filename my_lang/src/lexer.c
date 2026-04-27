#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/lexer.h"

char *ll_strdup(const char *src) {
    if (src == NULL) return NULL;
    size_t len = strlen(src);
    char *copy = (char *)malloc(len + 1);
    memcpy(copy, src, len + 1);
    return copy;
}

static void lexer_error(int line, int col, const char *message) {
    fprintf(stderr, "Lexer error at line %d, col %d: %s\n", line, col, message);
    exit(EXIT_FAILURE);
}

static void ensure_capacity(TokenArray *tokens) {
    size_t new_capacity;
    Token *resized;
    if (tokens->count < tokens->capacity) {
        return;
    }
    new_capacity = tokens->capacity == 0 ? 16 : tokens->capacity * 2;
    resized = (Token *)realloc(tokens->items, new_capacity * sizeof(Token));
    tokens->items = resized;
    tokens->capacity = new_capacity;
}

static void push_token(TokenArray *tokens, TokenType type, int line, int col, int i_val, double f_val, char c_val, const char *name) {
    Token token;
    ensure_capacity(tokens);
    token.type = type;
    token.line = line;
    token.col = col;
    token.int_val = i_val;
    token.float_val = f_val;
    token.char_val = c_val;
    token.name = name != NULL ? ll_strdup(name) : NULL;
    tokens->items[tokens->count++] = token;
}

static TokenType keyword_type(const char *text) {
    if (strcmp(text, "if") == 0) return TOKEN_IF;
    if (strcmp(text, "else") == 0) return TOKEN_ELSE;
    if (strcmp(text, "print") == 0) return TOKEN_PRINT;
    if (strcmp(text, "while") == 0) return TOKEN_WHILE;
    if (strcmp(text, "for") == 0) return TOKEN_FOR;
    if (strcmp(text, "def") == 0) return TOKEN_DEF;
    if (strcmp(text, "return") == 0) return TOKEN_RETURN;
    if (strcmp(text, "len") == 0) return TOKEN_LEN;
    if (strcmp(text, "str") == 0) return TOKEN_STR;
    if (strcmp(text, "AND") == 0) return TOKEN_AND;
    if (strcmp(text, "OR") == 0) return TOKEN_OR;
    if (strcmp(text, "NOT") == 0) return TOKEN_NOT;
    return TOKEN_IDENTIFIER;
}

TokenArray lex_source(const char *source) {
    TokenArray tokens;
    size_t i = 0;
    int line = 1;
    int col = 1;

    tokens.items = NULL;
    tokens.count = 0;
    tokens.capacity = 0;

    while (source[i] != '\0') {
        if (source[i] == '\n') {
            line++;
            col = 1;
            i++;
            continue;
        }

        if (isspace((unsigned char)source[i])) {
            col++;
            i++;
            continue;
        }

        if (source[i] == '/' && source[i + 1] == '/') {
            while (source[i] != '\0' && source[i] != '\n') {
                i++;
            }
            continue;
        }

        if (isdigit((unsigned char)source[i])) {
            int start_col = col;
            int is_float = 0;
            size_t start = i;
            
            while (isdigit((unsigned char)source[i]) || source[i] == '.') {
                if (source[i] == '.') is_float = 1;
                i++;
                col++;
            }

            char temp[64];
            size_t len = i - start;
            if (len >= sizeof(temp)) len = sizeof(temp) - 1;
            memcpy(temp, source + start, len);
            temp[len] = '\0';

            if (is_float) {
                push_token(&tokens, TOKEN_FLOAT, line, start_col, 0, atof(temp), 0, NULL);
            } else {
                push_token(&tokens, TOKEN_INT, line, start_col, atoi(temp), 0, 0, NULL);
            }
            continue;
        }

        if (isalpha((unsigned char)source[i]) || source[i] == '_') {
            int start_col = col;
            size_t start = i;

            while (isalnum((unsigned char)source[i]) || source[i] == '_') {
                i++;
                col++;
            }

            size_t length = i - start;
            char *text = (char *)malloc(length + 1);
            memcpy(text, source + start, length);
            text[length] = '\0';

            TokenType type = keyword_type(text);
            if (type == TOKEN_IDENTIFIER) {
                push_token(&tokens, type, line, start_col, 0, 0, 0, text);
            } else {
                push_token(&tokens, type, line, start_col, 0, 0, 0, NULL);
            }

            free(text);
            continue;
        }

        if (source[i] == '"') {
            int start_col = col;
            i++; col++;
            size_t start = i;
            while (source[i] != '\0' && source[i] != '"') {
                if (source[i] == '\n') {
                    line++;
                    col = 0;
                }
                i++; col++;
            }
            if (source[i] == '\0') {
                lexer_error(line, col, "Unterminated string literal.");
            }
            size_t length = i - start;
            char *text = (char *)malloc(length + 1);
            memcpy(text, source + start, length);
            text[length] = '\0';
            push_token(&tokens, TOKEN_STRING, line, start_col, 0, 0, 0, text);
            free(text);
            i++; col++;
            continue;
        }

        if (source[i] == '\'') {
            int start_col = col;
            i++; col++;
            if (source[i] == '\0' || source[i+1] != '\'') {
                lexer_error(line, col, "Invalid char literal.");
            }
            push_token(&tokens, TOKEN_CHAR, line, start_col, 0, 0, source[i], NULL);
            i += 2; col += 2;
            continue;
        }

        int start_col = col;
        switch (source[i]) {
            case '=':
                if (source[i + 1] == '=') {
                    push_token(&tokens, TOKEN_EQ, line, start_col, 0, 0, 0, NULL);
                    i += 2; col += 2;
                } else {
                    push_token(&tokens, TOKEN_ASSIGN, line, start_col, 0, 0, 0, NULL);
                    i++; col++;
                }
                break;
            case '!':
                if (source[i + 1] == '=') {
                    push_token(&tokens, TOKEN_NEQ, line, start_col, 0, 0, 0, NULL);
                    i += 2; col += 2;
                } else {
                    lexer_error(line, col, "Unexpected '!'. Did you mean '!=' or 'NOT'?");
                }
                break;
            case '<':
                if (source[i + 1] == '=') {
                    push_token(&tokens, TOKEN_LTE, line, start_col, 0, 0, 0, NULL);
                    i += 2; col += 2;
                } else {
                    push_token(&tokens, TOKEN_LT, line, start_col, 0, 0, 0, NULL);
                    i++; col++;
                }
                break;
            case '>':
                if (source[i + 1] == '=') {
                    push_token(&tokens, TOKEN_GTE, line, start_col, 0, 0, 0, NULL);
                    i += 2; col += 2;
                } else {
                    push_token(&tokens, TOKEN_GT, line, start_col, 0, 0, 0, NULL);
                    i++; col++;
                }
                break;
            case '+': push_token(&tokens, TOKEN_PLUS, line, start_col, 0, 0, 0, NULL); i++; col++; break;
            case '-': push_token(&tokens, TOKEN_MINUS, line, start_col, 0, 0, 0, NULL); i++; col++; break;
            case '*': push_token(&tokens, TOKEN_STAR, line, start_col, 0, 0, 0, NULL); i++; col++; break;
            case '/': push_token(&tokens, TOKEN_SLASH, line, start_col, 0, 0, 0, NULL); i++; col++; break;
            case '%': push_token(&tokens, TOKEN_MOD, line, start_col, 0, 0, 0, NULL); i++; col++; break;
            case '(': push_token(&tokens, TOKEN_LPAREN, line, start_col, 0, 0, 0, NULL); i++; col++; break;
            case ')': push_token(&tokens, TOKEN_RPAREN, line, start_col, 0, 0, 0, NULL); i++; col++; break;
            case '{': push_token(&tokens, TOKEN_LBRACE, line, start_col, 0, 0, 0, NULL); i++; col++; break;
            case '}': push_token(&tokens, TOKEN_RBRACE, line, start_col, 0, 0, 0, NULL); i++; col++; break;
            case ';': push_token(&tokens, TOKEN_SEMI, line, start_col, 0, 0, 0, NULL); i++; col++; break;
            case ',': push_token(&tokens, TOKEN_COMMA, line, start_col, 0, 0, 0, NULL); i++; col++; break;
            default:
                lexer_error(line, col, "Unknown character encountered.");
        }
    }

    push_token(&tokens, TOKEN_EOF, line, col, 0, 0, 0, NULL);
    return tokens;
}

void free_tokens(TokenArray *tokens) {
    size_t i;
    if (tokens == NULL || tokens->items == NULL) return;
    for (i = 0; i < tokens->count; i++) {
        free(tokens->items[i].name);
    }
    free(tokens->items);
    tokens->items = NULL;
    tokens->count = 0;
    tokens->capacity = 0;
}
