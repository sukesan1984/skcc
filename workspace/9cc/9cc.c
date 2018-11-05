#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum {
    TK_NUM = 256,
    TK_EOF,
};

// トークンの型
typedef struct {
    int ty;      // トークンの型
    int val;     // tyがTK_NUMの場合,その数値
    char *input; // トークン文字列(エラーメッセージ用)
} Token;

// トークナイズした結果のトークン列はこの配列に保存する
// 100個以上のトークンはこないものとする
Token tokens[100];
int pos = 0;

enum {
    ND_NUM = 256, // 整数のノードの型
};

typedef struct Node {
    int op; // 演算子かND_NUM
    struct Node *lhs; // 左辺
    struct Node *rhs; // 右辺
    int val; // ty がND_NUMの場合のみ使う
} Node;

Node *new_node(int op, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->op = op;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->op = ND_NUM;
    node->val = val;
    return node;
}

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
    int i = 0;
    while (*p) {
        // 空白文字列をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/') {
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p)) {
            tokens[i].ty = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            i++;
            continue;
        }

        fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
    }

    tokens[i].ty = TK_EOF;
    tokens[i].input = p;
}

// エラーを報告するための関数
void error(char *str, char *i){
    fprintf(stderr, "%s%s\n", str, i);
    exit(1);
}

void error_token(int i){
    error("予期せぬトークンです:", tokens[i].input);
}

Node *mul() {
    Node *lhs = new_node_num(tokens[pos++].val);
    if(tokens[pos].ty == TK_EOF)
        return lhs;

    if(tokens[pos].ty == '*') {
        pos++;
        return new_node('*', lhs, mul());
    }
    if(tokens[pos].ty == '/') {
        pos++;
        return new_node('/', lhs, mul());
    }

    if (lhs->op == ND_NUM)
        return lhs;
    error("想定しないトークンです(mul): %s", tokens[pos].input);
    return lhs;
}

Node *expr() {
    Node *lhs = mul();
    if (tokens[pos].ty == TK_EOF)
        return lhs;

    if(tokens[pos].ty == '+') {
        pos++;
        return new_node('+', lhs, expr());
    }

    if(tokens[pos].ty == '-') {
        pos++;
        return new_node('-', lhs, expr());
    }
    error("想定しないトークンです(expr): %s", tokens[pos].input);
    return lhs;
}

void gen(Node *node) {
    if (node->op == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch(node->op) {
        case '+':
            printf("  add rax, rdi\n");
            break;
        case '-':
            printf("  sub rax, rdi\n");
            break;
        case '*':
            printf("  mul rdi\n");
            break;
        case '/':
            printf("  mov rdx, 0\n");
            printf("  div rdi\n");
    }
    printf("  push rax\n");
}


int main(int argc, char **argv) {
    if (argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    // トークナイズしてパースする
    tokenize(argv[1]);
    Node* node = expr();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコード生成
    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}

