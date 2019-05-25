#include "9cc.h"

static Type int_ty = {INT, NULL};

static Map *vars;
static int stacksize;

static Node *addr_of(Node *base, Type *ty) {
    Node *node = calloc(1, sizeof(Node));
    node->op = ND_ADDR;
    node->ty = ptr_of(ty);

    Node *copy = calloc(1, sizeof(Node));
    memcpy(copy, base, sizeof(Node));
    node->lhs = copy;
    return node;
}

static Node *maybe_decay(Node *base, bool decay) {
    if (!decay || base->ty->ty != ARRAY)
        return base;
    Node *node = calloc(1, sizeof(Node));
    node->op = ND_ADDR;
    node->ty = ptr_of(base->ty->array_of);
    node->lhs = base;
    return node;
}

static Node* walk(Node *node, bool decay) {
    switch (node->op) {
    case ND_NUM:
        node->ty = &int_ty;
        return node;
    case ND_IDENT: {
        Var *var = map_get(vars, node->name);
        if (!var)
            error("undefined variable: %s", node->name);
        node->offset = var->offset;
        if(decay && var->ty->ty == ARRAY)
            *node = *addr_of(node, var->ty->array_of);
        else
            node->ty = var->ty;
        return node;
    }

    case ND_VARDEF:
        stacksize += size_of(node->ty);

        node->offset = stacksize;

        Var *var = calloc(1, sizeof(Var));
        var->ty = node->ty;
        var->offset = stacksize;
        map_put(vars, node->name, var);
        if (node->init)
            node->init = walk(node->init, true);
        return node;
    case ND_IF:
        node->lhs = walk(node->lhs, true);
        node->rhs = walk(node->rhs, true);
        return node;
    case ND_FOR:
        node->lhs = walk(node->lhs, true);
        node->lhs2 = walk(node->lhs2, true);
        node->rhs = walk(node->rhs, true);
        node->lhs3 = walk(node->lhs3, true);
        return node;
    case ND_WHILE:
        node->lhs = walk(node->lhs, true);
        node->rhs = walk(node->rhs, true);
        return node;
    case '=':
        node->lhs = walk(node->lhs, false);
        node->rhs = walk(node->rhs, true);
        node->ty = node->lhs->ty;
        return node;
    case '+':
    case '-':
    case '*':
    case '/':
    case '<':
    case '>':
    case ND_LOGAND:
    case ND_LOGOR:
    case ND_EQ:
    case ND_NE:
    case ND_LE:
        node->lhs = walk(node->lhs, true);
        node->rhs = walk(node->rhs, true);
        node->ty = node->lhs->ty;
        return node;
    case ND_DEREF:
        node->lhs = walk(node->lhs, true);
        if(node->lhs->ty->ty == ARRAY)
            fprintf(stderr, "operand is ARRAY\n");

        if(node->lhs->ty->ty != PTR)
            error("operand must be a pointer");
        node->ty = node->lhs->ty->ptr_of; // *p の場合 tyはptr_of
        return maybe_decay(node, decay);
    case ND_ADDR:
        node->lhs = walk(node->lhs, true);
        node->ty = ptr_of(node->lhs->ty);
        return node;
    case ND_RETURN:
        node->lhs = walk(node->lhs, true);
        return node;
    case ND_CALL:
        for (int i = 0; i < node->args->len; i++)
            node->args->data[i] = walk(node->args->data[i], true);
        node->ty = &int_ty;
        return node;
    case ND_FUNC:
        for (int i = 0; i < node->args->len; i++)
            node->args->data[i] = walk(node->args->data[i], true);
        node->body = walk(node->body, true);
        return node;
    case ND_COMP_STMT:
        for (int i = 0; i < node->stmts->len; i++)
            node->stmts->data[i] = walk(node->stmts->data[i], true);
        return node;
    case ND_EXPR_STMT:
        node->lhs = walk(node->lhs, true);
        return node;
    case ND_SIZEOF:
        node->lhs = walk(node->lhs, true);
        node->op = ND_NUM;
        node->val = size_of(node->lhs->ty);
        return node;
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
        node = walk(node, true);
        node->stacksize = stacksize;
    }
}
