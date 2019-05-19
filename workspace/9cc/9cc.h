#define _GNU_SOURCE
#include <assert.h>
#include <stdnoreturn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdbool.h>
// container.c
typedef struct {
    void **data;  // 実際のデータ
    int capacity; // バッファの大きさ
    int len;      // ベクタに追加済みの要素の個数。len == capacityのときにバッファがいっぱい、新たに要素を足す場合は、新たにバッファを確保して既存の要素をコピーし、dataポインタをすげ替える
} Vector;

typedef struct {
    Vector *keys;
    Vector *vals;
} Map;

// トークンの型
typedef struct {
    int ty;      // トークンの型
    int val;     // tyがTK_NUMの場合,その数値
    char *name;  // tyがTK_IDENTの場合、その名前
    char *input; // トークン文字列(エラーメッセージ用)
} Token;

enum {
    INT,
    PTR,
};

typedef struct Type {
    int ty;
    struct Type *ptr_of;
} Type;

typedef struct {
    Type *ty;
    int offset;
} Var;

typedef struct Node {
    int op; // 演算子かTK_NUM
    Type *ty;         // C type
    struct Node *lhs; // 左辺(tyがTK_FORのときforの左)
    struct Node *lhs2; //tyがTK_FORのときのforの真ん中
    struct Node *lhs3; //tyがTK_FORのときのforの右
    struct Node *rhs; // 右辺
    struct Node *body; // function definition
    struct Node *init;
    Vector *args;      // 関数の場合の引数が格納される
    Vector *stmts;     // stmtの集合が入る
    int val; // ty がTK_NUMの場合のみ使う
    char* name;
    int stacksize;
    int offset;
} Node;

// トークンの型を表す値
enum {
    TK_NUM = 256,
    TK_IDENT,
    TK_IF, // ifのトークン
    TK_RETURN,
    TK_WHILE, // whileのトークン
    TK_FOR, // forのトークン
    TK_EQ, // ==
    TK_NE, // !=
    TK_LE, // <=
    TK_GE, // >=
    TK_EOF,
    TK_INT, // int
    TK_LOGOR, // ||
    TK_LOGAND, // &&
    TK_SIZEOF, //sizeof
};

enum {
    ND_NUM = 256,
    ND_IDENT,
    ND_RETURN,
    ND_CALL,
    ND_FUNC,
    ND_COMP_STMT,
    ND_EXPR_STMT,
    ND_IF,
    ND_FOR,
    ND_WHILE,
    ND_EQ,
    ND_NE,
    ND_LE,
    ND_DEREF,
    ND_VARDEF,
    ND_ADDR,
    ND_LOGOR, // ||
    ND_LOGAND, // &&
    ND_SIZEOF,
};

// parse.c
noreturn void error(char *fmt, ...);
Vector *parse();

// tokenize.c
void tokenize(char *p);

// sema.c
void sema(Vector *nodes);

// codegen.c
void gen_main(Vector* v);

// Vectorを操作する関数群
Vector *new_vector();
void vec_push(Vector *vec, void *elem);

// Mapを操作する関数群
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);
bool map_exists(Map *map, char *key);


void runtest();

// Global変数
extern Node *code[100];
// トークナイズした結果のトークン列はvecに格納する
extern Vector* tokens;

//変数名とRBPからのオフセットを管理する
extern int variables;
extern Map* variable_map;
