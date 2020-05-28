// C preprocessor

#include "9cc.h"

static Map *macros;

typedef struct Context {
    Vector *input;
    Vector *output;
    int pos;
    struct Context *next;
} Context;

typedef struct CondIncl CondIncl;
struct CondIncl {
    CondIncl *next;
    enum { IN_THEN, IN_ELSE, IN_ELIF } ctx;
    bool included;
};

static CondIncl *cond_incl;
static Context *ctx;

static Context *new_ctx(Context *next, Vector *input) {
    Context * c = calloc(1, sizeof(Context));
    c->input = input;
    c->pos = 0;
    c->output = new_vector();
    c->next = next;
    return c;
}

enum {
    OBJLIKE,
    FUNCLIKE,
};

typedef struct Macro {
    int ty;
    Vector *tokens;
    Vector *params;
    bool deleted;
} Macro;

static Macro *new_macro(int ty, char *name) {
    Macro *m = calloc(1, sizeof(Macro));
    m->ty = ty;
    m->deleted = false;
    m->tokens = new_vector();
    m->params = new_vector();
    map_put(macros, name, m);
    return m;
}

static void append(Vector *v) {
    for (int i = 0; i < v->len; i++) {
        vec_push(ctx->output, v->data[i]);
    }
}

static void add(Token *t) {
    vec_push(ctx->output, t);
}

static Token* next() {
    assert(ctx->pos < ctx->input->len);
    return ctx->input->data[ctx->pos++];
}

static bool eof() {
    return ctx->pos == ctx->input->len;
}

static Token *get(int ty, char *msg) {
    Token *t = next();
    if (t->ty != ty)
        bad_token(t, msg);
    return t;
}

static char *ident(char *msg) {
    Token *t = get(TK_IDENT, msg);
    return t->name;
}

static Token *peek() {
    return ctx->input->data[ctx->pos];
}

static bool consume(int ty) {
    if (peek()->ty != ty)
        return false;
    ctx->pos++;
    return true;
}

static Vector *read_until_eol() {
    Vector *v = new_vector();
    while (!eof()) {
        Token *t = next();
        if (t->ty == '\n')
            break;
        vec_push(v, t);
    }
    return v;
}

static void skip_until_eol() {
    while (!eof()) {
        Token *t = next();
        if (t->ty == '\n')
            break;
    }
}

static Token *new_int(int val) {
    Token *t = calloc(1, sizeof(Token));
    t->ty = TK_NUM;
    t->val = val;
    return t;
}

static Token *new_param(int val) {
    Token *t = calloc(1, sizeof(Token));
    t->ty = TK_PARAM;
    t->val = val;
    return t;
}

static bool is_ident(Token *t, char *s) {
    return t->ty == TK_IDENT && !strcmp(t->name, s);
}

static CondIncl *push_cond_incl(bool included) {
    CondIncl *ci = calloc(1, sizeof(CondIncl));
    ci->next = cond_incl;
    ci->ctx = IN_THEN;
    ci->included = included;
    cond_incl = ci;
    return ci;
}

static void replace_param(Macro *m) {
    Vector *params = m->params;
    Vector *tokens = m->tokens;

    // Replaces macro parameter tokens with TK_PARAM tokens.
    Map *map = new_map();
    for (int i = 0; i < params->len; i++) {
        char *name = params->data[i];
        map_puti(map, name, i);
    }

    for (int i = 0; i < tokens->len; i++) {
        Token *t = tokens->data[i];
        if (t->ty != TK_IDENT)
            continue;
        int n = map_geti(map, t->name, -1);
        if (n == -1)
            continue;
        tokens->data[i] = new_param(n);
    }

    // Proces '#' followed by a macro parameter.
    Vector *v = new_vector();
    for (int i = 0; i < tokens->len; i++) {
        Token *t1 = tokens->data[i];
        Token *t2 = tokens->data[i+1];

        if (i != tokens->len - 1 && t1->ty == '#' && t2->ty == TK_PARAM) {
            t2->stringize = true;
            vec_push(v, t2);
            i++;
        } else {
            vec_push(v, t1);
        }
    }
    m->tokens = v;
}

static Vector *read_one_arg() {
    Vector *v = new_vector();
    Token *start = peek();
    int level = 0;

    while(!eof()) {
        Token *t = peek();
        if (level == 0)
            if (t->ty == ')' || t->ty == ',')
                return v;

        next();
        if (t->ty == '(')
            level++;
        else if (t->ty == ')')
            level--;
        vec_push(v, t);
    }
    bad_token(start, "unclosed macro argument");
    return NULL;
}

static Vector *read_args() {
    Vector *v = new_vector();
    if (consume(')'))
        return v;
    vec_push(v, read_one_arg());
    while (!consume(')')) {
        get(',', "comma expected");
        vec_push(v, read_one_arg());
    }
    return v;
}

static Token *get_token(int pos) {
    assert(pos < ctx->input->len);
    return ctx->input->data[pos];
}

static char *join_tokens(int start, int end) {
    int len = 1;
    for (int i = start; i <= end; i++) {
        Token *t = get_token(i);
        len += t->len;
    }

    char *buf = malloc(len);
    int pos = 0;
    for (int i = start; i <= end; i++) {
        Token *t = get_token(i);
        strncpy(buf + pos, t->input, t->len);
        pos += t->len;
    }
    buf[pos] = '\0';
    return buf;
}

static Token *stringize(Vector *tokens) {
    StringBuilder *sb = new_sb();

    for (int i = 0; i< tokens->len; i++) {
        Token *t = tokens->data[i];
        if (i)
            sb_add(sb, ' ');
        sb_append(sb, tokstr(t));
    }

    Token *t = calloc(1, sizeof(Token));
    t->ty = TK_STR;
    t->str = sb_get(sb);
    t->len = sb->len;
    return t;
}

static void apply(Macro *m, Token *start) {
    if (m->ty == OBJLIKE) {
        append(m->tokens);
        return;
    }

    // Function-like macro
    get('(', "( expected");
    Vector *args = read_args();
    if (m->params->len != args->len)
        bad_token(start, "number of parameter does not match");

    for (int i = 0; i < m->tokens->len; i++) {
        Token *t = m->tokens->data[i];

        if (is_ident(t, "__LINE__")) {
            add(new_int(line(t)));
            continue;
        }

        if (t->ty == TK_PARAM) {
            if (t->stringize)
                add(stringize(args->data[t->val]));
            else
                append(args->data[t->val]);
            continue;
        }

        add(t);
    }
}

static void funclike_macro(char *name) {
    Macro *m = new_macro(FUNCLIKE, name);
    vec_push(m->params, ident("parameter name expected"));
    while(!consume(')')) {
        get(',', "comma expected");
        vec_push(m->params, ident("parameter name expected"));
    }
    m->tokens = read_until_eol();
    replace_param(m);
}

static void objlike_macro(char *name) {
    Macro *m = new_macro(OBJLIKE, name);
    m->tokens = read_until_eol();
}

static void define() {
    char *name = ident("macro name expected");
    if (consume('('))
        return funclike_macro(name);
    return objlike_macro(name);
}

static void undef() {
    char *name = ident("macro name must be an identifier");
    Macro *m = new_macro(OBJLIKE, name);
    m->tokens = read_until_eol();
    m->deleted = true;
}

static char *join_paths(char *dir, char *file) {
    char *buf = malloc(strlen(dir) + strlen(file) + 2);
    sprintf(buf, "%s/%s", dir, file);
    return buf;
}

static bool file_exists(char *path) {
    struct stat st;
    return !stat(path, &st);
}

static char *search_include_paths(char *filename) {
    // Search a file from the include paths.
    for (char **p = include_paths; *p; p++) {
        char *path = join_paths(*p, filename);
        if (file_exists(path))
            return path;
    }
    error("%s: file not found", filename);
}

static void include() {
    Token *t = next();//get(TK_STR, "string expected");
    if (t->ty == TK_STR) {
        char *path = t->str;
        get('\n', "newline expected");
        append(tokenize(path, false));
    }

    if (t->ty == '<') {
        int start = ctx->pos;
        int end = ctx->pos;
        t = next();
        for (; t->ty != '>'; t = next()) {
            if (t->ty == TK_EOF)
                fprintf(stderr, "expected '>'");
            end += 1;
        }
        char *path = search_include_paths(join_tokens(start, end - 1));
        append(tokenize(path, false));
    }
}

// Skip until next `#endif`.
static void skip_cond_incl2() {
    fprintf(stderr, "無視される行のif\n");
    skip_until_eol(); // TK_IF行を消費
    while (!eof()) {
        int i = ctx->pos;
        Token *t1 = ((Token*)ctx->input->data[i]);
        Token *t2 =  ((Token*)ctx->input->data[i+1]);
        if (t1->ty == '#' && t2->ty == TK_IF) {
            skip_cond_incl2();
            return;
        }

        if (t1->ty == '#' && t2->ty == TK_ELSE) {
            skip_until_eol();
            continue;
        }
        if (t1->ty == '#' && strcmp(t2->name, "endif") == 0) {
            skip_until_eol(); // endifがくるまで
            return;
        }
        next();
    }
}

// Skip until next `#else`, `#elif` or `#endif`
// Nested `#if` and `#endif` are skipped.
static void skip_cond_incl() {
    while (!eof()) {
        int i = ctx->pos;
        Token *t1 = ((Token*)ctx->input->data[i]);
        Token *t2 =  ((Token*)ctx->input->data[i+1]);
        if (t1->ty == '#' && t2->ty == TK_IF) {
            skip_cond_incl2();
            continue;
        }
        if (t1->ty == '#' &&
                (t2->ty == TK_ELSE ||
                 (strcmp(t2->name, "endif") == 0) ||
                 (strcmp(t2->name, "elif") == 0)))
            return;
        next();
    }
}

static Vector* copy_until_eol() {
    Vector *_tokens = ctx->input;
    int _pos = ctx->pos;
    Vector *copied = new_vector();
    while (_pos < ctx->input->len) {
        Token *t = _tokens->data[_pos];
        if (t->ty == '\n')
            break;
        vec_push(copied, t);
        _pos++;
    }
    Token *eof = malloc(sizeof(Token));
    eof->ty = TK_EOF;
    vec_push(copied, eof);
    return copied;
}

static long eval_const_expr() {
    Vector *_tokens = preprocess(copy_until_eol());
    int _pos = 0;
    long expr = const_expr_token(_tokens, _pos);
    return expr;
}

Vector *preprocess(Vector *tokens) {
    if (!macros)
        macros = new_map();
    ctx = new_ctx(ctx, tokens);

    while(!eof()) {
        Token *t = next();
        if (t->ty == TK_IDENT) {
            Macro *m = map_get(macros, t->name);
            m = (m && m->deleted) ? NULL : m;
            if (m)
                apply(m, t);
            else
                add(t);
            continue;
        }

        if (t->ty != '#') {
            add(t);
            continue;
        }

        t = next();
        if (t->ty != TK_IDENT && t->ty != TK_IF && t->ty != TK_ELSE)
            bad_token(t, "identifier or if is expected");
        if (t->ty == TK_IF) {
            long expr = eval_const_expr();
            push_cond_incl(expr);
            if (!expr)
                skip_cond_incl();
            else
                skip_until_eol();
            continue;
        }

        if (t->ty == TK_ELSE) {
            if (!cond_incl || cond_incl->ctx == IN_ELSE)
                bad_token(t, "stray #else");
            cond_incl->ctx = IN_ELSE;
            skip_until_eol();
            if (cond_incl->included)
                skip_cond_incl();
            continue;
        }
        if (!strcmp(t->name, "define")) {
            define();
        } else if (!strcmp(t->name, "include")) {
            include();
        } else if (!strcmp(t->name, "undef")) {
            undef();
        } else if (!strcmp(t->name, "elif")) {
            if (!cond_incl || cond_incl->ctx == IN_ELSE)
                bad_token(t, "stray #elif");
            cond_incl->ctx = IN_ELIF;

            if (!cond_incl->included) {
               if (eval_const_expr()) {
                   skip_until_eol();
                   cond_incl->included = true;
               } else {
                   skip_until_eol();
                   skip_cond_incl();
               }
            } else {
                skip_until_eol();
                skip_cond_incl();
            }
           continue;
        } else if (!strcmp(t->name, "endif")) {
            if(!cond_incl)
                bad_token(t, "stray #endif");
            cond_incl = cond_incl->next;
            skip_until_eol();
            continue;
        } else {
            bad_token(t, "unknown directive");
        }
    }
    Vector *v = ctx->output;
    ctx = ctx->next;
    return v;
}
