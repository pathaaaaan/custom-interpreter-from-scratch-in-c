#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/eval.h"
#include "../include/value.h"

static void eval_error(int line, int col, const char *message) {
    fprintf(stderr, "Evaluation error at line %d, col %d: %s\n", line, col, message);
    exit(EXIT_FAILURE);
}

static int is_truthy(Value val) {
    if (val.type == VAL_BOOL) return val.as.boolean;
    if (val.type == VAL_INT) return val.as.integer != 0;
    if (val.type == VAL_FLOAT) return val.as.float_val != 0.0;
    if (val.type == VAL_NIL) return 0;
    return 1;
}

static int are_equal(Value a, Value b) {
    if (a.type != b.type) return 0;
    switch (a.type) {
        case VAL_INT: return a.as.integer == b.as.integer;
        case VAL_FLOAT: return a.as.float_val == b.as.float_val;
        case VAL_BOOL: return a.as.boolean == b.as.boolean;
        case VAL_CHAR: return a.as.character == b.as.character;
        case VAL_STRING: return strcmp(a.as.string, b.as.string) == 0;
        case VAL_NIL: return 1;
        default: return 0;
    }
}

static Value eval_env(Node *node, Environment *env, ObjectTracker *tracker, int *returning, Value *ret_val);
static void eval_block(Node *program, Environment *env, ObjectTracker *tracker, int *returning, Value *ret_val);

Value eval(Node *node, Environment *env, ObjectTracker *tracker) {
    int returning = 0;
    Value ret_val = val_nil();
    return eval_env(node, env, tracker, &returning, &ret_val);
}

void eval_program(Node *program, Environment *env, ObjectTracker *tracker) {
    int returning = 0;
    Value ret_val = val_nil();
    eval_block(program, env, tracker, &returning, &ret_val);
}

static void eval_block(Node *program, Environment *env, ObjectTracker *tracker, int *returning, Value *ret_val) {
    Node *current = program;
    while (current != NULL) {
        eval_env(current, env, tracker, returning, ret_val);
        if (*returning) return;
        current = current->next;
    }
}

static Value eval_env(Node *node, Environment *env, ObjectTracker *tracker, int *returning, Value *ret_val) {
    if (node == NULL) return val_nil();

    switch (node->type) {
        case NODE_INT: { Value v; v.type = VAL_INT; v.as.integer = node->int_val; return v; }
        case NODE_FLOAT: { Value v; v.type = VAL_FLOAT; v.as.float_val = node->float_val; return v; }
        case NODE_CHAR: { Value v; v.type = VAL_CHAR; v.as.character = node->char_val; return v; }
        case NODE_STRING: {
            Value v; v.type = VAL_STRING; 
            char *s = ll_strdup(node->name);
            track_object(tracker, s);
            v.as.string = s;
            return v;
        }
        case NODE_VAR: {
            Value val;
            if (!get_variable(env, node->name, &val)) {
                eval_error(node->line, node->col, "Undefined variable.");
            }
            return val;
        }
        case NODE_ASSIGN: {
            Value val = eval_env(node->right, env, tracker, returning, ret_val);
            if (*returning) return val_nil();
            set_variable(env, node->name, val);
            return val;
        }
        case NODE_UNARY: {
            Value right = eval_env(node->right, env, tracker, returning, ret_val);
            if (*returning) return val_nil();
            if (node->op == TOKEN_MINUS) {
                if (right.type == VAL_INT) { right.as.integer = -right.as.integer; return right; }
                if (right.type == VAL_FLOAT) { right.as.float_val = -right.as.float_val; return right; }
                eval_error(node->line, node->col, "Unary minus requires a number.");
            }
            if (node->op == TOKEN_NOT) {
                Value v; v.type = VAL_BOOL; v.as.boolean = !is_truthy(right); return v;
            }
            break;
        }
        case NODE_BINARY: {
            Value left = eval_env(node->left, env, tracker, returning, ret_val);
            if (*returning) return val_nil();
            Value right = eval_env(node->right, env, tracker, returning, ret_val);
            if (*returning) return val_nil();

            if (node->op == TOKEN_PLUS && left.type == VAL_STRING && right.type == VAL_STRING) {
                char *res = malloc(strlen(left.as.string) + strlen(right.as.string) + 1);
                strcpy(res, left.as.string);
                strcat(res, right.as.string);
                track_object(tracker, res);
                Value v; v.type = VAL_STRING; v.as.string = res;
                return v;
            }

            int is_float = (left.type == VAL_FLOAT || right.type == VAL_FLOAT);
            double l_val = left.type == VAL_FLOAT ? left.as.float_val : (double)left.as.integer;
            double r_val = right.type == VAL_FLOAT ? right.as.float_val : (double)right.as.integer;
            int l_int = left.as.integer;
            int r_int = right.as.integer;

            Value res;
            if (is_float) {
                res.type = VAL_FLOAT;
                switch (node->op) {
                    case TOKEN_PLUS: res.as.float_val = l_val + r_val; break;
                    case TOKEN_MINUS: res.as.float_val = l_val - r_val; break;
                    case TOKEN_STAR: res.as.float_val = l_val * r_val; break;
                    case TOKEN_SLASH: res.as.float_val = l_val / r_val; break;
                    case TOKEN_MOD: eval_error(node->line, node->col, "Modulo on floats not supported."); break;
                    default: eval_error(node->line, node->col, "Invalid float operator.");
                }
            } else {
                res.type = VAL_INT;
                switch (node->op) {
                    case TOKEN_PLUS: res.as.integer = l_int + r_int; break;
                    case TOKEN_MINUS: res.as.integer = l_int - r_int; break;
                    case TOKEN_STAR: res.as.integer = l_int * r_int; break;
                    case TOKEN_SLASH: 
                        if (r_int == 0) eval_error(node->line, node->col, "Division by zero.");
                        res.as.integer = l_int / r_int; break;
                    case TOKEN_MOD:
                        if (r_int == 0) eval_error(node->line, node->col, "Modulo by zero.");
                        res.as.integer = l_int % r_int; break;
                    default: eval_error(node->line, node->col, "Invalid int operator.");
                }
            }
            return res;
        }
        case NODE_COMPARE: {
            Value left = eval_env(node->left, env, tracker, returning, ret_val);
            if (*returning) return val_nil();
            Value right = eval_env(node->right, env, tracker, returning, ret_val);
            if (*returning) return val_nil();
            
            Value res; res.type = VAL_BOOL; res.as.boolean = 0;
            if (node->op == TOKEN_EQ) { res.as.boolean = are_equal(left, right); return res; }
            if (node->op == TOKEN_NEQ) { res.as.boolean = !are_equal(left, right); return res; }

            double l_val = left.type == VAL_FLOAT ? left.as.float_val : (double)left.as.integer;
            double r_val = right.type == VAL_FLOAT ? right.as.float_val : (double)right.as.integer;

            switch (node->op) {
                case TOKEN_LT: res.as.boolean = l_val < r_val; break;
                case TOKEN_GT: res.as.boolean = l_val > r_val; break;
                case TOKEN_LTE: res.as.boolean = l_val <= r_val; break;
                case TOKEN_GTE: res.as.boolean = l_val >= r_val; break;
                default: break;
            }
            return res;
        }
        case NODE_LOGICAL: {
            Value left = eval_env(node->left, env, tracker, returning, ret_val);
            if (*returning) return val_nil();
            if (node->op == TOKEN_OR) {
                if (is_truthy(left)) return left;
            } else if (node->op == TOKEN_AND) {
                if (!is_truthy(left)) return left;
            }
            return eval_env(node->right, env, tracker, returning, ret_val);
        }
        case NODE_IF: {
            if (is_truthy(eval_env(node->condition, env, tracker, returning, ret_val))) {
                if (*returning) return val_nil();
                eval_block(node->then_branch, env, tracker, returning, ret_val);
            } else if (node->else_branch) {
                if (*returning) return val_nil();
                eval_block(node->else_branch, env, tracker, returning, ret_val);
            }
            return val_nil();
        }
        case NODE_WHILE: {
            while (is_truthy(eval_env(node->condition, env, tracker, returning, ret_val))) {
                if (*returning) return val_nil();
                eval_block(node->body, env, tracker, returning, ret_val);
                if (*returning) break;
            }
            return val_nil();
        }
        case NODE_FOR: {
            // Environment scoping for loops
            Environment for_env;
            init_env(&for_env, env);

            if (node->init) {
                eval_env(node->init, &for_env, tracker, returning, ret_val);
            }
            
            while (is_truthy(eval_env(node->condition, &for_env, tracker, returning, ret_val))) {
                if (*returning) break;
                eval_block(node->body, &for_env, tracker, returning, ret_val);
                if (*returning) break;
                
                if (node->inc) {
                    eval_env(node->inc, &for_env, tracker, returning, ret_val);
                }
            }
            free_env(&for_env);
            return val_nil();
        }
        case NODE_DEF: {
            Value v;
            v.type = VAL_FUNCTION;
            // Since closures are not required, we don't save the environment
            v.as.func.body = node->body;
            v.as.func.param_count = node->param_count;
            v.as.func.params = node->param_names; // Points directly to AST strings
            set_variable(env, node->name, v);
            return val_nil();
        }
        case NODE_RETURN: {
            if (node->right) {
                *ret_val = eval_env(node->right, env, tracker, returning, ret_val);
            } else {
                *ret_val = val_nil();
            }
            *returning = 1;
            return val_nil();
        }
        case NODE_CALL: {
            Value func = eval_env(node->left, env, tracker, returning, ret_val);
            if (*returning) return val_nil();
            
            if (func.type != VAL_FUNCTION) {
                eval_error(node->line, node->col, "Attempt to call non-function.");
            }

            if (node->arg_count != func.as.func.param_count) {
                eval_error(node->line, node->col, "Incorrect argument count.");
            }

            Environment func_env;
            // No closures required, so we just attach to the global env.
            // But wait, what if recursion? We should probably just pass the top-most global env.
            // Traversing upwards to find global env:
            Environment *global_env = env;
            while (global_env->enclosing != NULL) global_env = global_env->enclosing;
            init_env(&func_env, global_env);

            for (int i = 0; i < node->arg_count; i++) {
                Value arg_val = eval_env(node->args[i], env, tracker, returning, ret_val);
                if (*returning) { free_env(&func_env); return val_nil(); }
                set_variable(&func_env, func.as.func.params[i], arg_val);
            }

            int func_returning = 0;
            Value func_ret_val = val_nil();
            eval_block(func.as.func.body, &func_env, tracker, &func_returning, &func_ret_val);
            
            free_env(&func_env);
            return func_ret_val;
        }
        case NODE_PRINT: {
            Value val = eval_env(node->left, env, tracker, returning, ret_val);
            if (*returning) return val_nil();
            print_value(val);
            printf("\n");
            return val_nil();
        }
        case NODE_LEN: {
            Value val = eval_env(node->left, env, tracker, returning, ret_val);
            if (*returning) return val_nil();
            if (val.type != VAL_STRING) eval_error(node->line, node->col, "len() requires a string.");
            Value res; res.type = VAL_INT; res.as.integer = strlen(val.as.string);
            return res;
        }
        case NODE_STR: {
            Value val = eval_env(node->left, env, tracker, returning, ret_val);
            if (*returning) return val_nil();
            char buffer[64];
            if (val.type == VAL_INT) snprintf(buffer, sizeof(buffer), "%d", val.as.integer);
            else if (val.type == VAL_FLOAT) snprintf(buffer, sizeof(buffer), "%g", val.as.float_val);
            else if (val.type == VAL_BOOL) snprintf(buffer, sizeof(buffer), "%s", val.as.boolean ? "true" : "false");
            else if (val.type == VAL_CHAR) snprintf(buffer, sizeof(buffer), "%c", val.as.character);
            else if (val.type == VAL_STRING) { return val; } // return copy?
            else snprintf(buffer, sizeof(buffer), "nil");
            
            char *s = ll_strdup(buffer);
            track_object(tracker, s);
            Value res; res.type = VAL_STRING; res.as.string = s;
            return res;
        }
    }

    eval_error(node->line, node->col, "Unknown AST node in evaluator.");
    return val_nil();
}

void print_ast(Node *node, int depth) {
    if (node == NULL) return;
    for (int i=0; i<depth; i++) printf("  ");
    switch (node->type) {
        case NODE_INT: printf("INT(%d)\n", node->int_val); break;
        case NODE_FLOAT: printf("FLOAT(%g)\n", node->float_val); break;
        case NODE_CHAR: printf("CHAR('%c')\n", node->char_val); break;
        case NODE_STRING: printf("STRING(\"%s\")\n", node->name); break;
        case NODE_VAR: printf("VAR(%s)\n", node->name); break;
        case NODE_ASSIGN: printf("ASSIGN(%s)\n", node->name); break;
        case NODE_BINARY: printf("BINARY(op:%d)\n", node->op); break;
        case NODE_UNARY: printf("UNARY(op:%d)\n", node->op); break;
        case NODE_COMPARE: printf("COMPARE(op:%d)\n", node->op); break;
        case NODE_LOGICAL: printf("LOGICAL(op:%d)\n", node->op); break;
        case NODE_IF: printf("IF\n"); break;
        case NODE_WHILE: printf("WHILE\n"); break;
        case NODE_FOR: printf("FOR\n"); break;
        case NODE_DEF: printf("DEF(%s)\n", node->name); break;
        case NODE_RETURN: printf("RETURN\n"); break;
        case NODE_CALL: printf("CALL\n"); break;
        case NODE_PRINT: printf("PRINT\n"); break;
        case NODE_LEN: printf("LEN\n"); break;
        case NODE_STR: printf("STR\n"); break;
    }

    print_ast(node->left, depth + 1);
    print_ast(node->right, depth + 1);
    print_ast(node->condition, depth + 1);
    print_ast(node->then_branch, depth + 1);
    print_ast(node->else_branch, depth + 1);
    print_ast(node->init, depth + 1);
    print_ast(node->inc, depth + 1);
    if (node->type == NODE_CALL) {
        for(int i=0; i<node->arg_count; i++) print_ast(node->args[i], depth + 1);
    }
    if (node->type == NODE_DEF || node->type == NODE_WHILE || node->type == NODE_FOR) {
        Node *n = node->body;
        while(n) {
            print_ast(n, depth + 1);
            n = n->next;
        }
    }
    if (node->type != NODE_DEF && node->type != NODE_WHILE && node->type != NODE_FOR) {
        print_ast(node->next, depth); // Only block siblings at same depth
    }
}
