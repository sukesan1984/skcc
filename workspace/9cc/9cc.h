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
#include <errno.h>
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
    char *str;   // tyがTK_STRの場合はそのリテラル
    char len;
    char *name;  // tyがTK_IDENTの場合、その名前
    char *input; // トークン文字列(エラーメッセージ用)

    // For preprocessor
    bool stringize;

    // For error reporting
    char *buf;
    char *filename;
    char *start;
    char *end;
} Token;

typedef struct Type {
    enum {INT, PTR, ARRAY, CHAR, VOID, STRUCT, FUNC} ty;
    int size;
    int align;

    // Pointer
    struct Type *ptr_to;

    // Array
    struct Type *array_of;
    size_t array_size;

    // Struct
    Vector *members;

    int offset;
    // Function
    struct Type *returning;
} Type;

Type *ary_of(Type *base, size_t size);
Type *ptr_to(Type *base);

typedef struct {
    Type *ty;
    int offset;
    bool is_local;
    char* data;
    int len;
    char *name;
    bool is_extern;
} Var;

typedef struct Node {
    int op; // 演算子かTK_NUM
    Type *ty;         // C type
    struct Node *lhs; // 左辺(tyがTK_FORのときforの左)
    struct Node *lhs2; //tyがTK_FORのときのforの真ん中
    struct Node *lhs3; //tyがTK_FORのときのforの右
    struct Node *rhs; // 右辺
    struct Node *cond; // TK_IFのcondition
    struct Node *if_body; // TK_IFのbody
    struct Node *else_body; // TK_ELSEのbody
    struct Node *body; // function definition
    struct Node *init;
    Vector *args;      // 関数の場合の引数が格納される
    Vector *stmts;     // stmtの集合が入る
    int val; // ty がTK_NUMの場合のみ使う
    char* name;

    int break_label;
    int continue_label;

    // For switch and case
    Vector *cases;
    int case_label;

    // For break and continue
    struct Node *target;

    // Global variable
    char *data;
    int len;

    int stacksize;

    // Offset from BP or begining of a struct
    int offset;

    bool is_extern;
} Node;

// トークンの型を表す値
enum {
    TK_NUM = 256,
    TK_STR, // String literal
    TK_IDENT,
    TK_IF, // ifのトークン
    TK_ELSE, // else
    TK_RETURN,
    TK_WHILE, // whileのトークン
    TK_SWITCH, // switch
    TK_CASE,
    TK_BREAK, // break
    TK_CONTINUE, // continue
    TK_FOR, // forのトークン
    TK_EQ, // ==
    TK_NE, // !=
    TK_LE, // <=
    TK_GE, // >=
    TK_EOF,
    TK_INT, // int
    TK_CHAR, // char
    TK_VOID, // void
    TK_LOGOR, // ||
    TK_LOGAND, // &&
    TK_SIZEOF, //sizeof
    TK_STRUCT, // struct
    TK_ARROW, // ->
    TK_TYPEDEF, // typedef
    TK_EXTERN, // extern
    TK_LSHIFT, // <<
    TK_RSHIFT, // >>
    TK_INC, // ++
    TK_DEC, // --
    TK_MUL_EQ, // *=
    TK_DIV_EQ, // /=
    TK_MOD_EQ, // %=
    TK_ADD_EQ, // +=
    TK_SUB_EQ, // -=
    TK_SHL_EQ, // <<=
    TK_SHR_EQ, // >>=
    TK_BITAND_EQ, //&=
    TK_XOR_EQ, // ^=
    TK_BITOR_EQ, // |=
    TK_PARAM,
};

enum {
    ND_NUM = 256,
    ND_IDENT, //257
    ND_LVAR, //258
    ND_GVAR, //259
    ND_RETURN, //260
    ND_CALL, //261
    ND_FUNC, //262
    ND_COMP_STMT, //263
    ND_EXPR_STMT, //264
    ND_IF, //265
    ND_FOR, //266
    ND_WHILE, //267
    ND_EQ, // 268
    ND_NE, // 269
    ND_LE, // 270
    ND_DEREF, //271
    ND_VARDEF, //272
    ND_ADDR, // 273
    ND_LOGOR, // || 274
    ND_LOGAND, // && 275
    ND_SIZEOF, // 276
    ND_STRUCT, // struct 277
    ND_DOT, // . Struct member access 278
    ND_NULL, // null node
    ND_STR,
    ND_STMT_EXPR,
    ND_LSHIFT, // <<
    ND_RSHIFT, // >>
    ND_NEG, // - for unary
    ND_PREINC, // ++i
    ND_PREDEC, // --i
    ND_POSTINC, // i++
    ND_POSTDEC, // i++
    ND_BREAK, //
    ND_MUL_EQ, // *=
    ND_DIV_EQ, // /=
    ND_MOD_EQ, // %=
    ND_ADD_EQ, // +=
    ND_SUB_EQ, // -=
    ND_SHL_EQ, // <<=
    ND_SHR_EQ, // >>=
    ND_BITAND_EQ, //&=
    ND_XOR_EQ, // ^=
    ND_BITOR_EQ, // |=
    ND_DECL,     // declaration
    ND_SWITCH,   // swtich
    ND_CASE,     // case
    ND_CONTINUE, // continue
};

// parse.c
noreturn void error(char *fmt, ...);
Vector *parse(Vector *tokens);

extern int nlabel;

// tokenize.c
Vector *tokenize(char *path, bool add_eof);
void bad_token(Token *t, char *msg);
char *tokstr(Token *t);
int line(Token *t);

// sema.c
void sema(Vector *nodes);

// codegen.c
void gen_main(Vector* v);

// Vectorを操作する関数群
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void *vec_pop(Vector *vec);

// Mapを操作する関数群
Map *new_map();
void map_put(Map *map, char *key, void *val);
void map_puti(Map *map, char *key, int val);
void *map_get(Map *map, char *key);
int map_geti(Map *map, char *key, int undef);
bool map_exists(Map *map, char *key);

typedef struct {
    char *data;
    int capacity;
    int len;
} StringBuilder;

StringBuilder *new_sb(void);
void sb_add(StringBuilder *sb, char c);
void sb_append(StringBuilder *sb, char *s);
void sb_append_n(StringBuilder *sb, char *s, int len);
char *sb_get(StringBuilder *sb);

char *format(char *fmt, ...);
char *read_file(char *path);

// util
int roundup(int x, int align);
Type *struct_of(Vector *members);

// Global変数
extern Node *code[100];
// トークナイズした結果のトークン列はvecに格納する

//変数名とRBPからのオフセットを管理する
extern int variables;
extern Map* variable_map;
extern Vector *globals;

// preprocess.c
Vector *preprocess(Vector* tokens);
