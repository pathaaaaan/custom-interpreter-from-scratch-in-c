#include <stdio.h>
#include <stdlib.h>

#include "../include/parser.h"

typedef struct {
    const TokenArray *tokens;
    size_t current;
} Parser;

static void parser_error(int line, int col, const char *message) {
    fprintf(stderr, "Parser error at line %d, col %d: %s\n", line, col, message);
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
    if (!is_at_end(parser)) parser->current++;
    return previous(parser);
}

static int check(Parser *parser, TokenType type) {
    if (is_at_end(parser)) return type == TOKEN_EOF;
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
    if (check(parser, type)) return advance(parser);
    parser_error(peek(parser)->line, peek(parser)->col, message);
    return NULL;
}

static Node *new_node(NodeType type, int line, int col) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (!node) parser_error(line, col, "Unable to allocate AST node.");
    node->type = type;
    node->line = line;
    node->col = col;
    node->int_val = 0;
    node->float_val = 0.0;
    node->char_val = '\0';
    node->name = NULL;
    node->op = TOKEN_EOF;
    node->left = NULL;
    node->right = NULL;
    node->condition = NULL;
    node->then_branch = NULL;
    node->else_branch = NULL;
    node->param_names = NULL;
    node->param_count = 0;
    node->body = NULL;
    node->init = NULL;
    node->inc = NULL;
    node->args = NULL;
    node->arg_count = 0;
    node->next = NULL;
    return node;
}

static Node *parse_expression(Parser *parser);
static Node *parse_statement(Parser *parser);
static Node *parse_block(Parser *parser);

static Node *parse_primary(Parser *parser) {
    const Token *tok = peek(parser);
    if (match(parser, TOKEN_INT)) {
        Node *n = new_node(NODE_INT, tok->line, tok->col);
        n->int_val = previous(parser)->int_val;
        return n;
    }
    if (match(parser, TOKEN_FLOAT)) {
        Node *n = new_node(NODE_FLOAT, tok->line, tok->col);
        n->float_val = previous(parser)->float_val;
        return n;
    }
    if (match(parser, TOKEN_CHAR)) {
        Node *n = new_node(NODE_CHAR, tok->line, tok->col);
        n->char_val = previous(parser)->char_val;
        return n;
    }
    if (match(parser, TOKEN_STRING)) {
        Node *n = new_node(NODE_STRING, tok->line, tok->col);
        n->name = ll_strdup(previous(parser)->name);
        return n;
    }
    if (match(parser, TOKEN_IDENTIFIER)) {
        Node *n = new_node(NODE_VAR, tok->line, tok->col);
        n->name = ll_strdup(previous(parser)->name);
        return n;
    }
    if (match(parser, TOKEN_LPAREN)) {
        Node *n = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after expression.");
        return n;
    }
    if (match(parser, TOKEN_LEN)) {
        Node *n = new_node(NODE_LEN, tok->line, tok->col);
        consume(parser, TOKEN_LPAREN, "Expected '(' after len.");
        n->left = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after len expression.");
        return n;
    }
    if (match(parser, TOKEN_STR)) {
        Node *n = new_node(NODE_STR, tok->line, tok->col);
        consume(parser, TOKEN_LPAREN, "Expected '(' after str.");
        n->left = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expected ')' after str expression.");
        return n;
    }
    parser_error(tok->line, tok->col, "Expected expression.");
    return NULL;
}

static Node *parse_call(Parser *parser) {
    Node *expr = parse_primary(parser);
    
    while (1) {
        if (match(parser, TOKEN_LPAREN)) {
            Node *node = new_node(NODE_CALL, expr->line, expr->col);
            node->left = expr;
            node->args = NULL;
            node->arg_count = 0;
            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    node->args = realloc(node->args, sizeof(Node*) * (node->arg_count + 1));
                    node->args[node->arg_count++] = parse_expression(parser);
                } while (match(parser, TOKEN_COMMA));
            }
            consume(parser, TOKEN_RPAREN, "Expected ')' after arguments.");
            expr = node;
        } else {
            break;
        }
    }
    return expr;
}

static Node *parse_unary(Parser *parser) {
    const Token *tok = peek(parser);
    if (match(parser, TOKEN_MINUS) || match(parser, TOKEN_NOT)) {
        TokenType op = previous(parser)->type;
        Node *node = new_node(NODE_UNARY, tok->line, tok->col);
        node->op = op;
        node->right = parse_unary(parser);
        return node;
    }
    return parse_call(parser);
}

static Node *parse_factor(Parser *parser) {
    Node *node = parse_unary(parser);
    while (match(parser, TOKEN_STAR) || match(parser, TOKEN_SLASH) || match(parser, TOKEN_MOD)) {
        Node *binary = new_node(NODE_BINARY, node->line, node->col);
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
        Node *binary = new_node(NODE_BINARY, node->line, node->col);
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
           match(parser, TOKEN_LT) || match(parser, TOKEN_GT) ||
           match(parser, TOKEN_LTE) || match(parser, TOKEN_GTE)) {
        Node *comp = new_node(NODE_COMPARE, node->line, node->col);
        comp->op = previous(parser)->type;
        comp->left = node;
        comp->right = parse_term(parser);
        node = comp;
    }
    return node;
}

static Node *parse_logical(Parser *parser) {
    Node *node = parse_comparison(parser);
    while (match(parser, TOKEN_AND) || match(parser, TOKEN_OR)) {
        Node *logical = new_node(NODE_LOGICAL, node->line, node->col);
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
    const Token *name_tok = consume(parser, TOKEN_IDENTIFIER, "Expected identifier.");
    Node *assign = new_node(NODE_ASSIGN, name_tok->line, name_tok->col);
    assign->name = ll_strdup(name_tok->name);
    consume(parser, TOKEN_ASSIGN, "Expected '=' after identifier.");
    assign->right = parse_expression(parser);
    consume(parser, TOKEN_SEMI, "Expected ';' after assignment.");
    return assign;
}

static Node *parse_print(Parser *parser) {
    Node *node = new_node(NODE_PRINT, previous(parser)->line, previous(parser)->col);
    consume(parser, TOKEN_LPAREN, "Expected '(' after print.");
    node->left = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expected ')' after print expression.");
    consume(parser, TOKEN_SEMI, "Expected ';' after print statement.");
    return node;
}


static Node *parse_if(Parser *parser) {
    Node *node = new_node(NODE_IF, previous(parser)->line, previous(parser)->col);
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

static Node *parse_while(Parser *parser) {
    Node *node = new_node(NODE_WHILE, previous(parser)->line, previous(parser)->col);
    consume(parser, TOKEN_LPAREN, "Expected '(' after while.");
    node->condition = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expected ')' after while condition.");
    consume(parser, TOKEN_LBRACE, "Expected '{' before while block.");
    node->body = parse_block(parser);
    return node;
}

static Node *parse_for(Parser *parser) {
    Node *node = new_node(NODE_FOR, previous(parser)->line, previous(parser)->col);
    consume(parser, TOKEN_LPAREN, "Expected '(' after for.");
    
    if (check(parser, TOKEN_IDENTIFIER) && parser->current + 1 < parser->tokens->count && parser->tokens->items[parser->current + 1].type == TOKEN_ASSIGN) {
        node->init = parse_assignment(parser);
    } else if (!check(parser, TOKEN_SEMI)){
        node->init = parse_expression(parser);
        consume(parser, TOKEN_SEMI, "Expected ';'");
    } else {
        consume(parser, TOKEN_SEMI, "Expected ';'");
    }
    
    if (!check(parser, TOKEN_SEMI)) {
        node->condition = parse_expression(parser);
    }
    consume(parser, TOKEN_SEMI, "Expected ';' after loop condition.");

    if (!check(parser, TOKEN_RPAREN)) {
        if (check(parser, TOKEN_IDENTIFIER) && parser->current + 1 < parser->tokens->count && parser->tokens->items[parser->current + 1].type == TOKEN_ASSIGN) {
            const Token *name_tok = consume(parser, TOKEN_IDENTIFIER, "Exp id");
            Node *assign = new_node(NODE_ASSIGN, name_tok->line, name_tok->col);
            assign->name = ll_strdup(name_tok->name);
            consume(parser, TOKEN_ASSIGN, "Exp '='");
            assign->right = parse_expression(parser);
            node->inc = assign;
        } else {
            node->inc = parse_expression(parser);
        }
    }
    consume(parser, TOKEN_RPAREN, "Expected ')' after for clauses.");
    consume(parser, TOKEN_LBRACE, "Expected '{' before for block.");
    node->body = parse_block(parser);
    return node;
}


static Node *parse_def(Parser *parser) {
    Node *node = new_node(NODE_DEF, previous(parser)->line, previous(parser)->col);
    const Token *name_tok = consume(parser, TOKEN_IDENTIFIER, "Expected function name.");
    node->name = ll_strdup(name_tok->name);
    consume(parser, TOKEN_LPAREN, "Expected '(' after function name.");
    
    if (!check(parser, TOKEN_RPAREN)) {
        do {
            node->param_names = realloc(node->param_names, sizeof(char*) * (node->param_count + 1));
            node->param_names[node->param_count++] = ll_strdup(consume(parser, TOKEN_IDENTIFIER, "Expected parameter name.")->name);
        } while (match(parser, TOKEN_COMMA));
    }
    consume(parser, TOKEN_RPAREN, "Expected ')' after parameters.");
    consume(parser, TOKEN_LBRACE, "Expected '{' before function body.");
    node->body = parse_block(parser);
    return node;
}

static Node *parse_return(Parser *parser) {
    Node *node = new_node(NODE_RETURN, previous(parser)->line, previous(parser)->col);
    if (!check(parser, TOKEN_SEMI)) {
        node->right = parse_expression(parser);
    }
    consume(parser, TOKEN_SEMI, "Expected ';' after return value.");
    return node;
}

static Node *parse_expression_statement(Parser *parser) {
    Node *node = parse_expression(parser);
    consume(parser, TOKEN_SEMI, "Expected ';' after expression.");
    return node;
}

static Node *parse_statement(Parser *parser) {
    if (match(parser, TOKEN_IF)) return parse_if(parser);
    if (match(parser, TOKEN_WHILE)) return parse_while(parser);
    if (match(parser, TOKEN_FOR)) return parse_for(parser);
    if (match(parser, TOKEN_DEF)) return parse_def(parser);
    if (match(parser, TOKEN_RETURN)) return parse_return(parser);
    if (match(parser, TOKEN_PRINT)) return parse_print(parser);

    if (check(parser, TOKEN_IDENTIFIER) &&
        parser->current + 1 < parser->tokens->count &&
        parser->tokens->items[parser->current + 1].type == TOKEN_ASSIGN) {
        return parse_assignment(parser);
    }
    return parse_expression_statement(parser);
}

static Node *append_statement(Node *head, Node *statement) {
    if (head == NULL) return statement;
    Node *tail = head;
    while (tail->next != NULL) tail = tail->next;
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
    while (node != NULL) {
        Node *next = node->next;
        free_ast(node->left);
        free_ast(node->right);
        free_ast(node->condition);
        free_ast(node->then_branch);
        free_ast(node->else_branch);
        free_ast(node->body);
        free_ast(node->init);
        free_ast(node->inc);
        for(int i = 0; i < node->arg_count; i++) {
            free_ast(node->args[i]);
        }
        free(node->args);
        for(int i = 0; i < node->param_count; i++) {
            free(node->param_names[i]);
        }
        free(node->param_names);
        free(node->name);
        free(node);
        node = next;
    }
}
