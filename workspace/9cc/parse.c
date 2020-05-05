#include "9cc.h"

typedef struct Env {
    Map *tags;
    Map *typedefs;
    struct Env *next;
} Env;

int pos = 0;
struct Env *env;
static Node null_stmt = {ND_NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, 0, 0, 0};

static Env *new_env(Env *next) {
    Env *env = calloc(1, sizeof(Env));
    env->tags = new_map();
    env->typedefs = new_map();
    env->next = next;
    return env;
}

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

static Type *new_prim_ty(int ty, int size) {
    Type *ret = calloc(1, sizeof(Type));
    ret->ty = ty;
    ret->size = size;
    ret->align = size;
    return ret;
}

static Type *void_ty() { return new_prim_ty(VOID, 0); }
static Type *char_ty() { return new_prim_ty(CHAR, 1); }
static Type *int_ty() { return new_prim_ty(INT, 4); }

static bool is_typename() {
    Token *t = tokens->data[pos];
    if (t->ty == TK_IDENT)
        return map_exists(env->typedefs, t->name);
    return t->ty == TK_INT || t->ty == TK_CHAR || t->ty == TK_VOID || t->ty == TK_STRUCT;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->op = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_expr(int ty, Node *expr) {
    Node *node = calloc(1, sizeof(Node));
    node->op = ty;
    node->lhs = expr;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = int_ty();
    node->op = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_str(char *str) {
    Node *node = malloc(sizeof(Node));
    node->ty = char_ty();
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

Node *new_node_func(char *name, Vector *args) {
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

static char *ident() {
    Token *t = tokens->data[pos++];
    if (t->ty != TK_IDENT)
        error("identifier expected, but got %s", t->input);
    return t->name;
}

Node *add();
Node *assign();
Node *unary();
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
            return new_node_func(t->name, args);
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

    for (;;) {
        if(consume('.')) {
            Node *node = calloc(1, sizeof(Node));
            node->op = ND_DOT;
            node->lhs = lhs;
            node->name = ident();
            lhs = node;
            continue;
        }

        if (consume(TK_ARROW)) {
            Node *node = calloc(1, sizeof(Node));
            node->op = ND_DOT;
            node->lhs = new_expr(ND_DEREF, lhs);
            node->name = ident();
            return node;
        }
        if (consume('[')) {
            lhs = new_expr(ND_DEREF, new_node('+', lhs, primary()));
            expect(']');
            continue;
        }
        return lhs;
    }
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

    if (consume('!')) {
        Node *node = new_expr('!', unary());
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

static Type *read_type();

Node *decl() {
    Node *node = calloc(1, sizeof(Node));
    node->op = ND_VARDEF;
    node->ty = read_type();
    while (consume('*'))
        node->ty = ptr_to(node->ty);
    node->name = ident();
    node->ty = read_array(node->ty);
    if (node->ty->ty == VOID)
        error("void variable: %s", node->name);
    if(consume('=')) {
        node->init = assign();
    }
    expect(';');
    return node;
}

Node *param() {
    Node *node = calloc(1, sizeof(Node));
    node->op = ND_VARDEF;
    node->ty = read_type();
    node->name = ident();
    return node;
}

Node *stmt() {
    Node *node;

    if (is_typename())  {
        node = decl();
        return node;
    }
    if (consume('{')) {
        Vector* block_items = new_vector();
        env = new_env(env);
        while (!consume('}')) {
            vec_push(block_items, (void *) stmt());
        }
        env = env->next;
        node = malloc(sizeof(Node));
        node->op = ND_COMP_STMT;
        node->stmts  = block_items;
        return node;
    }

    if (consume(TK_TYPEDEF)) {
        Node *node = decl();
        map_put(env->typedefs, node->name, node->ty);
        return &null_stmt;
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
    Type *ty = read_type();
    if (!ty) {
        Token *t = tokens->data[pos];
        error("typename expected, but got %s", t->input);
    }

    char *name = ident();

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
    node->data = calloc(1, node->ty->size);
    node->len = node->ty->size;
    expect(';');

    return node;
}

static Type *read_type() {
    Token *t = tokens->data[pos];
    if (t->ty != TK_INT && t->ty != TK_CHAR && t->ty != TK_STRUCT && t->ty != TK_IDENT && t->ty != TK_VOID)
        error("typename expected, but got %s", t->input);

    if (t->ty == TK_IDENT) {
        Type *ty = map_get(env->typedefs, t->name);
        if (ty)
            pos++;
        return ty;
    }
    if (t->ty == TK_INT)
    {
        pos++;
        return int_ty();
    }
    if (t->ty == TK_CHAR)
    {
        pos++;
        return char_ty();
    }

    if (t->ty == TK_VOID)
    {
        pos++;
        return void_ty();
    }
    if (t->ty == TK_STRUCT)
    {
        pos++;

        char *tag = NULL;
        Token *t = tokens->data[pos];

        if (t->ty == TK_IDENT) {
            pos++;
            tag = t->name;
        }
        Vector *members = NULL;
        if (consume('{')) {
            members = new_vector();
            while (!consume('}'))
                vec_push(members, decl());
        }

        if (!tag && !members)
            error("bad struct definition");

        if (tag && members) {
            map_put(env->tags, tag, members);
        } else if (tag && !members) {
            members = map_get(env->tags, tag);
            t = tokens->data[pos];
            if (!members)
                error("incomplete type: %s input: %s", tag, t->input);
        }

        return struct_of(members);
    }
    return NULL;
}

Vector *parse() {
    pos = 0;
    Vector *v = new_vector();
    env = new_env(env);
    while(((Token *)tokens->data[pos])->ty != TK_EOF)
        vec_push(v, toplevel());
    return v;
}

