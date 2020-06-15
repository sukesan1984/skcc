#define _GNU_SOURCE
#include "struct.h"
//#include <assert.h>
void assert(int expression);
//#include <stdnoreturn.h>
#define noreturn _Noreturn
//#include <stddef.h>
#define size_t int
#define NULL 0
//#include <stdarg.h>
//#include <stdio.h>
typedef struct FILE FILE;
extern struct FILE *stdin;
extern struct FILE *stdout;
extern struct FILE *stderr;
#define SEEK_SET        0        /* Seek from beginning of file.  */
#define SEEK_CUR        1        /* Seek from current position.  */
#define SEEK_END        2        /* Seek from end of file.  */
typedef struct {
    unsigned int gp_offset;
    unsigned int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];
//typedef __builtin_va_list va_list;
extern int printf (const char *__format, ...);
extern int fprintf (FILE *__stream, const char *__format, ...);
extern int sprintf (char *__s, const char *__format, ...);
extern int vfprintf (FILE *__s, const char *__format, va_list __arg);
extern int vsnprintf (char *__s, size_t __maxlen, const char *__format, va_list __arg);
extern FILE *fopen (const char *__filename, const char *__modes);
extern int fseek (FILE *__stream, long int __off, int __whence);
extern long int ftell (FILE *__stream);
extern int fread (void *__ptr, int __size, int __n, FILE *__stream);
extern int fclose (FILE *__stream);

//#include <stdlib.h>
#define EXIT_FAILURE    1   /* Failing exit status.  */
#define EXIT_SUCCESS    0   /* Successful exit status.  */

extern void *malloc (size_t __size);
extern void *realloc (void *__ptr, size_t __size);
extern void *calloc (size_t __nmemb, size_t __size);
extern long int strtol (const char *__nptr, char **__endptr, int __base);
extern void exit (int __status);
//#include <stdint.h>
typedef unsigned char uint8_t;
typedef long int                intptr_t;
//#include <string.h>
extern char *strdup(const char *__string);
extern char *strndup(const char *__string, int __n);
extern int strcmp (const char *__s1, const char *__s2);
extern int strncmp (const char *__s1, const char *__s2, size_t __n);
int strncasecmp(const char *s1, const char *s2, int n);
extern size_t strlen (const char *__s);
extern char *strchr (char *__s, int __c);
extern char *strcpy (char *__dest, const char *__src);
extern char *strncpy (char *__dest, const char *__src, size_t __n);
extern void *memcpy (void *__dest, const void *__src, size_t __n);
extern char *strerror (int __errnum);
//#include <sys/stat.h>
struct stat {
      char _[512];
};
int stat(char *path, struct stat *statbuf);
#define va_start __builtin_va_start
#define va_end __builtin_va_end
//#include <stdarg.h>
#define bool _Bool
#define false 0
#define true 1

//#include <ctype.h>
int isxdigit(int c);
int isspace(int c);
int isdigit(int c);
//#include <errno.h>

Type *ary_of(Type *base, size_t size);
Type *ptr_to(Type *base);
Type *bool_ty();
Type *int_ty();
Type *void_ty();
Type *char_ty();
Type *short_ty();
Type *long_ty();
Type *enum_ty();
Type *uchar_ty();
Type *ushort_ty();
Type *uint_ty();
Type *ulong_ty();
enum { IN_THEN, IN_ELSE, IN_ELIF };
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
    TK_DEFAULT, // default
    TK_BREAK, // break
    TK_CONTINUE, // continue
    TK_FOR, // forのトークン
    TK_EQ, // ==
    TK_NE, // !=
    TK_LE, // <=
    TK_GE, // >=
    TK_EOF,
    TK_INT, // int
    TK_LONG,
    TK_SHORT,
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
    TK_STATIC, // static
    TK_BOOL, // _Bool
    TK_ENUM,
    TK_DO,
    TK_SIGNED,
    TK_UNSIGNED,
    TK_ALIGNOF,
    TK_DOTS, // ...
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
    ND_DEFAULT,  // default
    ND_CONTINUE, // continue
    ND_CAST,
    ND_DO_WHILE, // do ~ while
    ND_ALIGNOF,
};

enum {
    VOID     = 1 << 0,
    BOOL     = 1 << 2,
    CHAR     = 1 << 4,
    SHORT    = 1 << 6,
    INT      = 1 << 8,
    LONG     = 1 << 10,
    OTHER    = 1 << 12,
    SIGNED   = 1 << 13,
    UNSIGNED = 1 << 14,
    PTR,
    ARRAY,
    STRUCT,
    FUNC,
    ENUM,
};

// parse.c
long const_expr_token(Vector *tokens, int pos);
Initializer *gvar_init_string(char *p, int len);
Vector *parse(Vector *tokens);
Var *new_global(Type* ty, char *name, char *data, int len, bool is_extern, bool is_static);

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

StringBuilder *new_sb();
void sb_add(StringBuilder *sb, char c);
void sb_append(StringBuilder *sb, char *s);
void sb_append_n(StringBuilder *sb, char *s, int len);
char *sb_get(StringBuilder *sb);

char *format(char *fmt, int label);
char *read_file(char *path);

// util
int roundup(int x, int align);

// Global変数
extern Node *code[100];
// トークナイズした結果のトークン列はvecに格納する

//変数名とRBPからのオフセットを管理する
extern int variables;
extern Map* variable_map;
extern Vector *globals;

// preprocess.c
Vector *preprocess(Vector* tokens);

//
// main.c
//
extern char **include_paths;
