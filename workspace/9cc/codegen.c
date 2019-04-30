#include "9cc.h"
#include <stdio.h>
#include <stdint.h>

void gen_main() {
    gen_initial();
    gen_prolog();

    for (int i = 0; code[i]; i++) {
        // 抽象構文木を下りながらコード生成
        gen(code[i]);
        // 式の評価結果としてスタックに一つの値が残ってる
        // はずなので、スタックが溢れないようにポップしておく
        printf("  pop rax\n");
    }
    gen_epilog();
}

void gen_initial() {
    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
}

void gen_prolog() {
    // プロローグ
    // 使用した変数分の領域を確保する
    printf("  push rbp\n");                     // ベースポインタをスタックにプッシュする
    printf("  mov rbp, rsp\n");                 // rspをrbpにコピーする
    printf("  sub rsp, %d\n", variables * 8);   // rspを使用した変数分動かす
}

void gen_epilog() {
    // エピローグ
    printf("  mov rsp, rbp\n");     // ベースポインタをrspにコピーして
    printf("  pop rbp\n");          // スタックの値をrbpに持ってくる
    printf("  ret\n");
}

// 左辺値を計算する
void gen_lval(Node *node) {
    if (node->ty != ND_IDENT)
        error("代入の左辺値が変数ではありません", 0);

    //Nodeが変数の場合
    int offset = (intptr_t) map_get(variable_map, node->name);// ('z' - node->name + 1) * 8;
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

    if (node->ty == TK_EQ ||
        node->ty == TK_NE ||
        node->ty == TK_LE ||
        node->ty == TK_L  ||
        node->ty == TK_GE ||
        node->ty == TK_G) {
        if (node->ty == TK_G || node->ty == TK_GE){
            gen(node->rhs);                 // 右辺を先にスタックに乗せる
            gen(node->lhs);
        } else {
            gen(node->lhs);                 // lhsの値がスタックにのる
            gen(node->rhs);                 // rhsの値がスタックにのる
        }

        printf("  pop rdi\n");          // 左辺をrdiにpop
        printf("  pop rax\n");          // 右辺をraxにpop
        printf("  cmp rax, rdi\n");     // 2つのレジスタの値が同じかどうか比較する

        if (node->ty == TK_EQ)
            printf("  sete al\n");          // al(raxの下位8ビットを指す別名レジスタ)にcmpの結果(同じなら1/それ以外なら0)をセット
        if (node->ty == TK_NE)
            printf("  setne al\n");
        if (node->ty == TK_L || node->ty == TK_G)
            printf("  setl al\n");
        if (node->ty == TK_LE || node->ty == TK_GE)
            printf("  setle al\n");
        printf("  movzb rax, al\n");    // raxを0クリアしてからalの結果をraxに格納
        printf("  push rax\n");         // スタックに結果を積む
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