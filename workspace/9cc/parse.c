#include <stdio.h>
#include <stdlib.h>
#include "9cc.h"

int pos = 0;
int variables = 0;

static Type int_ty = {INT, NULL};

static Type *ptr_of(Type *base) {
    Type *ty = calloc(1, sizeof(Type));
    ty->ty = PTR;
    ty->ptr_of = base;
    return ty;
}

int consume(int ty) {
    Token *t = tokens->data[pos];
    if (t->ty != ty)
        return 0;
    pos++;
    return 1;
}

int expect(int ty ) {
    Token *t = tokens->data[pos];
    if (!consume(ty)) {
        fprintf(stderr, "%d is expected but got %s", ty, t->input);
        exit(1);
    }
    return 1;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->op = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = &int_ty;
    node->op = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_ident(char *name) {
    Node *node = malloc(sizeof(Node));
    node->op = ND_IDENT;
    node->name = name;
    return node;
}

Node *new_node_func(int ty, char *name, Vector *args) {
    Node *node = malloc(sizeof(Node));
    node->op = ND_CALL;
    node->name = name;
    node->args = args;
    return node;
}

Node *new_node_for(int ty, Node *lhs, Node *lhs2, Node *lhs3, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->op = ty;
    node->lhs = lhs;
    node->lhs2 = lhs2;
    node->lhs3 = lhs3;
    node->rhs = rhs;
    return node;
}


Node *add();
Node *assign();
Node *unary();
static Type *type();

Node *term() {
    Token *t = tokens->data[pos];
    if (t->ty == TK_NUM){
        return new_node_num(((Token *)tokens->data[pos++])->val);
    }

    if (consume(TK_IDENT)) {
        // 変数と関数
        // 関数かチェック
        if(consume('(')) {
            // 引数は一旦6個まで対応する
            Vector* args = new_vector(); // 引数を格納する引数Nodeが入る
            while(consume(',') || !consume(')')) {
                Node *node = add();
                vec_push(args, (void *) node);
            }
            return new_node_func(ND_CALL, t->name, args);
        }

        long offset = (long) map_get(variable_map, t->name);
        if (offset == 0)
            error("%s は宣言されていません", t->name);

        return new_node_ident(t->name);
    }

    if(consume('(')) {
        Node *node = assign();
        Token *t = tokens->data[pos];
        if (t->ty != ')') {
            error("閉じ括弧で閉じる必要があります: %s", t->input);
        }
        pos++;
        return node;
    }
    t = tokens->data[pos];
    error("数字でも開き括弧でもないトークンです: %s", t->input);
    exit(1);
}


Node *mul();
Node *unary() {
    if (consume('*')) {
        Node *node = calloc(1, sizeof(Node));
        node->op = ND_DEREF;
        node->lhs = mul();
        return node;
    }
    return term();
}

Node *mul() {
    Node *lhs = unary();
    Token *t = tokens->data[pos];
    if(t->ty == TK_EOF)
        return lhs;

    if(consume('*'))
        return new_node('*', lhs, mul());

    if(consume('/'))
        return new_node('/', lhs, mul());

    if (lhs->op == ND_NUM)
        return lhs;
    return lhs;
}

Node *add() {
    Node *lhs = mul();

    if(consume('+'))
        return new_node('+', lhs, add());

    if(consume('-'))
        return new_node('-', lhs, add());

    if (lhs->op == ND_NUM)
        return lhs;
    return lhs;
}

Node *relational() {
    Node *lhs = add();
    if(consume(TK_LE))
        return new_node(ND_LE, lhs, relational());
    if(consume('<'))
        return new_node('<', lhs, relational());
    if(consume(TK_GE))
        return new_node(ND_LE, relational(), lhs);
    if(consume('>'))
        return new_node('>', relational(), lhs);

    return lhs;
}

Node *equality() {
    Node *lhs = relational();
    if(consume(TK_EQ))
        return new_node(ND_EQ, lhs, equality());
    if(consume(TK_NE))
        return new_node(ND_NE, lhs, equality());
    return lhs;
}

Node *assign() {
    Node *lhs = equality();
    if (consume('='))
        return new_node('=', lhs, assign());
    return lhs;
}

static Type *type() {
    Token *t = tokens->data[pos];
    if (t->ty != TK_INT)
        error("typename expected, but got %s", t->input);
    pos++;

    Type *ty = &int_ty;
    while(consume('*'))
        ty = ptr_of(ty);
    return ty;
}

Node *decl() {
    Node *node = calloc(1, sizeof(Node));
    Token *t = (Token *) tokens->data[pos];
    while(t->ty == '*') {
        pos++;
        t = (Token *) tokens->data[pos];
    }
    node->op = ND_VARDEF;
    if (t->ty != TK_IDENT)
        error("variable name expected, but got %s", t->input);

    // すでに使われた変数かどうか
    long offset = (long) map_get(variable_map, t->name);

    // 使われてなければ、識別子をキーとしてRBPからのオフセットを追加する
    if (offset == 0){
        offset = (variables + 1) * 8;

        map_put(variable_map, t->name, (void *) offset);
        variables++;
    }
    node->name = t->name;
    pos++;
    return node;
}

Node *stmt() {
    Node *node;
    if (consume(TK_INT))  {
        node = decl();
        expect(';');
        return node;
    }
    if (consume('{')) {
        Vector* block_items = new_vector();
        while (!consume('}')) {
            vec_push(block_items, (void *) stmt());
        }
        node = malloc(sizeof(Node));
        node->op = '{';
        node->block_items  = block_items;
        return node;
    }

    if (consume(TK_RETURN)) {
        node = malloc(sizeof(Node));
        node->op = ND_RETURN;
        node->lhs = assign();
    } else {
        node = assign();
    }
    consume(';');
    return node;
}

Node *control() {
    if (consume(TK_IF)) {
        if(consume('(')) {
            Node *node = assign(); // if/while分のカッコ内の処理
            Token *t = tokens->data[pos];
            if (t->ty != ')') {
                error("ifは閉じ括弧で閉じる必要があります: %s", t->input);
            }
            pos++;
            return new_node(ND_IF, node, control());
        }
    }

    if (consume(TK_WHILE)) {
        if(consume('(')) {
            Node *node = assign(); // if/while分のカッコ内の処理
            Token *t = tokens->data[pos];
            if (t->ty != ')') {
                error("ifは閉じ括弧で閉じる必要があります: %s", t->input);
            }
            pos++;
            return new_node(ND_WHILE, node, control());
        }
    }

    if (consume(TK_FOR)) {
        if(consume('(')) {
            Node *lhs = assign();
            if(!consume(';'))
                error("forの中に;が一つもありません: %s", ((Token *) tokens->data[pos])->input);
            Node *lhs2 = assign();
            if(!consume(';'))
                error("forの中には;が2つ必要です: %s", ((Token *) tokens->data[pos])->input);

            Node *lhs3 = assign();
            if (!consume(')'))
                error("forは閉じ括弧で閉じる必要があります: %s", ((Token *) tokens->data[pos])->input);
            return new_node_for(ND_FOR, lhs, lhs2, lhs3, control());
        }
    }

    return stmt();
}

Node* compound_stmt() {
    Node *node = calloc(1, sizeof(Node));
    node->op = ND_COMP_STMT;
    node->stmts = new_vector();
    while(!consume('}'))
        vec_push(node->stmts, control());
    return node;
}

Node *function() {
    Node *node = calloc(1, sizeof(Node));
    node->op = ND_FUNC;
    node->args = new_vector();

    
    if (!consume(TK_INT))
        error("method type specifier missing.%s", ((Token *) tokens->data[pos])->input);

    Token *t = (Token *) tokens->data[pos];
    if (!consume(TK_IDENT))
        error("function name expected, but got %s", t->input);
    node->name = t->name;

    expect('(');
    while(consume(',') || !consume(')')) {
        if(!consume(TK_INT))
            error("function variable type must be needed: %s", ((Token *) tokens->data[pos])->input);
        vec_push(node->args, decl());
    }
    expect('{');
    node->body = compound_stmt();
    return node;
}

Vector *parse() {
    pos = 0;
    Vector *v = new_vector();
    while(((Token *)tokens->data[pos])->ty != TK_EOF)
        vec_push(v, function());
    return v;
}

