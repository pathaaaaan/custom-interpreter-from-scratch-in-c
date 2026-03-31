#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/lexer.h"

char *ll_strdup(const char *src) {
    size_t len;
    char *copy;

    if (src == NULL) {
        return NULL;
    }

    len = strlen(src);
    copy = (char *)malloc(len + 1);
    if (copy == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(copy, src, len + 1);
    return copy;
}

static void lexer_error(const char *message) {
    fprintf(stderr, "Lexer error: %s\n", message);
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
    if (resized == NULL) {
        lexer_error("Unable to grow token buffer.");
    }

    tokens->items = resized;
    tokens->capacity = new_capacity;
}

static void push_token(TokenArray *tokens, TokenType type, int value, const char *name) {
    Token token;

    ensure_capacity(tokens);
    token.type = type;
    token.value = value;
    token.name = name != NULL ? ll_strdup(name) : NULL;
    tokens->items[tokens->count++] = token;
}

static TokenType keyword_type(const char *text) {
    if (strcmp(text, "if") == 0) {
        return TOKEN_IF;
    }
    if (strcmp(text, "else") == 0) {
        return TOKEN_ELSE;
    }
    if (strcmp(text, "print") == 0) {
        return TOKEN_PRINT;
    }
    if (strcmp(text, "AND") == 0) {
        return TOKEN_AND;
    }
    if (strcmp(text, "OR") == 0) {
        return TOKEN_OR;
    }
    return TOKEN_IDENTIFIER;
}

TokenArray lex_source(const char *source) {
    TokenArray tokens;
    size_t i;

    tokens.items = NULL;
    tokens.count = 0;
    tokens.capacity = 0;
    i = 0;

    while (source[i] != '\0') {
        if (isspace((unsigned char)source[i])) {
            i++;
            continue;
        }

        if (isdigit((unsigned char)source[i])) {
            int value = 0;

            while (isdigit((unsigned char)source[i])) {
                value = (value * 10) + (source[i] - '0');
                i++;
            }

            push_token(&tokens, TOKEN_INT, value, NULL);
            continue;
        }

        if (isalpha((unsigned char)source[i]) || source[i] == '_') {
            size_t start = i;
            size_t length;
            char *text;
            TokenType type;

            while (isalnum((unsigned char)source[i]) || source[i] == '_') {
                i++;
            }

            length = i - start;
            text = (char *)malloc(length + 1);
            if (text == NULL) {
                lexer_error("Unable to allocate identifier.");
            }

            memcpy(text, source + start, length);
            text[length] = '\0';

            type = keyword_type(text);
            if (type == TOKEN_IDENTIFIER) {
                push_token(&tokens, type, 0, text);
            } else {
                push_token(&tokens, type, 0, NULL);
            }

            free(text);
            continue;
        }

        switch (source[i]) {
            case '=':
                if (source[i + 1] == '=') {
                    push_token(&tokens, TOKEN_EQ, 0, NULL);
                    i += 2;
                } else {
                    push_token(&tokens, TOKEN_ASSIGN, 0, NULL);
                    i++;
                }
                break;
            case '!':
                if (source[i + 1] != '=') {
                    lexer_error("Unexpected '!'. Did you mean '!='?");
                }
                push_token(&tokens, TOKEN_NEQ, 0, NULL);
                i += 2;
                break;
            case '+':
                push_token(&tokens, TOKEN_PLUS, 0, NULL);
                i++;
                break;
            case '-':
                push_token(&tokens, TOKEN_MINUS, 0, NULL);
                i++;
                break;
            case '*':
                push_token(&tokens, TOKEN_STAR, 0, NULL);
                i++;
                break;
            case '/':
                push_token(&tokens, TOKEN_SLASH, 0, NULL);
                i++;
                break;
            case '(':
                push_token(&tokens, TOKEN_LPAREN, 0, NULL);
                i++;
                break;
            case ')':
                push_token(&tokens, TOKEN_RPAREN, 0, NULL);
                i++;
                break;
            case '{':
                push_token(&tokens, TOKEN_LBRACE, 0, NULL);
                i++;
                break;
            case '}':
                push_token(&tokens, TOKEN_RBRACE, 0, NULL);
                i++;
                break;
            case ';':
                push_token(&tokens, TOKEN_SEMI, 0, NULL);
                i++;
                break;
            case '<':
                push_token(&tokens, TOKEN_LT, 0, NULL);
                i++;
                break;
            case '>':
                push_token(&tokens, TOKEN_GT, 0, NULL);
                i++;
                break;
            default:
                lexer_error("Unknown character encountered.");
        }
    }

    push_token(&tokens, TOKEN_EOF, 0, NULL);
    return tokens;
}

void free_tokens(TokenArray *tokens) {
    size_t i;

    if (tokens == NULL || tokens->items == NULL) {
        return;
    }

    for (i = 0; i < tokens->count; i++) {
        free(tokens->items[i].name);
    }

    free(tokens->items);
    tokens->items = NULL;
    tokens->count = 0;
    tokens->capacity = 0;
}
