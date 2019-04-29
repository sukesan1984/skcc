#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum {
    TK_NUM = 256,
    TK_IDENT,
    TK_EOF = 1,
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

Token *add_token(Vector *v, int ty, char *input) {
    Token * token = malloc(sizeof(Token));
    token->ty = ty;
    token->input = input;
    vec_push(v, (void *)token);
    return token;
}

// トークナイズした結果のトークン列はvecに格納する
Vector* tokens;
int pos = 0;

enum {
    ND_NUM = 256, // 整数のノードの型
    ND_IDENT,
};

typedef struct Node {
    int ty; // 演算子かND_NUM
    struct Node *lhs; // 左辺
    struct Node *rhs; // 右辺
    int val; // ty がND_NUMの場合のみ使う
    char name;
} Node;

Node *code[100];

Node *stmt();
Node *assign();
Node *add();
Node *mul();
Node *term();

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_ident(char name) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_IDENT;
    node->name = name;
    return node;
}

int consume(int ty) {
    Token *t = tokens->data[pos];
    if (t->ty != ty)
        return 0;
    pos++;
    return 1;
}

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
    while (*p) {
        // 空白文字列をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '=' || *p == ';') {
            add_token(tokens, *p, p);

            p++;
            continue;
        }

        if (isdigit(*p)) {
            Token * t = add_token(tokens, TK_NUM, p);
            t->val = strtol(p, &p, 10);

            continue;
        }

        if ('a' <= *p && *p <= 'z') {
            Token *t = add_token(tokens, TK_IDENT, p);
            p++;
            continue;
        }

        fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
    }
    add_token(tokens, TK_EOF, p);
}

// エラーを報告するための関数
void error(char *str, char *i){
    fprintf(stderr, "%s%s\n", str, i);
    exit(1);
}

void error_token(int i){
    Token *t = tokens->data[i];
    error("予期せぬトークンです:", t->input);
}

void program() {
    int i = 0;
    while(((Token *)tokens->data[pos])->ty != TK_EOF)
        code[i++] = stmt();
    code[i] = NULL;
}

Node *stmt() {
    Node *lhs = assign();
    if (!consume(';')) {
        Token *t = tokens->data[pos];
        error("';'ではないトークンです: %s", t->input);
    }
    return lhs;
}

Node *assign() {
    Node *lhs = add();
    Token *t = tokens->data[pos];
    if (consume('='))
        return new_node('=', lhs, assign());
    return lhs;
}

Node *add() {
    Node *lhs = mul();
    Token *t = tokens->data[pos];

    if(consume('+'))
        return new_node('+', lhs, add());

    if(consume('-'))
        return new_node('-', lhs, add());

    if (lhs->ty == ND_NUM)
        return lhs;
    return lhs;
}

Node *mul() {
    Node *lhs = term();
    Token *t = tokens->data[pos];
    if(t->ty == TK_EOF)
        return lhs;

    if(consume('*'))
        return new_node('*', lhs, mul());

    if(consume('/'))
        return new_node('/', lhs, mul());

    if (lhs->ty == ND_NUM)
        return lhs;
    return lhs;
}

Node *term() {
    Token *t = tokens->data[pos];
    if (t->ty == TK_NUM){
        return new_node_num(((Token *)tokens->data[pos++])->val);
    }

    if (t->ty == TK_IDENT) {
        return new_node_ident(*((Token *)tokens->data[pos++])->input);
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

// 左辺値を計算する
void gen_lval(Node *node) {
    if (node->ty != ND_IDENT)
        error("代入の左辺値が変数ではありません", 0);

    //Nodeが変数の場合
    int offset = ('z' - node->name + 1) * 8;
    printf("  mov rax, rbp\n");         // ベースポインタをraxにコピー
    printf("  sub rax, %d\n", offset);  // raxをoffset文だけ押し下げ（nameの変数のアドレスをraxに保存)
    printf("  push rax\n");             // raxをスタックにプッシュ
}

void gen(Node *node) {
    if (node->ty == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    if (node->ty == ND_IDENT) {
        gen_lval(node);
        printf("  pop rax\n");          // スタックからpopしてraxに格納
        printf("  mov rax, [rax]\n");   // raxをアドレスとして値をロードしてraxに格納
        printf("  push rax\n");         // スタックにraxをpush
        return;
    }

    // 変数に格納
    if (node->ty == '=') {
        // 左辺が変数であるはず
        gen_lval(node->lhs);            // ここでスタックのトップに変数のアドレスが入っている
        gen(node->rhs);                 // 右辺値が評価されてスタックのトップに入っている

        printf("  pop rdi\n");          // 評価された右辺値がrdiにロード
        printf("  pop rax\n");          // 変数のアドレスがraxに格納
        printf("  mov [rax], rdi\n");   // raxのレジスタのアドレスにrdiの値をストアする
        printf("  push rdi\n");         // rdiの値をスタックにプッシュする
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch(node->ty) {
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

int expect_token(int line, Token* expected, Token* actual) {
    if (expected->ty == actual->ty
            && expected->val == actual->val) {
        return 0;
    }
    fprintf(stderr, "%d: (ty) %d expected, but got %d\n",
            line, expected->ty, actual->ty);
    fprintf(stderr, "%d: (val) %d expected, but got %d\n",
            line, expected->val, actual->val);
    exit(1);
}

void runtest() {
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);
    for (int i = 0; i < 100; i++)
        vec_push(vec, (void *)(uintptr_t) i);

    Token token;
    token.ty = TK_NUM;
    token.val = 5;

    Vector *vec2 = new_vector();
    vec_push(vec2, (void *) &token);

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (intptr_t)vec->data[0]);
    expect(__LINE__, 50, (intptr_t)vec->data[50]);
    expect(__LINE__, 99, (intptr_t)vec->data[99]);

    expect_token(__LINE__, &token, (Token *)vec2->data[0]);

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
    tokens = new_vector();
    tokenize(argv[1]);
    program();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個分の領域を確保する
    printf("  push rbp\n");                 // ベースポインタをスタックにプッシュする
    printf("  mov rbp, rsp\n");             // rspをrbpにコピーする
    printf("  sub rsp, %d\n", 26 * 8);      // rspを26文字の変数分動かす

    for (int i = 0; code[i]; i++) {
        // 抽象構文木を下りながらコード生成
        gen(code[i]);
        // 式の評価結果としてスタックに一つの値が残ってる
        // はずなので、スタックが溢れないようにポップしておく
        printf("  pop rax\n");
    }

    // エピローグ
    printf("  mov rsp, rbp\n");     // ベースポインタをrspにコピーして
    printf("  pop rbp\n");          // スタックの値をrbpに持ってくる
    printf("  ret\n");
    return 0;
}

