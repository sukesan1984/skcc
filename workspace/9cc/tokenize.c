#include "9cc.h"

static char *buf;
static char *filename;
static Vector *tokens;

void error_token(int i){
    Token *t = tokens->data[i];
    error("予期せぬトークンです:", t->input);
}

char *tokstr(Token *t) {
    assert(t->start && t->end);
    return strndup(t->start, t->end - t->start);
}

int line(Token *t) {
    int n = 0;
    for (char *p = t->buf; p < t->end; p++)
        if (*p == '\n')
            n++;
    return n;
}

static bool startswith(char *p, char *q) {
    return strncmp(p, q, strlen(q)) == 0;
}

void show_tokens(Vector *tokens) {
    fprintf(stderr, "show_tokens...\n");
    for (int i = 0; i < tokens->len; i++) {
        fprintf(stderr, "%s", tokstr(tokens->data[i]));
    }
}

Token *add_token(Vector *v, int ty, char *input) {
    Token * token = malloc(sizeof(Token));
    token->ty = ty;
    token->buf = buf;
    token->start = input;
    token->filename = filename;
    token->input = input;
    token->has_space = false;
    vec_push(v, (void *)token);
    return token;
}

void add_space(Vector *v) {
    if (v->len == 0)
        return;
    Token *last = v->data[v->len - 1];
    last->has_space = true;
}

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

int tokenize_comparable(Vector* tokens, int ty, char *p, char* token) {
    int len = strlen(token);
    if (strncmp(p, token, len) == 0) {
        Token * t = add_token(tokens, ty, p);
        t->end = (p + len);
        t->len = len;
        return 1;
    }
    return 0;
}

static char *c_char(long *res, char *p) {
    // Nonescaped
    if (*p != '\\') {
        *res = *p;
        return p + 1;
    }
    p++;

    char escaped[256];
    escaped['a'] = '\a';
    escaped['b'] = '\b';
    escaped['f'] = '\f';
    escaped['n'] = '\n';
    escaped['r'] = '\r';
    escaped['t'] = '\t';
    escaped['v'] = '\v';
    escaped['e'] = 033;
    escaped['E'] = 033;

    if (*p == '0') {
        *res = '\0';
        return p + 1;
    }
    // Simple (e.g. `\n` or `\a`)
    int esc = escaped[(uint8_t) *p];

    if (esc) {
        *res = esc;
        return p + 1;
    }
    *res = *p;
    return p + 1;
}

static char* add_number_literal_type(Token *t, char *p, long val, int base) {
    bool l = false;
    bool u = false;

    if (startswith(p, "LLU") || startswith(p, "LLu") ||
        startswith(p, "llU") || startswith(p, "llu") ||
        startswith(p, "ULL") || startswith(p, "Ull") ||
        startswith(p, "uLL") || startswith(p, "ull")) {
        p += 3;
        l = u = true;
    } else if (!strncasecmp(p, "lu", 2) || !strncasecmp(p, "ul", 2)) {
        p += 2;
        l = u = true;
    } else if (startswith(p, "LL") || startswith(p, "ll")) {
        p += 2;
        l = true;
    } else if (*p == 'L' || *p == 'l') {
        p += 1;
        l = true;
    } else if (*p == 'U' || *p == 'u') {
        p += 1;
        u = true;
    }

    Type *ty;
    if (base == 10) {
        if (l && u)
            ty = ulong_ty();
        else if (l)
            ty = long_ty();
        else if (u)
            ty = (val >> 32) ? ulong_ty() : uint_ty();
        else
            ty = (val >> 31) ? long_ty() : int_ty();
    } else {
        if (l && u)
            ty = ulong_ty();
        else if (l)
            ty = (val >> 63) ? ulong_ty() : long_ty();
        else if (u)
            ty = (val >> 32) ? ulong_ty() : uint_ty();
        else if (val >> 63)
            ty = ulong_ty();
        else if (val >> 32)
            ty = long_ty();
        else if (val >> 31)
            ty = uint_ty();
        else
            ty = int_ty();
    }
    t->type = ty;
    return p;
}

static char *hexdecimal(char *p) {
    Token *t = add_token(tokens, TK_NUM, p);
    t->val = 0;
    p += 2;

    if (!isxdigit(*p))
        fprintf(stderr, "bad hexdecimal number");
    for (;;) {
        if ('0' <= *p && *p <= '9') {
            t->val = (t->val * 16) + (*p++ - '0');
        } else if ('a' <= *p && *p <= 'f') {
            t->val = (t->val * 16) + (*p++ - 'a' + 10);
        } else if ('A' <= *p && *p <= 'F') {
            t->val = (t->val * 16) + (*p++ - 'A' + 10);
        } else {
            t->end = p;
            p = add_number_literal_type(t, p, t->val, 16);
            return p;
        }
    }
}

static char *octal(char *p) {
    Token *t = add_token(tokens, TK_NUM, p++);
    t->val = 0;
    while ('0' <= *p && *p <= '7')
        t->val = t->val * 8 + *p++ - '0';
    t->end = p;
    p = add_number_literal_type(t, p, t->val, 8);
    return p;
}

static void scan() {
    char *p = buf;
    while (*p) {
        if (*p == '\n') {
            Token *t = add_token(tokens, *p, p);
            p++;
            t->end = p;
            t->len = 1;
            continue;
        }
        // 空白文字列をスキップ
        if (isspace(*p)) {
            add_space(tokens);
            p++;
            continue;
        }

        // constをスキップ
        if (strncmp(p, "const", 5) == 0 && !is_alnum(p[5])) {
            p += 5;
            continue;
        }

        if (strncmp(p, "_Noreturn", 9) == 0 && !is_alnum(p[9])) {
            p += 9;
            continue;
        }

        if (strncmp(p, "__inline", 8) == 0 && !is_alnum(p[8])) {
            p += 8;
            continue;
        }

        if (strncmp(p, "//", 2) == 0) {
            while(*p != '\n') {
                p++;
            }
            continue;
        }

        if (strncmp(p, "/*", 2) == 0) {
            while(strncmp(p, "*/", 2) != 0) {
                p++;
            }
            p += 2;
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            Token *t = add_token(tokens, TK_RETURN, p);
            p += 6;
            t->end = p;
            t->len = 6;
            continue;
        }

        if (tokenize_comparable(tokens, TK_EQ, p, "==")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_NE, p, "!=")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_LE, p, "<=")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_GE, p, ">=")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_MUL_EQ, p, "*=")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_DIV_EQ, p, "/=")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_MOD_EQ, p, "%=")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_ADD_EQ, p, "+=")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_SUB_EQ, p, "-=")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_SHL_EQ, p, "<<=")) { p += 3; continue; }
        if (tokenize_comparable(tokens, TK_SHR_EQ, p, ">>=")) { p += 3; continue; }
        if (tokenize_comparable(tokens, TK_BITAND_EQ, p, "&=")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_XOR_EQ, p, "^=")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_BITOR_EQ, p, "|=")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_LOGOR, p, "||")) { p += 2; continue; }
        if (tokenize_comparable(tokens, TK_LOGAND, p, "&&")) { p += 2; continue; }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            Token *t = add_token(tokens, TK_IF, p);
            p += 2;
            t->end = p;
            t->len = 2;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            Token *t = add_token(tokens, TK_ELSE, p);
            p += 4;
            t->end = p;
            t->len = 2;
            continue;
        }

        if (strncmp(p, "struct", 6) == 0 && !is_alnum(p[6])) {
            Token *t = add_token(tokens, TK_STRUCT, p);
            p += 6;
            t->end = p;
            t->len = 6;
            continue;
        }

        if (strncmp(p, "do", 2) == 0 && !is_alnum(p[2])) {
            Token *t = add_token(tokens, TK_DO, p);
            p += 2;
            t->end = p;
            t->len = 2;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            Token *t = add_token(tokens, TK_WHILE, p);
            p += 5;
            t->end = p;
            t->len = 5;
            continue;
        }

        if (strncmp(p, "case", 4) == 0 && !is_alnum(p[4])) {
            Token *t = add_token(tokens, TK_CASE, p);
            p += 4;
            t->end = p;
            t->len = 4;
            continue;
        }

        if (strncmp(p, "switch", 6) == 0 && !is_alnum(p[6])) {
            Token *t = add_token(tokens, TK_SWITCH, p);
            p += 6;
            t->end = p;
            t->len = 6;
            continue;
        }

        if (strncmp(p, "break", 5) == 0 && !is_alnum(p[5])) {
            Token *t = add_token(tokens, TK_BREAK, p);
            p += 5;
            t->len = 5;
            t->end = p;
            continue;
        }

        if (strncmp(p, "continue", 8) == 0 && !is_alnum(p[8])) {
            Token *t = add_token(tokens, TK_CONTINUE, p);
            p += 8;
            t->len = 8;
            t->end = p;
            continue;
        }

        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            Token *t = add_token(tokens, TK_FOR, p);
            p += 3;
            t->len = 3;
            t->end = p;
            continue;
        }

        if (strncmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
            Token *t = add_token(tokens, TK_INT, p);
            p += 3;
            t->end = p;
            t->len = 3;
            continue;
        }

        if (strncmp(p, "long", 4) == 0 && !is_alnum(p[4])) {
            Token *t = add_token(tokens, TK_LONG, p);
            p += 4;
            t->end = p;
            t->len = 4;
            continue;
        }

        if (strncmp(p, "short", 5) == 0 && !is_alnum(p[5])) {
            Token *t = add_token(tokens, TK_SHORT, p);
            p += 5;
            t->end = p;
            t->len = 5;
            continue;
        }

        if (strncmp(p, "char", 4) == 0 && !is_alnum(p[4])) {
            Token *t = add_token(tokens, TK_CHAR, p);
            p += 4;
            t->end = p;
            t->len = 4;
            continue;
        }

        if (strncmp(p, "void", 4) == 0 && !is_alnum(p[4])) {
            Token *t = add_token(tokens, TK_VOID, p);
            p += 4;
            t->len = 4;
            t->end = p;
            continue;
        }

        if (strncmp(p, "signed", 6) == 0 && !is_alnum(p[6])) {
            Token *t = add_token(tokens, TK_SIGNED, p);
            p += 6;
            t->len = 6;
            t->end = p;
            continue;
        }

        if (strncmp(p, "unsigned", 8) == 0 && !is_alnum(p[8])) {
            Token *t = add_token(tokens, TK_UNSIGNED, p);
            p += 8;
            t->len = 8;
            t->end = p;
            continue;
        }

        if (strncmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
            Token *t = add_token(tokens, TK_SIZEOF, p);
            p += 6;
            t->end = p;
            t->len = 6;
            continue;
        }

        if (strncmp(p, "alignof", 7) == 0 && !is_alnum(p[7])) {
            Token *t = add_token(tokens, TK_ALIGNOF, p);
            p += 7;
            t->end = p;
            t->len = 7;
            continue;
        }

        if (strncmp(p, "extern", 6) == 0 && !is_alnum(p[6])) {
            Token *t = add_token(tokens, TK_EXTERN, p);
            p += 6;
            t->end = p;
            t->len = 6;
            continue;
        }

        if (strncmp(p, "->", 2) == 0) {
            Token *t = add_token(tokens, TK_ARROW, p);
            p += 2;
            t->end = p;
            t->len = 2;
            continue;
        }

        if (strncmp(p, "typedef", 7) == 0 && !is_alnum(p[7])) {
            Token *t = add_token(tokens, TK_TYPEDEF, p);
            p += 7;
            t->end = p;
            t->len = 7;
            continue;
        }

        if (strncmp(p, "static", 6) == 0 && !is_alnum(p[6])) {
            Token *t = add_token(tokens, TK_STATIC, p);
            p += 6;
            t->end = p;
            t->len = 6;
            continue;
        }

        if (strncmp(p, "_Bool", 5) == 0 && !is_alnum(p[5])) {
            Token *t = add_token(tokens, TK_BOOL, p);
            p += 5;
            t->end = p;
            t->len = 5;
            continue;
        }

        if (strncmp(p, "enum", 4) == 0 && !is_alnum(p[4])) {
            Token *t = add_token(tokens, TK_ENUM, p);
            fprintf(stderr, "tokenize ENUM\n");
            p += 4;
            t->end = p;
            t->len = 4;
            continue;
        }

        if (strncmp(p, "<<", 2) == 0) {
            Token *t = add_token(tokens, TK_LSHIFT, p);
            p += 2;
            t->end = p;
            t->len = 2;
            continue;
        }

        if (strncmp(p, ">>", 2) == 0) {
            Token *t = add_token(tokens, TK_RSHIFT, p);
            p += 2;
            t->end = p;
            t->len = 2;
            continue;
        }

        if (strncmp(p, "++", 2) == 0) {
            Token *t = add_token(tokens, TK_INC, p);
            p += 2;
            t->end = p;
            t->len = 2;
            continue;
        }

        if (strncmp(p, "--", 2) == 0) {
            Token *t = add_token(tokens, TK_DEC, p);
            p += 2;
            t->end = p;
            t->len = 2;
            continue;
        }

        if (strncmp(p, "...", 3) == 0) {
            Token *t = add_token(tokens, TK_DOTS, p);
            p += 3;
            t->end = p;
            t->len = 3;
            continue;
        }
        if (strchr("+-*/%()=;{},<>&[].!?:|&^~#", *p)) {
            Token *t = add_token(tokens, *p, p);
            p++;
            t->end = p;
            t->len = 1;
            continue;
        }

        if (!strncasecmp(p, "0x", 2)) {
            p = hexdecimal(p);
            continue;
        }

        if (*p == '0') {
            p = octal(p);
            continue;
        }

        if (isdigit(*p)) {
            Token * t = add_token(tokens, TK_NUM, p);
            long val = strtol(p, &p, 10);
            t->val = val;
            t->end = p;
            p = add_number_literal_type(t, p, t->val, 10);
            continue;
        }

        // 文字型
        if (*p == '\'') {
            Token *t = add_token(tokens, TK_NUM, p++);
            p = c_char(&t->val, p);
            t->type = char_ty();
            p++;
            t->end = p;
            continue;
        }


        if (*p == '"') {
            p++;
            char* init_p = p;
            int len = 0;
            Token *t = add_token(tokens, TK_STR, p);
            // mallocするのにlenをとっておく
            while(!(*p == '"' && *(p-1) != '\\')) {
                len++;
                p++;
            }
            char *str = (char*) malloc(len + 1);
            p = init_p;
            int i = 0;
            len = 0;
            // ここで、\, nみたいに分かれているのを統一する => \n
            while(*p != '"') {
                long c;
                p = c_char(&c, p);
                str[i++] = c;
                len++;
            }
            t->end = p;
            str[len] = '\0';
            t->len = len + 1;
            p++;
            t->str = str;
            continue;
        }

        if (is_alnum(*p)) {
            char* init_p = p;
            int len = 0;
            // パースできる文字出るじゃない文字が出てくるまで
            Token *t = add_token(tokens, TK_IDENT, p);
            while(is_alnum(*p)){
                len++;
                p++;
            }
            char *name = (char *) malloc(len + 1);
            strncpy(name, init_p, len);
            name[len] = '\0';
            t->len = len;
            t->name = name;
            t->end = p;
            continue;
        }

        fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
    }
}

static void strip_newlines() {
    Vector *v = new_vector();
    for (int i = 0; i < tokens->len; i++) {
        Token *t = tokens->data[i];
        if (t->ty != '\n')
            vec_push(v, t);
    }
    tokens = v;
}

static void remove_backslash_newline() {
    char *p = buf;
    for (char *q = p; *q;) {
        if (q[0] == '\\' && q[1] == '\n')
            q += 2;
        else
            *p++ = *q++;
    }
    *p = '\0';
}

static void append(Token *x, Token *y) {
    StringBuilder *sb = new_sb();
    fprintf(stderr, "x->len: %d, y->len: %d\n", x->len, y->len);
    sb_append_n(sb, x->str, x->len - 1);
    sb_append_n(sb, y->str, y->len - 1);
    x->str = sb_get(sb);
    fprintf(stderr, "append: %s\n", x->str);
    x->len = sb->len;
}

static void join_string_literals() {
    Vector *v = new_vector();
    Token *last = NULL;
    for (int i = 0; i < tokens->len; i++) {
        Token *t = tokens->data[i];
        if (last && last->ty == TK_STR && t->ty == TK_STR) {
            fprintf(stderr, "last: %s t: %s\n", last->str, t->str);
            append(last, t);
            continue;
        }
        last = t;
        vec_push(v, t);
    }
    tokens = v;
}

// pが指している文字列をトークンに分割してtokensに保存する
Vector *tokenize(char *path, bool add_eof) {
    Vector *tokens_ = tokens;
    char *filename_ = filename;
    char *buf_ = buf;

    tokens = new_vector();
    filename = path;
    buf = read_file(path);
    remove_backslash_newline();
    scan();
    join_string_literals();

    if (add_eof)
        add_token(tokens, TK_EOF, buf);
    tokens = preprocess(tokens);
    //fprintf(stderr, "preprocessed...\n");
    //show_tokens(tokens);
    strip_newlines();
    Vector *ret = tokens;
    buf = buf_;
    tokens = tokens_;
    filename = filename_;
    return ret;
}

static void print_line(char *start, char *path, char *pos) {
    int line = 0;
    int col = 0;

    for (char *p = start; p; p++) {
        if (*p == '\n') {
            start = p + 1;
            line++;
            col = 0;
            continue;
        }

        if (p != pos) {
            col++;
            continue;
        }

        fprintf(stderr, "error at %s:%d:%d\n\n", path, line + 1,col + 1);

        int linelen = strchr(p, '\n') - start;
        fprintf(stderr, "%.*s\n", linelen, start);

        for (int i = 0; i < col; i++)
            fprintf(stderr, " ");
        fprintf(stderr, "^\n\n");
        return;
    }
}

noreturn void bad_token(Token *t, char *msg) {
    print_line(t->buf, t->filename, t->start);
    error(msg);
}

