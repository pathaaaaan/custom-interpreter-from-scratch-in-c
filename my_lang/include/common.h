#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

typedef enum {
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_CHAR,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_ASSIGN,    // =
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_STAR,      // *
    TOKEN_SLASH,     // /
    TOKEN_MOD,       // %
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_SEMI,      // ;
    TOKEN_COMMA,     // ,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_DEF,
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_LEN,
    TOKEN_STR,
    TOKEN_EQ,        // ==
    TOKEN_NEQ,       // !=
    TOKEN_LT,        // <
    TOKEN_GT,        // >
    TOKEN_LTE,       // <=
    TOKEN_GTE,       // >=
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    int line;
    int col;
    int int_val;
    double float_val;
    char char_val;
    char *name; // holds identifiers or strings
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenArray;

typedef enum {
    NODE_INT,
    NODE_FLOAT,
    NODE_CHAR,
    NODE_STRING,
    NODE_VAR,
    NODE_ASSIGN,
    NODE_BINARY,
    NODE_COMPARE,
    NODE_LOGICAL,
    NODE_UNARY,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_DEF,
    NODE_RETURN,
    NODE_CALL,
    NODE_PRINT,
    NODE_LEN,
    NODE_STR
} NodeType;

typedef struct Node {
    NodeType type;
    int line;
    int col;

    int int_val;
    double float_val;
    char char_val;
    char *name;

    TokenType op; 

    struct Node *left;
    struct Node *right;
    struct Node *condition;
    struct Node *then_branch;
    struct Node *else_branch;

    // Functions (`def`)
    char **param_names;
    int param_count;
    struct Node *body; // used for def, while, for

    // For loop
    struct Node *init;
    struct Node *inc;

    // Function calls
    struct Node **args;
    int arg_count;

    struct Node *next; // used for block statements
} Node;

char *ll_strdup(const char *src);

#endif
