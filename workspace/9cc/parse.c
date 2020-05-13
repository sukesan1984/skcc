#include "9cc.h"

typedef struct Env {
    Map *tags;
    Map *typedefs;
    struct Env *next;
} Env;

int pos = 0;
struct Env *env;
static Node null_stmt = {ND_NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, 0, 0, 0, false};

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
Node *expr();

static Type* read_array(Type *ty) {
    Vector *v = new_vector();
    while(consume('[')) {
        Node *len = expr();
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

static Node *compound_stmt();

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
                Node *node = assign();
                vec_push(args, (void *) node);
            }
            return new_node_func(t->name, args);
        }

        return new_node_ident(t->name);
    }

    if(consume('(')) {
        if (consume('{')) {
            Node *node = calloc(1, sizeof(Node));
            node->op = ND_STMT_EXPR;
            node->lhs = compound_stmt();
            expect(')');
            return node;
        }
        Node *node = expr();
        expect(')');
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
            lhs = new_expr(ND_DEREF, new_node('+', lhs, assign()));
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

    // ++ ident
    if (consume(TK_INC)) {
        Node *node = new_expr(ND_PREINC, unary());
        return node;
    }

    if (consume('-')) {
        Node *node = new_expr(ND_NEG, unary());
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

    if(consume('%'))
        return new_node('%', lhs, mul());

    if (lhs->op == ND_NUM)
        return lhs;
    return lhs;
}

Node *add() {
    Node *lhs = mul();
    for(;;) {
        if(consume('+'))
            lhs =  new_node('+', lhs, mul());
        else if(consume('-'))
            lhs = new_node('-', lhs, mul());
        else
            return lhs;
    }
}

Node *shift() {
    Node *lhs = add();
    for(;;) {
        if(consume(TK_LSHIFT))
            lhs = new_node(ND_LSHIFT, lhs, add());
        else if (consume(TK_RSHIFT))
            lhs = new_node(ND_RSHIFT, lhs, add());
        else
            return lhs;
    }
}


Node *relational() {
    Node *lhs = shift();
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

Node* bit_and() {
    Node *lhs = equality();
    for(;;) {
        if (consume('&'))
            return new_node('&', lhs, bit_and());
        else
            return lhs;
    }
}

Node *exclusive_or() {
    Node *lhs = bit_and();
    for(;;) {
        if (consume('^'))
            lhs = new_node('^', lhs, exclusive_or());
        else
            return lhs;
    }
}

Node* inclusive_or() {
    Node *lhs = exclusive_or();
    for(;;) {
        if (consume('|'))
            lhs = new_node('|', lhs, inclusive_or());
        else
            return lhs;
    }
}

Node *logand() {
    Node *node = inclusive_or();
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

Node *conditional() {
    Node *condition = logor();
    if (!consume('?'))
        return condition;
    Node *node = calloc(1, sizeof(Node));
    node->op = '?';
    node->cond = condition;
    node->if_body = expr();
    expect(':');
    node->else_body = conditional();
    return node;
}

Node *assign() {
    Node *node = conditional();
    if (consume('='))
        return new_node('=', node, conditional());
    return node;
}

Node *expr() {
    Node *lhs = assign();
    if (!consume(','))
        return lhs;

    return new_node(',', lhs, expr());
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
        node->init = expr();
    }
    expect(';');
    return node;
}

Node *expr_stmt() {
    Node *node = new_expr(ND_EXPR_STMT, expr());
    expect(';');
    return node;
}

Node *param() {
    Node *node = calloc(1, sizeof(Node));
    node->op = ND_VARDEF;
    node->ty = read_type();
    while (consume('*'))
        node->ty = ptr_to(node->ty);
    node->name = ident();
    return node;
}


static Node* stmt();
Node *control() {
    if (consume(TK_IF)) {
        Node *if_node = calloc(1, sizeof(Node));
        if_node->op = ND_IF;
        if(consume('(')) {
            Node *cond = expr(); // if/while分のカッコ内の処理
            Token *t = tokens->data[pos];
            if (t->ty != ')') {
                error("ifは閉じ括弧で閉じる必要があります: %s", t->input);
            }
            pos++;
            if_node->cond = cond;
            if_node->if_body = control();
        }
        if (consume(TK_ELSE)){
            if_node->else_body = stmt();
        }
        return if_node;
    }

    if (consume(TK_WHILE)) {
        if(consume('(')) {
            Node *node = expr(); // if/while分のカッコ内の処理
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
            Node *lhs = expr();
            if(!consume(';'))
                error("forの中に;が一つもありません: %s", ((Token *) tokens->data[pos])->input);
            Node *lhs2 = expr();
            if(!consume(';'))
                error("forの中には;が2つ必要です: %s", ((Token *) tokens->data[pos])->input);

            Node *lhs3 = expr();
            if (!consume(')'))
                error("forは閉じ括弧で閉じる必要があります: %s", ((Token *) tokens->data[pos])->input);
            return new_node_for(ND_FOR, lhs, lhs2, lhs3, control());
        }
    }
    return stmt();
}

Node *stmt() {
    if (is_typename())
        return decl();
    Node *node = calloc(1, sizeof(Node));
    Token *t = tokens->data[pos];

    switch (t->ty) {
        case TK_IF:
        case TK_WHILE:
        case TK_FOR:
            return control();
        case TK_RETURN:
            pos++;
            node->op = ND_RETURN;
            node->lhs = expr();
            expect(';');
            return node;
        case '{': {
            pos++;
            Vector* block_items = new_vector();
            env = new_env(env);
            while (!consume('}')) {
                vec_push(block_items, (void *) stmt());
            }
            env = env->next;
            node->op = ND_COMP_STMT;
            node->stmts  = block_items;
            return node;
        }
        case TK_TYPEDEF:
            pos++;
            node = decl();
            map_put(env->typedefs, node->name, node->ty);
            return &null_stmt;
        default:
            return expr_stmt();
    }
}


Node* compound_stmt() {
    Node *node = calloc(1, sizeof(Node));
    node->op = ND_COMP_STMT;
    node->stmts = new_vector();
    while(!consume('}'))
        vec_push(node->stmts, stmt());
    return node;
}


Node *toplevel() {
    bool is_extern = false;
    if (consume(TK_EXTERN))
        is_extern = true;
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
    node->is_extern = is_extern;
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
    fprintf(stderr, "parse succeeded\n");
    return v;
}

