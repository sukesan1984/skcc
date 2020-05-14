#include "9cc.h"

void error_token(int i){
    Token *t = tokens->data[i];
    error("予期せぬトークンです:", t->input);
}

Token *add_token(Vector *v, int ty, char *input) {
    Token * token = malloc(sizeof(Token));
    token->ty = ty;
    token->input = input;
    vec_push(v, (void *)token);
    return token;
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
        add_token(tokens, ty, p);
        return 1;
    }
    return 0;
}

static char *c_char(int *res, char *p) {
    // Nonescaped
    if (*p != '\\') {
        *res = *p;
        return p + 1;
    }
    p++;

    static char escaped[256] = {
          ['a'] = '\a', ['b'] = '\b',   ['f'] = '\f',
          ['n'] = '\n', ['r'] = '\r',   ['t'] = '\t',
          ['v'] = '\v', ['e'] = '\033', ['E'] = '\033',
    };

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

static char *hexdecimal(char *p) {
    Token *t = add_token(tokens, TK_NUM, p);
    t->val = 0;
    p += 2;

    if (!isxdigit(*p))
        fprintf(stderr, "bad hexdecimal number");
    for (;;) {
        if ('0' <= *p && *p <= '9')
            t->val = (t->val * 16) + (*p++ - '0');
        else if ('a' <= *p && *p <= 'f')
            t->val = (t->val * 16) + (*p++ - 'a' + 10);
        else if ('A' <= *p && *p <= 'F')
            t->val = (t->val * 16) + (*p++ - 'A' + 10);
        else
            return p;
    }
}

static char *octal(char *p) {
    Token *t = add_token(tokens, TK_NUM, p++);
    t->val = 0;
    while ('0' <= *p && *p <= '7')
        t->val = t->val * 8 + *p++ - '0';
    return p;
}

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
    while (*p) {
        // 空白文字列をスキップ
        if (isspace(*p)) {
            p++;
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
            add_token(tokens, TK_RETURN, p);
            p += 6;
            continue;
        }

        if (tokenize_comparable(tokens, TK_EQ, p, "==")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_NE, p, "!=")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_LE, p, "<=")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_GE, p, ">=")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_MUL_EQ, p, "*=")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_DIV_EQ, p, "/=")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_MOD_EQ, p, "%=")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_ADD_EQ, p, "+=")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_SUB_EQ, p, "-=")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_SHL_EQ, p, "<<=")) { p += 3; continue; };
        if (tokenize_comparable(tokens, TK_SHR_EQ, p, ">>=")) { p += 3; continue; };
        if (tokenize_comparable(tokens, TK_BITAND_EQ, p, "&=")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_XOR_EQ, p, "^=")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_BITOR_EQ, p, "|=")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_LOGOR, p, "||")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_LOGAND, p, "&&")) { p += 2; continue; };

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            add_token(tokens, TK_IF, p);
            p += 2;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            add_token(tokens, TK_ELSE, p);
            p += 4;
            continue;
        }

        if (strncmp(p, "struct", 6) == 0 && !is_alnum(p[6])) {
            add_token(tokens, TK_STRUCT, p);
            p += 6;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            add_token(tokens, TK_WHILE, p);
            p += 5;
            continue;
        }

        if (strncmp(p, "break", 5) == 0 && !is_alnum(p[5])) {
            add_token(tokens, TK_BREAK, p);
            p += 5;
            continue;
        }

        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            add_token(tokens, TK_FOR, p);
            p += 3;
            continue;
        }

        if (strncmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
            add_token(tokens, TK_INT, p);
            p += 3;
            continue;
        }

        if (strncmp(p, "char", 4) == 0 && !is_alnum(p[4])) {
            add_token(tokens, TK_CHAR, p);
            p += 4;
            continue;
        }

        if (strncmp(p, "void", 4) == 0 && !is_alnum(p[4])) {
            add_token(tokens, TK_VOID, p);
            p += 4;
            continue;
        }

        if (strncmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
            add_token(tokens, TK_SIZEOF, p);
            p += 6;
            continue;
        }

        if (strncmp(p, "extern", 6) == 0 && !is_alnum(p[6])) {
            add_token(tokens, TK_EXTERN, p);
            p += 6;
            continue;
        }

        if (strncmp(p, "->", 2) == 0) {
            add_token(tokens, TK_ARROW, p);
            p += 2;
            continue;
        }

        if (strncmp(p, "typedef", 7) == 0) {
            add_token(tokens, TK_TYPEDEF, p);
            p += 7;
            continue;
        }

        if (strncmp(p, "<<", 2) == 0) {
            add_token(tokens, TK_LSHIFT, p);
            p += 2;
            continue;
        }

        if (strncmp(p, ">>", 2) == 0) {
            add_token(tokens, TK_RSHIFT, p);
            p += 2;
            continue;
        }

        if (strncmp(p, "++", 2) == 0) {
            add_token(tokens, TK_INC, p);
            p += 2;
            continue;
        }

        if (strncmp(p, "--", 2) == 0) {
            add_token(tokens, TK_DEC, p);
            p += 2;
            continue;
        }

        if (strchr("+-*/%()=;{},<>&[].!?:|&^", *p)) {
            add_token(tokens, *p, p);
            p++;
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
            t->val = strtol(p, &p, 10);
            continue;
        }

        // 文字型
        if (*p == '\'') {
            Token *t = add_token(tokens, TK_NUM, p++);
            p = c_char(&t->val, p);
            p++;
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
                int c;
                p = c_char(&c, p);
                str[i++] = c;
                len++;
            }
            str[len] = '\0';
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
            t->name = name;
            continue;
        }

        fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
    }
    add_token(tokens, TK_EOF, p);
}

