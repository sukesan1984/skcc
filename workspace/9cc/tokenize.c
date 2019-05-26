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

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
    while (*p) {
        // 空白文字列をスキップ
        if (isspace(*p)) {
            p++;
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
        if (tokenize_comparable(tokens, TK_LOGOR, p, "||")) { p += 2; continue; };
        if (tokenize_comparable(tokens, TK_LOGAND, p, "&&")) { p += 2; continue; };

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            add_token(tokens, TK_IF, p);
            p += 2;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            add_token(tokens, TK_WHILE, p);
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

        if (strncmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
            add_token(tokens, TK_SIZEOF, p);
            p += 6;
            continue;
        }
        if (strchr("+-*/()=;{},<>&[]", *p)) {
            add_token(tokens, *p, p);
            p++;
            continue;
        }


        if (isdigit(*p)) {
            Token * t = add_token(tokens, TK_NUM, p);
            t->val = strtol(p, &p, 10);
            continue;
        }

        if (*p == '"') {
            p++;
            char* init_p = p;
            int len = 0;
            Token *t = add_token(tokens, TK_STR, p);
            while(*p != '"') {
                len++;
                p++;
            }
            p++;
            char *str = (char*) malloc(len + 1);
            strncpy(str, init_p, len);
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
            t->name = name;
            continue;
        }

        fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
    }
    add_token(tokens, TK_EOF, p);
}

