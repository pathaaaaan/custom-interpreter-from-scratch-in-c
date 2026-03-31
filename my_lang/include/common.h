#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

typedef enum {
    TOKEN_INT,
    TOKEN_IDENTIFIER,
    TOKEN_ASSIGN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMI,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_PRINT,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    int value;
    char *name;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenArray;

typedef enum {
    NODE_INT,
    NODE_VAR,
    NODE_ASSIGN,
    NODE_BINARY,
    NODE_COMPARE,
    NODE_LOGICAL,
    NODE_IF,
    NODE_PRINT
} NodeType;

typedef struct Node {
    NodeType type;
    int value;
    TokenType op;
    char *name;
    struct Node *left;
    struct Node *right;
    struct Node *condition;
    struct Node *then_branch;
    struct Node *else_branch;
    struct Node *next;
} Node;

char *ll_strdup(const char *src);

#endif
