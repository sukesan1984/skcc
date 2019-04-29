#include "9cc.h"
#include <stdio.h>

// 左辺値を計算する
void gen_lval(Node *node) {
    if (node->ty != ND_IDENT)
        error("代入の左辺値が変数ではありません", 0);

    //Nodeが変数の場合
    int offset = ('z' - node->name + 1) * 8;
    printf("  mov rax, rbp\n");         // ベースポインタをraxにコピー
    printf("  sub rax, %d\n", offset);  // raxをoffset文だけ押し下げ（nameの変数のアドレスをraxに保存)
    printf("  push rax\n");             // raxをスタックにプッシュ
}

void gen(Node *node) {
    if (node->ty == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    if (node->ty == ND_IDENT) {
        gen_lval(node);
        printf("  pop rax\n");          // スタックからpopしてraxに格納
        printf("  mov rax, [rax]\n");   // raxをアドレスとして値をロードしてraxに格納
        printf("  push rax\n");         // スタックにraxをpush
        return;
    }

    if (node->ty == ND_RETURN) {
        gen(node->lhs);
        printf("  pop rax\n");          // genで生成された値をraxにpopして格納

        //関数のエピローグ
        printf("  mov rsp, rbp\n");     // ベースポインタをrspにコピーして
        printf("  pop rbp\n");          // スタックの値をrbpに持ってくる
        printf("  ret\n");
        return;
    }

    // 変数に格納
    if (node->ty == '=') {
        // 左辺が変数であるはず
        gen_lval(node->lhs);            // ここでスタックのトップに変数のアドレスが入っている
        gen(node->rhs);                 // 右辺値が評価されてスタックのトップに入っている

        printf("  pop rdi\n");          // 評価された右辺値がrdiにロード
        printf("  pop rax\n");          // 変数のアドレスがraxに格納
        printf("  mov [rax], rdi\n");   // raxのレジスタのアドレスにrdiの値をストアする
        printf("  push rdi\n");         // rdiの値をスタックにプッシュする
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch(node->ty) {
        case '+':
            printf("  add rax, rdi\n");
            break;
        case '-':
            printf("  sub rax, rdi\n");
            break;
        case '*':
            printf("  mul rdi\n");
            break;
        case '/':
            printf("  mov rdx, 0\n");
            printf("  div rdi\n");
    }
    printf("  push rax\n");
}
