#include "9cc.h"
char **include_paths;
static char *input_file;

static void parse_args(int argc, char **argv) {
    include_paths = malloc(sizeof(char*) * argc);
    int npaths = 0;

    for (int i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "-I", 2)) {
            include_paths[npaths++] = argv[i] + 2;
            continue;
        }
        input_file = argv[i];
    }
    if (!input_file)
        error("no input files");
}

// トークナイズした結果のトークン列はvecに格納する
int main(int argc, char **argv) {
    parse_args(argc, argv);

    // トークナイズしてパースする
    Vector* tokens = tokenize(input_file, true);
    Vector *nodes = parse(tokens);
    sema(nodes);
    gen_main(nodes);

    return 0;
}

