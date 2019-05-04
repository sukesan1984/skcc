#include <stdio.h>
#include <stdlib.h>
#include "9cc.h"

int pos = 0;
int variables = 0;
// エラーを報告するための関数
void error(char *str, char *i){
    fprintf(stderr, "%s%s\n", str, i);
    exit(1);
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
    if (!consume(ty))
        error("%d is expected but got %s", t->input);
    return 1;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_ident(char *name) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_IDENT;
    node->name = name;
    return node;
}

Node *new_node_func(int ty, char *name, Vector *args) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_CALL;
    node->name = name;
    node->args = args;
    return node;
}

Node *new_node_for(int ty, Node *lhs, Node *lhs2, Node *lhs3, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->lhs2 = lhs2;
    node->lhs3 = lhs3;
    node->rhs = rhs;
    return node;
}

Vector *parse() {
    pos = 0;
    Vector *v = new_vector();
    while(((Token *)tokens->data[pos])->ty != TK_EOF)
        vec_push(v, function());
    return v;
}

Node *function() {
    Node *node = calloc(1, sizeof(Node));
    node->ty = ND_FUNC;
    node->args = new_vector();

    Token *t = tokens->data[pos];
    if (t->ty != TK_IDENT)
        error("function name expected, but got %s", t->input);
    node->name = t->name;
    pos++;

    expect('(');
    while(consume(',') || !consume(')'))
        vec_push(node->args, term());
    expect('{');
    node->body = compound_stmt();
    return node;
}

Node* compound_stmt() {
    Node *node = calloc(1, sizeof(Node));
    node->ty = ND_COMP_STMT;
    node->stmts = new_vector();
    while(!consume('}'))
        vec_push(node->stmts, control());
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

Node *stmt() {
    Node *node;
    if (consume('{')) {
        Vector* block_items = new_vector();
        while (!consume('}')) {
            vec_push(block_items, (void *) stmt());
        }
        node = malloc(sizeof(Node));
        node->ty = '{';
        node->block_items  = block_items;
        return node;
    }

    if (consume(TK_RETURN)) {
        node = malloc(sizeof(Node));
        node->ty = ND_RETURN;
        node->lhs = assign();
    } else {
        node = assign();
    }
    consume(';');
    return node;
}

Node *assign() {
    Node *lhs = equality();
    if (consume('='))
        return new_node('=', lhs, assign());
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

Node *add() {
    Node *lhs = mul();

    if(consume('+'))
        return new_node('+', lhs, add());

    if(consume('-'))
        return new_node('-', lhs, add());

    if (lhs->ty == TK_NUM)
        return lhs;
    return lhs;
}

Node *mul() {
    Node *lhs = term();
    Token *t = tokens->data[pos];
    if(t->ty == TK_EOF)
        return lhs;

    if(consume('*'))
        return new_node('*', lhs, mul());

    if(consume('/'))
        return new_node('/', lhs, mul());

    if (lhs->ty == TK_NUM)
        return lhs;
    return lhs;
}

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

        // すでに使われた変数かどうか
        long offset = (long) map_get(variable_map, t->name);

        // 使われてなければ、識別子をキーとしてRBPからのオフセットを追加する
        if (offset == 0){
            offset = (variables + 1) * 8;

            map_put(variable_map, t->name, (void *) offset);
            variables++;
        }
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
