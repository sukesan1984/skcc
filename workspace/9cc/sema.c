#include "9cc.h"

static Type int_ty = {INT, NULL};

static Map *vars;
static int stacksize;

int size_of(Type *ty) {
    if (ty->ty == INT)
        return 4;
    if (ty->ty == ARRAY)
        return 8;
    assert(ty->ty == PTR);
    return 8;
}

int get_stacksize(Node *node) {
    if(node->ty->ty == INT) {
        return 1;
    }
    return size_of(node->ty->ptr_of);
}

static void walk(Node *node) {
    switch (node->op) {
    case ND_NUM:
        node->stacksize = 1;
        return;
    case ND_IDENT: {
        Var *var = map_get(vars, node->name);
        if (!var)
            error("undefined variable: %s", node->name);
        node->ty = var->ty;
        node->offset = var->offset;
        node->stacksize = get_stacksize(node);
        //node->ty->tyがARRAYの場合PTRに置き換える
        if(node->ty->ty == ARRAY) {
            node->op = ND_ARRAY;
            Type *ty = calloc(1, sizeof(Type));
            ty->ty = PTR;
            ty->ptr_of = node->ty->ptr_of;
            node->ty = ty;
        }
        return;
    }

    case ND_VARDEF:
        if(node->ty->ty == ARRAY) {
            stacksize += 8 * node->ty->array_size;
        } else {
            stacksize += 8;
        }

        node->offset = stacksize;

        Var *var = calloc(1, sizeof(Var));
        var->ty = node->ty;
        var->offset = stacksize;
        map_put(vars, node->name, var);
        if (node->init)
            walk(node->init);
        return;
    case ND_IF:
        walk(node->lhs);
        walk(node->rhs);
        return;
    case ND_FOR:
        walk(node->lhs);
        walk(node->lhs2);
        walk(node->rhs);
        walk(node->lhs3);
        return;
    case ND_WHILE:
        walk(node->lhs);
        walk(node->rhs);
        return;
    case '+':
    case '-':
    case '*':
    case '/':
    case '=':
    case '<':
    case '>':
    case ND_LOGAND:
    case ND_LOGOR:
    case ND_EQ:
    case ND_NE:
    case ND_LE:
        walk(node->lhs);
        walk(node->rhs);
        node->ty = node->lhs->ty;
        return;
    case ND_DEREF:
        walk(node->lhs);
        node->ty = node->lhs->ty->ptr_of; // *p の場合 tyはptr_of
        node->stacksize = get_stacksize(node);
        return;
    case ND_ADDR:
    case ND_RETURN:
        walk(node->lhs);
        return;
    case ND_CALL:
        for (int i = 0; i < node->args->len; i++)
            walk(node->args->data[i]);
        node->ty = &int_ty;
        return;
    case ND_FUNC:
        for (int i = 0; i < node->args->len; i++)
            walk(node->args->data[i]);
        walk(node->body);
        return;
    case ND_COMP_STMT:
        for (int i = 0; i < node->stmts->len; i++)
            walk(node->stmts->data[i]);
        return;
    case ND_EXPR_STMT:
        walk(node->lhs);
        return;
    case ND_SIZEOF:
        walk(node->lhs);
        node->op = ND_NUM;
        node->val = size_of(node->lhs->ty);
        return;
    default:
        assert(0 && "unknown node type");
    }
}

void sema(Vector *nodes) {
    for (int i = 0; i < nodes->len; i++) {
        Node *node = nodes->data[i];
        assert(node->op == ND_FUNC);
        vars = new_map();
        stacksize = 0;
        walk(node);
        node->stacksize = stacksize;
    }
}
