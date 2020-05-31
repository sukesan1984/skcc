#define bool _Bool
#define true 1
#define false 0
#define __bool_true_false_are_defined 1
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

typedef struct Type {
    int ty;
    int size;
    int align;
    bool is_unsigned;

    // Pointer
    struct Type *ptr_to;

    // Array
    int array_size;

    // Struct
    Vector *members;

    int offset;
    // Function
    struct Type *returning;
    bool is_incomplete;
    bool is_variadic;
} Type;

// トークンの型
typedef struct {
    int ty;      // トークンの型
    Type *type;  // tyがTK_NUMの場合の型
    long val;     // tyがTK_NUMの場合,その数値
    char *str;   // tyがTK_STRの場合はそのリテラル
    char len;
    char *name;  // tyがTK_IDENTの場合、その名前
    char *input; // トークン文字列(エラーメッセージ用)

    // For preprocessor
    bool stringize;
    bool has_space;

    // For error reporting
    char *buf;
    char *filename;
    char *start;
    char *end;
} Token;

typedef struct Initializer {
    struct Initializer *next;
    int sz;
    long val;

    char *label;
} Initializer;

typedef struct {
    Type *ty;
    int offset;
    bool is_local;
    char* data;
    int len;
    char *name;
    bool is_extern;
    bool is_static;
    Initializer *initializer;
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
    long val; // ty がTK_NUMの場合のみ使う
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
    Initializer *initializer;

    int stacksize;

    // Offset from BP or begining of a struct
    int offset;

    bool is_extern;
    bool is_static;
    struct Node *next;
} Node;

