#include <stdio.h>
#include <stdlib.h>

#include "../include/eval.h"

static void eval_error(const char *message) {
    fprintf(stderr, "Evaluation error: %s\n", message);
    exit(EXIT_FAILURE);
}

int eval(Node *node, SymbolTable *table) {
    int left;
    int right;
    int value;

    if (node == NULL) {
        return 0;
    }

    switch (node->type) {
        case NODE_INT:
            return node->value;
        case NODE_VAR:
            if (!get_variable(table, node->name, &value)) {
                eval_error("Undefined variable.");
            }
            return value;
        case NODE_ASSIGN:
            value = eval(node->right, table);
            set_variable(table, node->name, value);
            return value;
        case NODE_BINARY:
            left = eval(node->left, table);
            right = eval(node->right, table);
            switch (node->op) {
                case TOKEN_PLUS:
                    return left + right;
                case TOKEN_MINUS:
                    return left - right;
                case TOKEN_STAR:
                    return left * right;
                case TOKEN_SLASH:
                    if (right == 0) {
                        eval_error("Division by zero.");
                    }
                    return left / right;
                default:
                    eval_error("Invalid arithmetic operator.");
            }
            break;
        case NODE_COMPARE:
            left = eval(node->left, table);
            right = eval(node->right, table);
            switch (node->op) {
                case TOKEN_EQ:
                    return left == right;
                case TOKEN_NEQ:
                    return left != right;
                case TOKEN_LT:
                    return left < right;
                case TOKEN_GT:
                    return left > right;
                default:
                    eval_error("Invalid comparison operator.");
            }
            break;
        case NODE_LOGICAL:
            if (node->op == TOKEN_AND) {
                left = eval(node->left, table);
                if (!left) {
                    return 0;
                }
                return eval(node->right, table) != 0;
            }
            if (node->op == TOKEN_OR) {
                left = eval(node->left, table);
                if (left) {
                    return 1;
                }
                return eval(node->right, table) != 0;
            }
            eval_error("Invalid logical operator.");
            break;
        case NODE_IF:
            if (eval(node->condition, table)) {
                eval_program(node->then_branch, table);
            } else if (node->else_branch != NULL) {
                eval_program(node->else_branch, table);
            }
            return 0;
        case NODE_PRINT:
            value = eval(node->left, table);
            printf("%d\n", value);
            return value;
    }

    eval_error("Unknown AST node.");
    return 0;
}

void eval_program(Node *program, SymbolTable *table) {
    Node *current = program;

    while (current != NULL) {
        eval(current, table);
        current = current->next;
    }
}
