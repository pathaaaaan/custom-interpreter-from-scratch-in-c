#include <stdio.h>
#include <stdlib.h>

#include "../include/parser.h"

typedef struct {
    const TokenArray *tokens;
    size_t current;
} Parser;

static void parser_error(const char *message) {
    fprintf(stderr, "Parser error: %s\n", message);
    exit(EXIT_FAILURE);
}

static const Token *peek(Parser *parser) {
    return &parser->tokens->items[parser->current];
}

static const Token *previous(Parser *parser) {
    return &parser->tokens->items[parser->current - 1];
}

static int is_at_end(Parser *parser) {
    return peek(parser)->type == TOKEN_EOF;
}

static const Token *advance(Parser *parser) {
    if (!is_at_end(parser)) {
        parser->current++;
    }
    return previous(parser);
}

static int check(Parser *parser, TokenType type) {
    if (is_at_end(parser)) {
        return type == TOKEN_EOF;
    }
    return peek(parser)->type == type;
}

static int match(Parser *parser, TokenType type) {
    if (check(parser, type)) {
        advance(parser);
        return 1;
    }
    return 0;
}

static const Token *consume(Parser *parser, TokenType type, const char *message) {
    if (check(parser, type)) {
        return advance(parser);
    }
    parser_error(message);
    return NULL;
}

static Node *new_node(NodeType type) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        parser_error("Unable to allocate AST node.");
    }

    node->type = type;
    node->value = 0;
    node->op = TOKEN_EOF;
    node->name = NULL;
    node->left = NULL;
    node->right = NULL;
    node->condition = NULL;
    node->then_branch = NULL;
    node->else_branch = NULL;
    node->next = NULL;
    return node;
}

static Node *parse_expression(Parser *parser);
static Node *parse_logical(Parser *parser);
static Node *parse_comparison(Parser *parser);
static Node *parse_term(Parser *parser);
static Node *parse_factor(Parser *parser);
static Node *parse_unary(Parser *parser);
static Node *parse_statement(Parser *parser);
static Node *parse_block(Parser *parser);

static Node *parse_unary(Parser *parser) {
    if (match(parser, TOKEN_MINUS)) {
        Node *node = new_node(NODE_BINARY);
        Node *zero = new_node(NODE_INT);

        zero->value = 0;
        node->op = TOKEN_MINUS;
        node->left = zero;
        node->right = parse_unary(parser);
        return node;
    }

    if (match(parser, TOKEN_INT)) {
        Node *node = new_node(NODE_INT);
        node->value = previous(parser)->value;
        return node;
    }

    if (match(parser, TOKEN_IDENTIFIER)) {
        Node *node = new_node(NODE_VAR);
        node->name = ll_strdup(previous(parser)->name);
        return node;
    }

    if (match(parser, TOKEN_LPAREN)) {
        Node *node = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after expression.");
        return node;
    }

    parser_error("Expected expression.");
    return NULL;
}

static Node *parse_factor(Parser *parser) {
    Node *node = parse_unary(parser);

    while (match(parser, TOKEN_STAR) || match(parser, TOKEN_SLASH)) {
        Node *binary = new_node(NODE_BINARY);

        binary->op = previous(parser)->type;
        binary->left = node;
        binary->right = parse_unary(parser);
        node = binary;
    }

    return node;
}

static Node *parse_term(Parser *parser) {
    Node *node = parse_factor(parser);

    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        Node *binary = new_node(NODE_BINARY);

        binary->op = previous(parser)->type;
        binary->left = node;
        binary->right = parse_factor(parser);
        node = binary;
    }

    return node;
}

static Node *parse_comparison(Parser *parser) {
    Node *node = parse_term(parser);

    while (match(parser, TOKEN_EQ) || match(parser, TOKEN_NEQ) ||
           match(parser, TOKEN_LT) || match(parser, TOKEN_GT)) {
        Node *comparison = new_node(NODE_COMPARE);

        comparison->op = previous(parser)->type;
        comparison->left = node;
        comparison->right = parse_term(parser);
        node = comparison;
    }

    return node;
}

static Node *parse_logical(Parser *parser) {
    Node *node = parse_comparison(parser);

    while (match(parser, TOKEN_AND) || match(parser, TOKEN_OR)) {
        Node *logical = new_node(NODE_LOGICAL);

        logical->op = previous(parser)->type;
        logical->left = node;
        logical->right = parse_comparison(parser);
        node = logical;
    }

    return node;
}

static Node *parse_expression(Parser *parser) {
    return parse_logical(parser);
}

static Node *parse_assignment(Parser *parser) {
    const Token *name = consume(parser, TOKEN_IDENTIFIER, "Expected identifier.");
    Node *assign = new_node(NODE_ASSIGN);

    assign->name = ll_strdup(name->name);
    consume(parser, TOKEN_ASSIGN, "Expected '=' after identifier.");
    assign->right = parse_expression(parser);
    consume(parser, TOKEN_SEMI, "Expected ';' after assignment.");
    return assign;
}

static Node *parse_print(Parser *parser) {
    Node *node = new_node(NODE_PRINT);

    consume(parser, TOKEN_LPAREN, "Expected '(' after print.");
    node->left = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expected ')' after print expression.");
    consume(parser, TOKEN_SEMI, "Expected ';' after print statement.");
    return node;
}

static Node *parse_if(Parser *parser) {
    Node *node = new_node(NODE_IF);

    consume(parser, TOKEN_LPAREN, "Expected '(' after if.");
    node->condition = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expected ')' after if condition.");
    consume(parser, TOKEN_LBRACE, "Expected '{' before if block.");
    node->then_branch = parse_block(parser);

    if (match(parser, TOKEN_ELSE)) {
        consume(parser, TOKEN_LBRACE, "Expected '{' before else block.");
        node->else_branch = parse_block(parser);
    }

    return node;
}

static Node *parse_expression_statement(Parser *parser) {
    Node *node = parse_expression(parser);
    consume(parser, TOKEN_SEMI, "Expected ';' after expression.");
    return node;
}

static Node *parse_statement(Parser *parser) {
    if (match(parser, TOKEN_IF)) {
        return parse_if(parser);
    }

    if (match(parser, TOKEN_PRINT)) {
        return parse_print(parser);
    }

    if (check(parser, TOKEN_IDENTIFIER) &&
        parser->current + 1 < parser->tokens->count &&
        parser->tokens->items[parser->current + 1].type == TOKEN_ASSIGN) {
        return parse_assignment(parser);
    }

    return parse_expression_statement(parser);
}

static Node *append_statement(Node *head, Node *statement) {
    Node *tail;

    if (head == NULL) {
        return statement;
    }

    tail = head;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    tail->next = statement;
    return head;
}

static Node *parse_block(Parser *parser) {
    Node *head = NULL;

    while (!check(parser, TOKEN_RBRACE) && !is_at_end(parser)) {
        head = append_statement(head, parse_statement(parser));
    }

    consume(parser, TOKEN_RBRACE, "Expected '}' after block.");
    return head;
}

Node *parse_tokens(const TokenArray *tokens) {
    Parser parser;
    Node *head = NULL;

    parser.tokens = tokens;
    parser.current = 0;

    while (!is_at_end(&parser)) {
        head = append_statement(head, parse_statement(&parser));
    }

    return head;
}

void free_ast(Node *node) {
    Node *next;

    while (node != NULL) {
        next = node->next;
        free_ast(node->left);
        free_ast(node->right);
        free_ast(node->condition);
        free_ast(node->then_branch);
        free_ast(node->else_branch);
        free(node->name);
        free(node);
        node = next;
    }
}
