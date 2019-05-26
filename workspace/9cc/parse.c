#include "9cc.h"

int pos = 0;
static Type int_ty = {INT, NULL};
static Type char_ty = {CHAR, NULL};
//int variables = 0;
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

Node *new_node_str(char *str) {
    Node *node = malloc(sizeof(Node));
    node->ty = &char_ty;
    node->op = ND_STR;
    node->data = str;
    node->len = strlen(str) + 1;
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

Node *primary();

static Type* read_array(Type *ty) {
    Vector *v = new_vector();
    while(consume('[')) {
        Node *len = primary();
        if (len->op != ND_NUM)
            error("number expected");
        vec_push(v, len);
        expect(']');
    }
    for (int i = v->len - 1;  i >= 0; i--) {
        Node *len = v->data[i];
        ty = ary_of(ty, len->val);
    }
    return ty;
}

Node *primary() {
    Token *t = tokens->data[pos];
    if (t->ty == TK_NUM){
        return new_node_num(((Token *)tokens->data[pos++])->val);
    }

    if (t->ty == TK_STR) {
        return new_node_str(((Token *) tokens->data[pos++])->str);
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

static Node *postfix() {
    Node *lhs = primary();
    while(consume('[')) {
        Node *node = calloc(1, sizeof(Node));
        node->op = ND_DEREF;
        node->lhs = new_node('+', lhs, primary());
        lhs = node;
        expect(']');
        consume(']');
    }
    return lhs;
}

Node *mul();
Node *unary() {
    if (consume('*')) {
        Node *node = calloc(1, sizeof(Node));
        node->op = ND_DEREF;
        node->lhs = mul();
        node->name = node->lhs->name;
        return node;
    }

    if (consume('&')) {
        Node *node = calloc(1, sizeof(Node));
        node->op = ND_ADDR;
        node->lhs = mul();
        return node;
    }

    if (consume(TK_SIZEOF)) {
        Node *node = calloc(1, sizeof(Node));
        node->op = ND_SIZEOF;
        node->lhs = unary();
        return node;
    }
    return postfix();
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

Node *logand() {
    Node *node = equality();
    for (;;) {
        Token *t = tokens->data[pos];
        if (t->ty != TK_LOGAND)
            return node;
        pos++;
        node = new_node(ND_LOGAND, node, equality());
    }
}

Node *logor() {
    Node *node = logand();
    for (;;) {
        Token *t = tokens->data[pos];
        if (t->ty != TK_LOGOR)
            return node;
        pos++;
        node = new_node(ND_LOGOR, node, logand());
    }
}

Node *assign() {
    Node *node = logor();
    if (consume('='))
        return new_node('=', node, logor());
    return node;
}

static Type *type() {
    Token *t = tokens->data[pos];
    if (t->ty != TK_INT && t->ty != TK_CHAR)
        error("typename expected, but got %s", t->input);

    Type *ty;
    if (t->ty == TK_INT)
        ty = &int_ty;
    if (t->ty == TK_CHAR)
        ty = &char_ty;

    pos++;

    while(consume('*'))
        ty = ptr_of(ty);
    return ty;
}

Node *decl() {
    Node *node = calloc(1, sizeof(Node));
    node->op = ND_VARDEF;
    node->ty = type();
    Token *t = (Token *) tokens->data[pos++];
    if (t->ty != TK_IDENT)
        error("variable name expected, but got %s", t->input);
    node->name = t->name;
    node->ty = read_array(node->ty);
    if(consume('=')) {
        node->init = assign();
    }
    return node;
}

Node *param() {
    Node *node = calloc(1, sizeof(Node));
    node->op = ND_VARDEF;
    node->ty = type();

    Token *t = tokens->data[pos];
    if (t->ty != TK_IDENT)
        error("parameter name expected, but got %s", t->input);
    node->name = t->name;
    pos++;
    return node;
}

Node *stmt() {
    Node *node;
    Token *t = tokens->data[pos];

    if (t->ty == TK_INT || t->ty == TK_CHAR)  {
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
        node->op = ND_COMP_STMT;
        node->stmts  = block_items;
        return node;
    }

    if (consume(TK_RETURN)) {
        node = malloc(sizeof(Node));
        node->op = ND_RETURN;
        node->lhs = assign();
    } else {
        node = malloc(sizeof(Node));
        node->op = ND_EXPR_STMT;
        node->lhs = assign();
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

Node *toplevel() {
    Type *ty = type();
    if (!ty) {
        Token *t = tokens->data[pos];
        error("typename expected, but got %s", t->input);
    }

    Token *t = (Token *) tokens->data[pos];
    if (!consume(TK_IDENT))
        error("variable / function name expected, but got %s", t->input);

    char *name = t->name;

    // Fuction
    if (consume('(')) {
        Node *node = calloc(1, sizeof(Node));
        node->name = name;
        node->op = ND_FUNC;
        node->args = new_vector();
        if (!consume(')')) {
            vec_push(node->args, param());
            while(consume(','))
                vec_push(node->args, param());
            expect(')');
        }
        expect('{');
        node->body = compound_stmt();
        return node;
    }

    //Global variables
    Node *node = calloc(1, sizeof(Node));
    node->name = name;
    node->op = ND_VARDEF;
    node->ty = read_array(ty);
    node->data = calloc(1, size_of(node->ty));
    node->len = size_of(node->ty);
    expect(';');

    return node;
}

Vector *parse() {
    pos = 0;
    Vector *v = new_vector();
    while(((Token *)tokens->data[pos])->ty != TK_EOF)
        vec_push(v, toplevel());
    return v;
}

