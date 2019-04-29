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

