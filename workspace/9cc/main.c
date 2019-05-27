#include "9cc.h"

// トークナイズした結果のトークン列はvecに格納する
Vector* tokens;
int main(int argc, char **argv) {
    if (argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    // トークナイズしてパースする
    tokens = new_vector();
    //fprintf(stderr, "引数: %s\n", argv[1]);
    if (strcmp(argv[1], "-test") == 0) {
        runtest();
        return 0;
    }
    char *filename = argv[1];
    char *input = read_file(filename);
    tokenize(input);
    Vector *nodes = parse();
    sema(nodes);
    gen_main(nodes);

    return 0;
}

