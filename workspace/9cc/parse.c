#include "9cc.h"

int nlabel = 1;
static int str_label = 0;
extern Vector* globals;

static Vector *switches;
static Vector *breaks;
static Vector *continues;
static Vector *tokens;

int pos = 0;
struct Env *env;
static Node null_stmt = {ND_NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0, 0, NULL, 0, NULL, NULL, 0, NULL, 0, 0, false, false, NULL};
static Env *new_env(Env *next) {
    Env *e = calloc(1, sizeof(Env));
    e->tags = new_map();
    e->typedefs = new_map();
    e->next = next;
    e->var_scope = NULL;
    return e;
}

static Type *find_typedef(char *name) {
    for (Env *e = env; e; e = e->next)
        if (map_exists(e->typedefs, name))
            return map_get(e->typedefs, name);
    return NULL;
}

static Type *find_tag(char *name) {
    for (Env *e = env; e; e = e->next)
        if (map_exists(e->tags, name))
            return map_get(e->tags, name);
    return NULL;
}

static VarScope* find_var(char *name) {
    for (Env *e = env; e; e = e->next) {
        VarScope *sc = e->var_scope;
        while(sc) {
            if (strcmp(sc->name, name) == 0)
                return sc;
            sc = sc->next;
        }
    }
    return NULL;
}

int peek(int ty) {
    Token *t = tokens->data[pos];
    if (t->ty != ty)
        return 0;
    return 1;
}

int consume(int ty) {
    Token *t = tokens->data[pos];
    if (t->ty != ty)
        return 0;
    pos++;
    return 1;
}

int expect(int ty) {
    Token *t = tokens->data[pos];
    if (!consume(ty)) {
        fprintf(stderr, "%d is expected but got %s", ty, t->input);
        exit(1);
    }
    return 1;
}

static Type *new_prim_ty(int ty, int size, bool is_unsigned) {
    Type *ret = calloc(1, sizeof(Type));
    ret->ty = ty;
    ret->size = size;
    ret->align = size;
    ret->is_unsigned = is_unsigned;
    return ret;
}

Type *void_ty() { return new_prim_ty(VOID, 0, false); }
Type *char_ty() { return new_prim_ty(CHAR, 1, false); }
Type *short_ty() { return new_prim_ty(SHORT, 2, false); }
Type *int_ty() { return new_prim_ty(INT, 4, false); }
Type *long_ty() { return new_prim_ty(LONG, 8, false); }
Type *enum_ty() { return new_prim_ty(ENUM, 4, false); }
Type *bool_ty() { return new_prim_ty(BOOL, 1, false); }

Type *uchar_ty() { return new_prim_ty(CHAR, 1, true); }
Type *ushort_ty() { return new_prim_ty(SHORT, 2, true); }
Type *uint_ty() { return new_prim_ty(INT, 4, true); }
Type *ulong_ty() { return new_prim_ty(LONG, 8, true); }

static bool is_typename(Token *t) {
    if (t->ty == TK_IDENT)
        return find_typedef(t->name);
    return t->ty == TK_INT || t->ty == TK_LONG || t->ty == TK_SHORT || t->ty == TK_CHAR || t->ty == TK_BOOL || t->ty == TK_VOID || t->ty == TK_STRUCT || t->ty == TK_ENUM || t->ty == TK_SIGNED || t->ty == TK_UNSIGNED;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->op = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_loop(int op) {
    Node *node = calloc(1, sizeof(Node));
    node->op = op;
    node->break_label = nlabel++;
    node->continue_label = nlabel++;
    return node;
}

Node *new_expr(int ty, Node *expr) {
    Node *node = calloc(1, sizeof(Node));
    node->op = ty;
    node->lhs = expr;
    return node;
}

Node *new_node_num(long val) {
    Node *node = malloc(sizeof(Node));
    node->ty = (val >> 31) ? long_ty() :  int_ty();
    node->op = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_str(Token *t) {
    Node *node = malloc(sizeof(Node));
    node->op = ND_STR;
    node->data = t->str;
    node->len = t->len;
    node->ty = ary_of(char_ty(), node->len);
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

static Node *new_desg_node2(Type *type, char *name, Designator *desg) {
    if (!desg)
        return new_node_ident(name);

    Node *node = new_desg_node2(type, name, desg->next);

    if (desg->name) {
        Node *n = calloc(1, sizeof(Node));
        n->op = ND_DOT;
        n->name = desg->name;
        n->lhs = node;
        return n;
    }
    node = new_node('+', node, new_node_num(desg->idx));
    return new_expr(ND_DEREF, node);
}

static Node *new_desg_node(Type *type, char *name, Designator *desg, Node *rhs) {
    Node *lhs = new_desg_node2(type, name, desg);
    Node *node = new_node('=', lhs, rhs);
    Node *n = calloc(1, sizeof(Node));
    n->op = ND_EXPR_STMT;
    n->lhs = node;
    return n;
}

static VarScope *push_scope(char *name) {
    VarScope *sc = calloc(1, sizeof(VarScope));
    sc->name = name;
    if (env->var_scope != NULL) {
        sc->next = env->var_scope;
        env->var_scope = sc;
    } else {
        env->var_scope = sc;
    }
    return sc;
}

static char *ident() {
    Token *t = tokens->data[pos++];
    if (t->ty != TK_IDENT) {
        fprintf(stderr, "identifier expected, but got %s ty: %d", t->input, t->ty);
        exit(1);
    }
    return t->name;
}

Node *add();
Node *assign();
Node *unary();
Node *primary();
Node *expr();

Node *conditional();
static long eval(Node *node);

static long const_expr() {
    Token *t = tokens->data[pos];
    Node *node = conditional();
    return eval(node);
}

long const_expr_token(Vector *override_tokens, int override_pos) {
    Vector *_tokens = tokens;
    int _pos = pos;
    tokens = override_tokens;
    pos = override_pos;
    Node *node = conditional();
    long result = eval(node);
    pos = _pos;
    tokens = _tokens;
    return result;
}

static Type* read_array(Type *ty) {
    Vector *v = new_vector();
    bool is_incomplete = false;
    while(consume('[')) {
        if (!consume(']')) {
            Node *len = expr();
            if (len->op != ND_NUM)
                fprintf(stderr, "number expected");
            vec_push(v, len);
            expect(']');
        } else {
            is_incomplete = true;
            ty = ary_of(ty, 0);
        }
    }
    for (int i = v->len - 1;  i >= 0; i--) {
        Node *len = v->data[i];
        ty = ary_of(ty, len->val);
    }
    ty->is_incomplete = is_incomplete;
    return ty;
}

static Type *abstract_declarator(Type *ty);

static Node *compound_stmt();
static Type *decl_specifiers();
Node *primary() {
    Token *t = tokens->data[pos];
    if (t->ty == TK_NUM){
        Node* node = new_node_num(((Token *)tokens->data[pos++])->val);
        node->ty = t->type;
        return node;
    }

    if (t->ty == TK_STR) {
        t = tokens->data[pos++];
        Node *node = calloc(1, sizeof(Node));
        node->op = ND_GVAR;
        node->ty = ary_of(char_ty(), t->len);
        char *name = format(".L.str%d", str_label++);
        node->name = name;
        Var *var = new_global(node->ty, name, t->str, t->len, false, true);
        var->initializer = gvar_init_string(t->str, t->len);
        if(!globals)
            globals = new_vector();
        vec_push(globals, var);
        return node;
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

        VarScope *sc = find_var(t->name);
        if (sc) {
            if(sc->enum_ty)
                return new_node_num(sc->enum_val);
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

    if (consume(TK_SIZEOF)) {
        Token *t1 = tokens->data[pos];
        Token *t2 = tokens->data[pos + 1];
        if (t1->ty == '(' && is_typename(t2)) {
            expect('(');
            Type *ty = decl_specifiers();
            ty = abstract_declarator(ty);
            expect(')');
            if (ty-ty == VOID) {
                fprintf(stderr, "voidはだめ\n");
                exit(1);
            }
            return new_node_num(ty->size);
        }
        Node *node = calloc(1, sizeof(Node));
        node->op = ND_SIZEOF;
        node->lhs = unary();
        return node;
    }

    if (consume(TK_ALIGNOF)) {
        Token *t1 = tokens->data[pos];
        Token *t2 = tokens->data[pos + 1];
        if (t1->ty == '(' && is_typename(t2)) {
            expect('(');
            Type *ty = decl_specifiers();
            ty = abstract_declarator(ty);
            expect(')');
            if (ty-ty == VOID) {
                fprintf(stderr, "voidはだめ\n");
                exit(1);
            }
            return new_node_num(ty->align);
        }
        Node *node = calloc(1, sizeof(Node));
        node->op = ND_ALIGNOF;
        node->lhs = unary();
        return node;
    }

    t = tokens->data[pos];
    fprintf(stderr, "数字でも開き括弧でもないトークンです: %s", t->input);
    exit(1);
}

static Node *postfix() {
    Node *lhs = primary();

    for (;;) {
        if (consume(TK_INC)) {
            lhs = new_expr(ND_POSTINC, lhs);
            continue;
        }
        if (consume(TK_DEC)) {
            lhs = new_expr(ND_POSTDEC, lhs);
            continue;
        }
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
            lhs = node;
            continue;
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
static Node* cast();
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
        Node *node = new_expr('!', cast());
        return node;
    }

    if (consume('~')) {
        Node *node = new_expr('~', cast());
        return node;
    }

    // ++ ident
    if (consume(TK_INC)) {
        Node *node = new_expr(ND_PREINC, cast());
        return node;
    }

    if (consume(TK_DEC)) {
        Node *node = new_expr(ND_PREDEC, cast());
        return node;
    }

    if (consume('-')) {
        Node *node = new_expr(ND_NEG, cast());
        return node;
    }

    return postfix();
}

static Type *typename();
static Node *cast() {
    Token *t1 = tokens->data[pos];
    Token *t2 = tokens->data[pos + 1];
    if (t1->ty == '(' && is_typename(t2)) {
        expect('(');
        Node *node = calloc(1, sizeof(Node));
        node->op = ND_CAST;
        node->ty = typename();
        expect(')');
        node->lhs = cast();
        return node;
    }
    return unary();
}


Node *mul() {
    Node *lhs = cast();
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

static int assignment_operator() {
    if(consume('='))
        return '=';
    if (consume(TK_MUL_EQ))
        return ND_MUL_EQ;
    if (consume(TK_DIV_EQ))
        return ND_DIV_EQ;
    if (consume(TK_MOD_EQ))
        return ND_MOD_EQ;
    if (consume(TK_ADD_EQ))
        return ND_ADD_EQ;
    if (consume(TK_SUB_EQ))
        return ND_SUB_EQ;
    if (consume(TK_SHL_EQ))
        return ND_SHL_EQ;
    if (consume(TK_SHR_EQ))
        return ND_SHR_EQ;
    if (consume(TK_BITAND_EQ))
        return ND_BITAND_EQ;
    if (consume(TK_XOR_EQ))
        return ND_XOR_EQ;
    if (consume(TK_BITOR_EQ))
        return ND_BITOR_EQ;
    return 0;
}

Node *assign() {
    Node *node = conditional();
    for(;;) {
        int op = assignment_operator();
        if (op)
            node = new_node(op, node, assign());
        else
            return node;
    }
}

Node *expr() {
    Node *lhs = assign();
    if (!consume(','))
        return lhs;

    return new_node(',', lhs, expr());
}

static bool peek_end() {
    bool ret = consume('}') || (consume(',') && consume('}'));
    return ret;
}

static Node *lvar_init_zero(Node *cur, Type *ty, char *name, Designator *desg) {
    if (ty->ty == ARRAY) {
        for (int i = 0; i < ty->array_size; i++) {
            Designator desg2 = {desg, i++, NULL};
            cur = lvar_init_zero(cur, ty->ptr_to, name, &desg2);
        }
        return cur;
    }
    cur->next = new_desg_node(ty, name, desg, new_node_num(0));
    return cur->next;
}

static Node *lvar_initializer2(Node *cur, Type *ty, char *name, Designator *desg) {
    if (ty->ty == ARRAY && ty->ptr_to->ty == CHAR) {
        Token* t = tokens->data[pos++];
        if (t->ty == TK_STR) {
            if (ty->is_incomplete) {
                ty->size = t->len;
                ty->array_size = t->len;
                ty->is_incomplete = false;
            }
            int len = (ty->array_size < t->len) ? ty->array_size : t->len;
            for (int i = 0; i < len; i++) {
                Designator desg2 = {desg, i, NULL};
                Node *rhs = new_node_num(t->str[i]);
                cur->next = new_desg_node(ty, name, &desg2, rhs);
                cur = cur->next;
            }

            for (int i = len; i < ty->array_size; i++) {
                Designator desg2 = {desg, i, NULL};
                cur = lvar_init_zero(cur, ty->ptr_to, name, &desg2);
            }
            return cur;
        }
    }
    if (ty->ty == ARRAY) {
        expect('{');
        int i = 0;
        if (!consume('}')) {
            do {
                Designator desg2 = { desg, i++, NULL};
                cur = lvar_initializer2(cur, ty->ptr_to, name, &desg2);
            } while (!peek_end());
        }
        while (i < ty->array_size) {
            Designator desg2 = {desg, i++, NULL};
            cur = lvar_init_zero(cur, ty->ptr_to, name, &desg2);
        }

        if (ty->is_incomplete) {
            ty->size = ty->ptr_to->size * i;
            ty->array_size = i;
            ty->is_incomplete = false;
        }

        return cur;
    }

    if (ty->ty == STRUCT) {
        expect('{');
        int i = 0;
        Vector *members = ty->members;
        if (!consume('}')) {
            do {
                Node *n = members->data[i++];
                Designator desg2 = { desg, 0, n->name };
                cur = lvar_initializer2(cur, n->ty, name, &desg2);
            } while (!peek_end());
        }
        while (i < ty->members->len) {
            Node *n = members->data[i++];
            Designator desg2 = { desg, 0, n->name };
            cur = lvar_init_zero(cur, n->ty, name, &desg2);
        }
        return cur;
    }


    Node *node = assign();
    cur->next = new_desg_node(ty, name, desg, node);
    return cur->next;
}

static Node *lvar_initializer(Type *type, char *name) {
    Node head = {};
    lvar_initializer2(&head, type, name, NULL);
    Node *node = calloc(1, sizeof(Node));
    node->op = ND_COMP_STMT;
    node->stmts = new_vector();
    Node *next = head.next;
    while(next) {
        vec_push(node->stmts, next);
        next = next->next;
    }

    return node;
}

static Node *declarator(Type *ty);
static Node *direct_decl(Type *ty) {
    Token *t = tokens->data[pos];
    Node *node;
    Type *placeholder = calloc(1, sizeof(Type));
    if (t->ty == TK_IDENT) {
        node = calloc(1, sizeof(Node));
        node->op = ND_VARDEF;
        node->ty = placeholder;
        node->name = ident();
    } else if (consume('(')) {
        node = declarator(placeholder);
        expect(')');
    } else {
        bad_token(t, "bad direct-declarator");
    }

    if (node->ty->ty == VOID) {
        fprintf(stderr, "void variable: %s", node->name);
        exit(1);
    }
    *placeholder = *read_array(ty);


    if(consume('=')) {
        node->init = lvar_initializer(node->ty, node->name);
    } else {
        if (node->ty->is_incomplete) {
            fprintf(stderr, "incomplete type: %s", node->name);
            exit(1);
        }
    }
    return node;
}

static Node *declarator(Type *ty) {
    while (consume('*'))
        ty = ptr_to(ty);
    return direct_decl(ty);
}

static Type *abstract_direct_decl(Type *ty) {
    Type *placeholder = calloc(1, sizeof(Type));
    Type *ret_type = placeholder;
    if (consume('(')) {
        ret_type = abstract_declarator(placeholder);
        expect(')');
    }
    *placeholder = *read_array(ty);

    return ret_type;
}

static Type *abstract_declarator(Type *ty) {
    while (consume('*'))
        ty = ptr_to(ty);
    return abstract_direct_decl(ty);
}

static Type *typename() {
    Type *ty = decl_specifiers();
    return abstract_declarator(ty);
}

static Node *declaration() {
    Type *ty = decl_specifiers();
    if (consume(';')) {
        return &null_stmt;
    }
    Node *node = declarator(ty);
    expect(';');
    return node;
}

static Node *param_declaration() {
    Type *ty = decl_specifiers();
    return declarator(ty);
}

Node *expr_stmt() {
    Node *node = new_expr(ND_EXPR_STMT, expr());
    expect(';');
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
                fprintf(stderr, "ifは閉じ括弧で閉じる必要があります: %s", t->input);
                exit(1);
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

    if (consume(TK_SWITCH)) {
        Node *node = new_loop(ND_SWITCH);
        node->cases = new_vector();
        expect('(');
        node->cond = expr();
        expect(')');
        vec_push(breaks, node);
        vec_push(switches, node);
        node->body = stmt();
        vec_pop(switches);
        vec_pop(breaks);
        return node;
    }

    if (consume(TK_CASE)) {
        Token *t = tokens->data[pos];
        if (switches->len == 0)
            bad_token(t, "stray case");
        Node *node = calloc(1, sizeof(Node));
        node->op = ND_CASE;
        node->case_label = nlabel++;
        node->val = const_expr();
        expect(':');
        node->body = stmt();
        Node *n = switches->data[switches->len - 1];
        vec_push(n->cases, node);
        return node;
    }

    if (consume(TK_DEFAULT)) {
        Token *t = tokens->data[pos];
        Node *node = calloc(1, sizeof(Node));
        node->op = ND_DEFAULT;
        node->case_label = nlabel++;
        expect(':');
        node->body = stmt();
        Node *n = switches->data[switches->len - 1];
        vec_push(n->cases, node);
        return node;
    }
    if (consume(TK_DO)) {
        Node *node = new_loop(ND_DO_WHILE);
        vec_push(breaks, node);
        vec_push(continues, node);
        node->op = ND_DO_WHILE;
        node->body = stmt();
        expect(TK_WHILE);
        expect('(');
        node->cond = assign();
        expect(')');
        expect(';');
        vec_pop(breaks);
        vec_pop(continues);
        return node;
    }

    if (consume(TK_WHILE)) {
        if(consume('(')) {
            Node *while_node = new_loop(ND_WHILE);
            vec_push(breaks, while_node);
            vec_push(continues, while_node);
            Node *node = expr(); // if/while分のカッコ内の処理
            Token *t = tokens->data[pos];
            if (t->ty != ')') {
                fprintf(stderr, "ifは閉じ括弧で閉じる必要があります: %s", t->input);
                exit(1);
            }
            pos++;
            while_node->lhs = node;
            while_node->rhs = control();
            vec_pop(breaks);
            vec_pop(continues);
            return while_node;
        }
    }

    if (consume(TK_FOR)) {
        if(consume('(')) {
            Node *node = new_loop(ND_FOR);
            vec_push(breaks, node);
            vec_push(continues, node);
            if (is_typename(tokens->data[pos])) {
                node->lhs = declaration();
            } else if (consume(';')){
                node->lhs = &null_stmt;
            } else {
                node->lhs = expr_stmt();
            }
            if(!consume(';')) {
                node->lhs2 = expr();
                expect(';');
            }

            if (!consume(')')) {
                node->lhs3 = expr();
                expect(')');
            }

            node->rhs = stmt();
            vec_pop(breaks);
            vec_pop(continues);
            return node;
        }
    }
    return stmt();
}

Node *stmt() {
    if (is_typename(tokens->data[pos]))
        return declaration();
    Node *node = calloc(1, sizeof(Node));
    Token *t = tokens->data[pos];

    switch (t->ty) {
        case TK_IF:
        case TK_WHILE:
        case TK_SWITCH:
        case TK_CASE:
        case TK_DEFAULT:
        case TK_FOR:
        case TK_DO:
            return control();
        case TK_RETURN:
            pos++;
            node->op = ND_RETURN;
            if (consume(';'))
                return node;
            node->lhs = expr();
            expect(';');
            return node;
        case TK_BREAK:
            if (breaks->len == 0)
                bad_token(t, "stray break");
            pos++;
            node->op = ND_BREAK;
            node->target = breaks->data[breaks->len - 1];
            expect(';');
            return node;
        case TK_CONTINUE:
            if (continues->len == 0)
                bad_token(t, "stray continue");
            pos++;
            node->op = ND_CONTINUE;
            node->target = continues->data[continues->len - 1];
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
            node = declaration();
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


static Initializer *new_init_val(Initializer *cur, int sz, int val) {
    Initializer *init = calloc(1, sizeof(Initializer));
    init->sz = sz;
    init->val = val;
    cur->next = init;
    return init;
}

static Initializer *new_init_label(Initializer *cur, char *label) {
    Initializer *init = calloc(1, sizeof(Initializer));
    init->label = label;
    cur->next = init;
    return init;
}

Initializer *gvar_init_string(char *p, int len) {
    Initializer head = {};
    Initializer *cur = &head;
    for (int i = 0; i < len; i++)
        cur = new_init_val(cur, 1, (signed char) p[i]);
    return head.next;
}

static long eval(Node *node) {
    switch(node->op) {
        case '+':
            return eval(node->lhs) + eval(node->rhs);
        case '-':
            return eval(node->lhs) - eval(node->rhs);
        case '*':
            return eval(node->lhs) * eval(node->rhs);
        case '/':
            return eval(node->lhs) / eval(node->rhs);
        case '&':
            return eval(node->lhs) & eval(node->rhs);
        case '|':
            return eval(node->lhs) | eval(node->rhs);
        case '^':
            return eval(node->lhs) ^ eval(node->rhs);
        case ND_LSHIFT:
            return eval(node->lhs) << eval(node->rhs);
        case ND_RSHIFT:
            return eval(node->lhs) >> eval(node->rhs);
        case ND_EQ:
            return eval(node->lhs) == eval(node->rhs);
        case ND_NE:
            return eval(node->lhs) != eval(node->rhs);
        case '<':
            return eval(node->lhs) < eval(node->rhs);
        case '>':
            return eval(node->lhs) > eval(node->rhs);
        case ND_LE:
            return eval(node->lhs) <= eval(node->rhs);
        case '?':
            return eval(node->cond) ? eval(node->if_body) : eval(node->else_body);
        case ',':
            return eval(node->rhs);
        case '!':
            return !eval(node->lhs);
        case '~':
            return ~eval(node->lhs);
        case ND_LOGAND:
            return eval(node->lhs) && eval(node->rhs);
        case ND_LOGOR:
            return eval(node->lhs) || eval(node->rhs);
        case ND_NUM:
            return node->val;
    }
    fprintf(stderr, "not a constant expression %d", node->op);
    exit(1);
}

static Initializer *new_init_zero(Initializer *cur, int nbytes) {
    for (int i = 0;  i < nbytes; i++)
        cur = new_init_val(cur, 1, 0);
    return cur;
}

static Initializer *gvar_initializer2(Initializer *cur, Type *ty) {
    if (ty->ty == ARRAY) {
        expect('{');
        int i = 0;

        if (!consume('}')) {
            do {
                cur = gvar_initializer2(cur, ty->ptr_to);
                i++;
            } while (!peek_end());
        }

        if (i < ty->array_size)
            cur = new_init_zero(cur, ty->ptr_to->size * (ty->array_size - i));
        if (ty->is_incomplete) {
            ty->size = ty->ptr_to->size * i;
            ty->array_size = i;
            ty->is_incomplete = false;
        }
        return cur;
    }
    if (ty->ty == STRUCT) {
        expect('{');
        int i = 0;

        Vector *members = ty->members;
        if (!consume('}')) {
            do {
                Node *node = members->data[i];
                cur = gvar_initializer2(cur, node->ty);
                int start = node->ty->offset + node->ty->size;
                int end = (members->len > (i + 1)) ? ((Node*)members->data[i+1])->ty->offset : (node->ty->offset + node->ty->size);
                cur = new_init_zero(cur, end - start);
                i++;
            } while (!peek_end());
        }
        if (members->len > i)
            cur = new_init_zero(cur, ty->size - ((Node*)members->data[i])->ty->offset);
        return cur;
    }
    Node *expr = conditional();
    if (expr->op == ND_ADDR) {
        if (expr->lhs->op != ND_IDENT) {
            fprintf(stderr, "invalid initializer");
            exit(1);
        }
        return new_init_label(cur, expr->lhs->name);
    }

    if ((expr->op == ND_VARDEF || expr->op == ND_GVAR) && expr->ty->ty == ARRAY)
        return new_init_label(cur, expr->name);

    return new_init_val(cur, ty->size, eval(expr));
}

static Initializer *gvar_initializer(Type *ty) {
    Initializer head = {};
    gvar_initializer2(&head, ty);
    return head.next;
}

Node *toplevel() {
    bool is_typedef = consume(TK_TYPEDEF);
    bool is_extern = consume(TK_EXTERN);
    bool is_static = consume(TK_STATIC);
    Type *ty = decl_specifiers();

    if (consume(';'))
        return NULL;

    while (consume('*'))
        ty = ptr_to(ty);

    char *name = ident();

    // Fuction
    if (consume('(')) {
        Node *node = calloc(1, sizeof(Node));
        bool is_variadic = false;
        switches = new_vector();
        breaks = new_vector();
        continues = new_vector();
        node->name = name;
        node->args = new_vector();
        node->ty = calloc(1, sizeof(Type));
        node->ty->ty = FUNC;
        node->ty->returning = ty;
        if (!consume(')')) {
            vec_push(node->args, param_declaration());
            while(consume(',')) {
                if (consume(TK_DOTS)) {
                    is_variadic = true;
                    break;
                }
                vec_push(node->args, param_declaration());
            }
            node->ty->is_variadic = is_variadic;
            expect(')');
        }

        if (consume(';')) {
            node->op = ND_DECL;
            return node;
        }

        node->op = ND_FUNC;
        node->is_static = is_static;
        expect('{');
        node->body = compound_stmt();
        return node;
    }

    ty = read_array(ty);

    if (is_typedef) {
        expect(';');
        map_put(env->typedefs, name, ty);
        return NULL;
    }

    //Global variables
    Node *node = calloc(1, sizeof(Node));
    node->name = name;
    node->op = ND_VARDEF;
    node->ty = ty;
    node->is_extern = is_extern;
    node->is_static = is_static;
    node->data = calloc(1, node->ty->size);
    node->len = node->ty->size;
    if (consume('='))
        node->initializer = gvar_initializer(ty);
    expect(';');

    return node;
}

static void add_members(Type *ty, Vector *members) {
    int off = 0;
    for (int i = 0; i < members->len; i++) {
        Node *node = members->data[i];
        assert(node->op == ND_VARDEF);

        Type *t = node->ty;
        off = roundup(off, t->align);
        t->offset = off;
        off += t->size;
        if (ty->align < node->ty->align)
            ty->align = node->ty->align;
    }
    ty->members = members;
    ty->size = roundup(off, ty->align);
}

static Type *struct_decl() {
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
            vec_push(members, declaration());
    }

    if (!tag && !members) {
        fprintf(stderr, "bad struct definition");
        exit(1);
    }

    Type *ty = NULL;
    if (tag)
        ty = find_tag(tag);
    if (!ty) {
        ty = calloc(1, sizeof(Type));
        ty->ty = STRUCT;
        if (tag)
            map_put(env->tags, tag, ty);
    }

    if (members)
        add_members(ty, members);
    return ty;
}

static Type *enum_decl() {
    pos++;
    Type *ty = enum_ty();
    char *tag = NULL;
    Token *t = tokens->data[pos];

    if (t->ty == TK_IDENT) {
        pos++;
        tag = t->name;
    }
    if (tag && !peek('{')) {
        ty = find_tag(tag);
        if (!ty) {
            fprintf(stderr, "unknown enum type");
            exit(1);
        }
        if (ty->ty != ENUM) {
            fprintf(stderr, "not an enum tag");
            exit(1);
        }
        return ty;
    }

    expect('{');

    long cnt = 0;
    VarScope *sc;
    for (;;) {
        char *name = ident();
        if (consume('='))
            cnt = const_expr();
        sc = push_scope(name);
        sc->enum_ty = ty;
        sc->enum_val = cnt++;
        if (peek_end())
            break;
    }
    if (sc) {
        if (tag)
            map_put(env->tags, tag, ty);
    }
    return ty;

}

static Type *decl_specifiers() {
    Token *t = tokens->data[pos];
    if (t->ty != TK_INT && t->ty != TK_LONG && t->ty != TK_SHORT && t->ty != TK_CHAR && t->ty != TK_STRUCT && t->ty != TK_IDENT && t->ty != TK_VOID && t->ty != TK_BOOL && t->ty != TK_ENUM && t->ty != TK_SIGNED && t->ty != TK_UNSIGNED) {
        fprintf(stderr, "typename expected, but got %s", t->input);
        exit(1);
    }

    if (t->ty == TK_IDENT) {
        Type *ty = find_typedef(t->name);
        if (ty)
            pos++;
        return ty;
    }
    Type *ty = int_ty();
    int counter = 0;
    while (is_typename(tokens->data[pos])) {
        t = tokens->data[pos];
        if (t->ty == TK_STRUCT || t->ty == TK_ENUM) {
            if (t->ty == TK_STRUCT)
                return struct_decl();
            else
                return enum_decl();
        }
        pos++;
        if (t->ty == TK_VOID)
            counter += VOID;
        else if (t->ty == TK_BOOL)
            counter += BOOL;
        else if (t->ty == TK_CHAR)
            counter += CHAR;
        else if (t->ty == TK_SHORT)
            counter += SHORT;
        else if (t->ty == TK_INT)
            counter += INT;
        else if (t->ty == TK_LONG)
            counter += LONG;
        else if (t->ty == TK_SIGNED)
            counter |= SIGNED;
        else if (t->ty == TK_UNSIGNED)
            counter |= UNSIGNED;
        else
            bad_token(t, "typename expected");
        switch (counter) {
            case VOID:
                ty = void_ty();
                break;
            case BOOL:
                ty = bool_ty();
                break;
            case CHAR:
            case SIGNED + CHAR:
                ty = char_ty();
                break;
            case UNSIGNED + CHAR:
                ty = uchar_ty();
                break;
            case SHORT:
            case SHORT + INT:
            case SIGNED + SHORT:
            case SIGNED + SHORT + INT:
                ty = short_ty();
                break;
            case UNSIGNED + SHORT:
            case UNSIGNED + SHORT + INT:
                ty = ushort_ty();
                break;
            case INT:
            case SIGNED:
            case SIGNED + INT:
                ty = int_ty();
                break;
            case UNSIGNED:
            case UNSIGNED + INT:
                ty = uint_ty();
                break;
            case LONG:
            case LONG + INT:
            case LONG + LONG:
            case LONG + LONG + INT:
            case SIGNED + LONG:
            case SIGNED + LONG + INT:
            case SIGNED + LONG + LONG:
            case SIGNED + LONG + LONG + INT:
                ty = long_ty();
                break;
            case UNSIGNED + LONG:
            case UNSIGNED + LONG + INT:
            case UNSIGNED + LONG + LONG:
            case UNSIGNED + LONG + LONG + INT:
                ty = ulong_ty();
                break;
            default:
                fprintf(stderr, "counter: %d", counter);
                bad_token(t, "invalid type.");
        }
    }
    return ty;
}

Vector *parse(Vector *tokens_) {
    tokens = tokens_;
    pos = 0;
    Vector *v = new_vector();
    env = new_env(env);
    while(((Token *)tokens->data[pos])->ty != TK_EOF) {
        Node *node = toplevel();
        if (node)
            vec_push(v, node);
    }
    fprintf(stderr, "parse succeeded\n");
    return v;
}

