#include "9cc.h"
#include <stdio.h>
#include <string.h>

Node *code[100];
// トークナイズした結果のトークン列はvecに格納する
Vector* tokens;
Map* variable_map;
int main(int argc, char **argv) {
    if (argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    // トークナイズしてパースする
    tokens = new_vector();
    variable_map = new_map();
    //fprintf(stderr, "引数: %s\n", argv[1]);
    if (strcmp(argv[1], "-test") == 0) {
        runtest();
        return 0;
    }
    tokenize(argv[1]);
    program();
    gen_main();

    return 0;
}

