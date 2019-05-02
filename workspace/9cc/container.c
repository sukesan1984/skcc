#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "9cc.h"

Token *add_token(Vector *v, int ty, char *input) {
    Token * token = malloc(sizeof(Token));
    token->ty = ty;
    token->input = input;
    vec_push(v, (void *)token);
    return token;
}

// 可変長Vector
// 任意蝶の入力をサポートする
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

Map *new_map() {
    Map *map = malloc(sizeof(Map));
    map->keys = new_vector();
    map->vals = new_vector();
    return map;
}

void map_put(Map *map, char *key, void *val) {
    vec_push(map->keys, key);
    vec_push(map->vals, val);
}

void *map_get(Map *map, char *key) {
    for (int i = map->keys->len -1; i >= 0; i--)
        if (strcmp(map->keys->data[i], key) == 0)
            return map->vals->data[i];
    return NULL;
}

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

void test_map() {
    Map *map = new_map();
    expect(__LINE__, 0, (long)map_get(map, "foo"));

    map_put(map, "foo", (void *) 2);
    expect(__LINE__, 2, (long)map_get(map, "foo"));

    map_put(map, "bar", (void *) 4);
    expect(__LINE__, 4, (long)map_get(map, "bar"));

    map_put(map, "foo", (void *) 6);
    expect(__LINE__, 6, (long)map_get(map, "foo"));

    printf("OK map\n");
}

void test_equality_tokenize() {
    printf("equality) tokenize \n");
    tokens = new_vector();
    char *args = "a == 1; b != 1; c <= 1; d >= 1; e < 1; f > 1;";
    tokenize(args);
    expect(__LINE__, 25, tokens->len);
}

void test_while_tokenize() {
    printf("while) tokenize \n");
    tokens = new_vector();
    char *args = "while (a != 5) a = a + 1;";
    tokenize(args);
    expect(__LINE__, 13, tokens->len);
    expect(__LINE__, TK_WHILE, ((Token *) tokens->data[0])->ty);
    expect(__LINE__, '(', ((Token *) tokens->data[1])->ty);
}

void test_for_tokenize() {
    printf("for) tokenize \n");
    tokens = new_vector();
    char *args = "b = 3; for (i = 1; i < 5; i = i + 1) b = b + 2;";
    tokenize(args);
    expect(__LINE__, 27, tokens->len);
    expect(__LINE__, TK_FOR, ((Token *) tokens->data[4])->ty);
    expect(__LINE__, '(', ((Token *) tokens->data[5])->ty);
}

void test_function_call_tokenize() {
    printf("function call) tokenize \n");
    tokens = new_vector();
    char *args = "return hoge();";
    tokenize(args);
    expect(__LINE__, 6, tokens->len);
    expect(__LINE__, TK_CALL, ((Token *) tokens->data[1])->ty);
    expect(__LINE__, '(', ((Token *) tokens->data[2])->ty);
}


void test_tokenize() {
    printf("1) tokenize\n");
    char *args = "a = 1; b = 2;return a + b;";
    tokenize(args);
    expect(__LINE__, 14, tokens->len);
    expect(__LINE__, TK_IDENT, ((Token *) (tokens->data[0]))->ty);
    expect(__LINE__, 'a', *((Token *) (tokens->data[0]))->name);
    expect(__LINE__, '=', ((Token *) (tokens->data[1]))->ty);
    expect(__LINE__, TK_NUM, ((Token *) (tokens->data[2]))->ty);
    expect(__LINE__, ';', ((Token *) (tokens->data[3]))->ty);
    expect(__LINE__, TK_IDENT, ((Token *) (tokens->data[4]))->ty);
    expect(__LINE__, 'b', *((Token *) (tokens->data[4]))->name);

    printf("2) multi char variable tokenize\n");
    tokens = new_vector();
    args = "abc = 10;";
    tokenize(args);
    expect(__LINE__, 5, tokens->len);
    
    test_equality_tokenize();

    printf("3) if tokenize \n");
    tokens = new_vector();
    args = "if (a) 3;";
    tokenize(args);
    expect(__LINE__, 7, tokens->len);
    expect(__LINE__, TK_IF, ((Token *) (tokens->data[0]))->ty);
    expect(__LINE__, '(', ((Token *) (tokens->data[1]))->ty);
    expect(__LINE__, TK_IDENT, ((Token *) (tokens->data[2]))->ty);
    expect(__LINE__, ')', ((Token *) (tokens->data[3]))->ty);
    expect(__LINE__, TK_NUM, ((Token *) (tokens->data[4]))->ty);
    expect(__LINE__, ';', ((Token *) (tokens->data[5]))->ty);
    expect(__LINE__, TK_EOF, ((Token *) (tokens->data[6]))->ty);

    test_while_tokenize();
    test_for_tokenize();
    test_function_call_tokenize();

    printf("tokenize OK\n");
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

    test_tokenize();

    test_map();

    printf("OK\n");
}

