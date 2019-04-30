// トークンの型を表す値
enum {
    TK_NUM = 256,
    TK_RETURN,
    TK_IDENT,
    TK_EOF = 1,
    TK_EQ, // ==
    TK_NE, // !=
    TK_LE, // <=
    TK_GE, // >=
    TK_L,  // <
    TK_G,  // >
};

enum {
    ND_NUM = 256,   // 整数のノードの型
    ND_RETURN,      // returnのノード
    ND_IDENT,       // 変数のノード
};

// トークンの型
typedef struct {
    int ty;      // トークンの型
    int val;     // tyがTK_NUMの場合,その数値
    char *name;  // tyがTK_IDENTの場合、その名前
    char *input; // トークン文字列(エラーメッセージ用)
} Token;

typedef struct Node {
    int ty; // 演算子かND_NUM
    struct Node *lhs; // 左辺
    struct Node *rhs; // 右辺
    int val; // ty がND_NUMの場合のみ使う
    char* name;
} Node;

// parse.c
void error(char *str, char *i);
void error_token(int i);
int consume(int ty);
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(char* name);
int is_alnum(char c);
void tokenize(char *p);
void program();
Node *stmt();
Node *assign();
Node *add();
Node *mul();
Node *term();

// codegen.c
void gen_main();
void gen_initial();
void gen_prolog();
void gen_epilog();
void gen_lval(Node *node);
void gen(Node *node);

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

// Vectorを操作する関数群
Vector *new_vector();
void vec_push(Vector *vec, void *elem);

// Mapを操作する関数群
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);

// Vectorにtokenを足す
Token *add_token(Vector *v, int ty, char *input);

int expect(int line, int expected, int actual);
int expect_token(int line, Token* expected, Token* actual);
void runtest();

// Global変数
extern Node *code[100];
// トークナイズした結果のトークン列はvecに格納する
extern Vector* tokens;

//変数名とRBPからのオフセットを管理する
extern int variables;
extern Map* variable_map;
