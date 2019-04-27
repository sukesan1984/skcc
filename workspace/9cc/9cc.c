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

// 可変長Vector
// 任意蝶の入力をサポートする
typedef struct {
    void **data;  // 実際のデータ
    int capacity; // バッファの大きさ
    int len;      // ベクタに追加済みの要素の個数。len == capacityのときにバッファがいっぱい、新たに要素を足す場合は、新たにバッファを確保して既存の要素をコピーし、dataポインタをすげ替える
} Vector;

Vector *new_vector() {
    Vector *vec = malloc(sizeof(Vector));
    vec-> data = malloc(sizeof(void *) * 16);
    vec->capacity = 16;
    vec->len = 0;
    return vec;
}

void vec_push(Vector *vec, void *elem) {
    if (vec->capacity == vec->len) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len++] = elem;
}

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

Node *expr();
Node *mul();
Node *term();

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

int consume(int ty) {
    if (tokens[pos].ty != ty)
        return 0;
    pos++;
    return 1;
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

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
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
    Node *lhs = term();
    if(tokens[pos].ty == TK_EOF)
        return lhs;

    if(consume('*'))
        return new_node('*', lhs, mul());

    if(consume('/'))
        return new_node('/', lhs, mul());

    if (lhs->op == ND_NUM)
        return lhs;
    error("想定しないトークンです(mul): %s", tokens[pos].input);
    return lhs;
}

Node *expr() {
    Node *lhs = mul();
    if (tokens[pos].ty == TK_EOF)
        return lhs;

    if(consume('+'))
        return new_node('+', lhs, expr());

    if(consume('-'))
        return new_node('-', lhs, expr());

    if (lhs->op == ND_NUM)
        return lhs;

    error("想定しないトークンです(expr): %s", tokens[pos].input);
    return lhs;
}

Node *term() {
    if (tokens[pos].ty == TK_NUM)
        return new_node_num(tokens[pos++].val);

    if(consume('(')) {
        Node *node = expr();
        if (tokens[pos].ty != ')') {
            error("閉じ括弧で閉じる必要があります: %s", tokens[pos].input);
        }
        pos++;
        return node;
    }
    error("数字でも開き括弧でもないトークンです: %s", tokens[pos].input);
    exit(1);
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int expect(int line, int expected, int actual) {
    if (expected == actual)
        return 0;
    fprintf(stderr, "%d: %d expected, but got %d\n",
            line, expected, actual);
    exit(1);
}

void runtest() {
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);

    for (int i = 0; i < 100; i++)
        vec_push(vec, (void *)(uintptr_t) i);

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (intptr_t)vec->data[0]);
    expect(__LINE__, 50, (intptr_t)vec->data[50]);
    expect(__LINE__, 99, (intptr_t)vec->data[99]);
    printf("OK\n");
}

int main(int argc, char **argv) {
    if (argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    //fprintf(stderr, "引数: %s\n", argv[1]);
    if (strcmp(argv[1], "-test") == 0) {
        runtest();
        return 0;
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

