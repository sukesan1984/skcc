#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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

