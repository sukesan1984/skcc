#include "9cc.h"

static Type int_ty = {INT, 4, 4};


static int str_label;
static Map *vars;
Vector *globals;
Vector *strings;
static int stacksize;

static Node *maybe_decay(Node *base, bool decay) {
    if (!decay || base->ty->ty != ARRAY)
        return base;
    Node *node = calloc(1, sizeof(Node));
    node->op = ND_ADDR;
    node->ty = ptr_to(base->ty->array_of);
    node->lhs = base;
    return node;
}

static Var *new_global(Type* ty, char *data, int len) {
    Var *var = calloc(1, sizeof(Var));
    var->ty = ty;
    var->is_local = false;
    var->data = data;
    var->len = len;
    var->name = format(".L.str%d", str_label++);
    return var;
}

static Node* walk(Node *node, bool decay) {
    switch (node->op) {
    case ND_NUM:
        node->ty = &int_ty;
        return node;
    case ND_STR: {
        Var *var = new_global(node->ty, node->data, node->len);
        vec_push(globals, var);
        Node *ret = calloc(1, sizeof(Node));
        ret->op = ND_GVAR;
        ret->ty = node->ty;
        ret->name = var->name;
        return walk(ret, decay);
    }
    case ND_IDENT: {
        Var *var = map_get(vars, node->name);
        if (!var)
            error("undefined variable: %s", node->name);
        if (var->is_local) {
            Node *ret = calloc(1, sizeof(Node));
            ret->op = ND_LVAR;
            ret->ty = var->ty;
            ret->offset = var->offset;
            return maybe_decay(ret, decay);
        }

        Node *ret = calloc(1, sizeof(Node));
        ret->op = ND_GVAR;
        ret->ty = var->ty;
        ret->name = var->name;
        return maybe_decay(ret, decay);
    }

    case ND_GVAR:
        if (decay && node->ty->ty == ARRAY)
            return maybe_decay(node, decay);
        return node;
    case ND_VARDEF:
        stacksize = roundup(stacksize, node->ty->align);
        stacksize += node->ty->size;
        node->offset = stacksize;

        Var *var = calloc(1, sizeof(Var));
        var->ty = node->ty;
        var->offset = stacksize;
        var->is_local = true;
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
        node->ty = node->lhs->ty->ptr_to; // *p の場合 tyはptr_of
        return maybe_decay(node, decay);
    case ND_ADDR:
        node->lhs = walk(node->lhs, true);
        node->ty = ptr_to(node->lhs->ty);
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
        node->lhs = walk(node->lhs, false);
        node->op = ND_NUM;
        node->val = node->lhs->ty->size;
        return node;
    default:
        assert(0 && "unknown node type");
    }
}

void sema(Vector *nodes) {
    str_label = 0;
    globals = new_vector();
    vars = new_map();
    for (int i = 0; i < nodes->len; i++) {
        Node *node = nodes->data[i];

        // Global Variables
        if (node->op == ND_VARDEF) {
            Var *var = new_global(node->ty, node->data, node->len);
            vec_push(globals, var);
            map_put(vars, node->name, var);
            continue;
        }

        assert(node->op == ND_FUNC);
        stacksize = 0;
        node = walk(node, true);
        node->stacksize = stacksize;
    }
}
