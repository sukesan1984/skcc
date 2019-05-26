#include "9cc.h"

char* argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char* argreg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char* argreg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};

static char *escape(char *s, int len) {
    char *buf = malloc(len * 4);
    char *p = buf;
    for (int i = 0; i < len; i++) {
        if(s[i] == '\\') {
            *p++ = '\\';
            *p++ = '\\';
        } else if (isgraph(s[i]) || s[i] == ' ') {
            *p++ = s[i];
        } else {
            sprintf(p, "\\%03o", s[i]);
            p += 4;
        }
    }
    *p = '\0';
    return buf;
}

void gen_initial() {
    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".data\n");
    for (int i = 0; i < globals->len; i++) {
        Var *var = globals->data[i];
        printf("%s:\n", var->name);
        printf("  .ascii \"%s\"\n", escape(var->data, var->len));
    }

    for (int i = 0; i < strings->len; i++) {
        Node *node = strings->data[i];
        assert(node->op == ND_STR);
        printf("%s:\n", node->name);
        printf("  .asciz \"%s\"\n", node->str);

    }

    printf(".global main\n");
}

void gen_epilog() {
    // エピローグ
    printf("#gen_epilog\n");
    printf("  mov rsp, rbp # ベースポインタをrspにコピーして \n");     // ベースポインタをrspにコピーして
    printf("  pop rbp      # スタックの値をrbpに持ってくる\n");        // スタックの値をrbpに持ってくる
    printf("  ret          # return \n");
}
void gen_expr(Node *node);

void gen_binop(Node *lhs, Node *rhs){
    gen_expr(lhs);
    gen_expr(rhs);
}

void gen_lval(Node *node);
void gen_stmt(Node *node);

int jump_num = 0;                    // ifでjumpする回数を保存
void gen_expr(Node *node){
    switch(node->op) {
        case ND_NUM: {
            printf("#gen_expr ND_NUM \n");
            printf("  push %d        # スタックに数字を積む\n", node->val);
            return;
        }
        // 変数に格納
        case '=': {
            // 左辺が変数であるはず
            gen_lval(node->lhs);            // ここでスタックのトップに変数のアドレスが入っている
            gen_expr(node->rhs);            // 右辺値が評価されてスタックのトップに入っている

            printf("#gen_expr =の処理開始\n");
            printf("  pop rdi        # 評価された右辺値をrdiにロード\n");          // 評価された右辺値がrdiにロード
            printf("  pop rax        # 左辺の変数のアドレスがraxに格納\n");          // 変数のアドレスがraxに格納

            char *reg = "rdi";
            if (node->lhs->ty->ty == INT)
                reg = "edi";
            else if(node->lhs->ty->ty == CHAR)
                reg = "dil";

            printf("  mov [rax], %s # raxのレジスタのアドレスにrdiの値をストアする(この場合左辺のアドレスに右辺の評価値を書き込む) \n", reg);   // raxのレジスタのアドレスにrdiの値をストアする
            printf("  push rdi       # 右辺値をスタックにプッシュする\n");         // rdiの値をスタックにプッシュする
            return;
        }

        case ND_CALL: {
            printf("#gen_expr ND_CALL開始:\n");
            int args_len = node->args->len;
            for (int i = 0; i < args_len; i++) {
                gen_expr((Node *)  node->args->data[i]);         // スタックに引数を順に積む
                printf("#gen_expr ND_CALL(引数処理): %d番目の引数\n", i);
                printf("  pop rax      # スタックされた引数の評価値をスタックからraxに格納\n");                     // 結果をraxに格納
                printf("  mov %s, rax # raxには引数が積まれているので、各レジスタに値を格納\n", argreg[i]);        // raxから各レジスタに格納
            }
            printf("  call %s       #関数呼び出し \n", node->name);         // 関数の呼び出し
            printf("  push rax      #関数の結果をスタックに積む \n");         // スタックに結果を積む
            return;
        }

        case ND_GVAR:
        case ND_LVAR: {
            printf("#gen_expr ND_IDENTの処理開始\n");
            gen_lval(node);
            printf("  pop rax        # 左辺値がコンパイルされた結果をスタックからraxにロード\n");          // スタックからpopしてraxに格納
            char *reg = "rax";
            if (node->ty->ty == INT)
                reg = "eax";
            else if(node->ty->ty == CHAR) {
                printf("  push rax\n");
                return;
            }
            printf("  mov %s, [rax] # raxをアドレスとして値をロードしてraxに格納(この場合左辺値のアドレスに格納された値がraxに入る)\n", reg);   // raxをアドレスとして値をロードしてraxに格納
            printf("  push rax       # 結果をスタックに積む\n");         // スタックにraxをpush
            return;
        }

        case ND_DEREF: {
            printf("#gen_expr ND_DEREFが右辺にきたときの処理開始\n");
            gen_lval(node);
            printf("#スタックに値を格納したいさきのアドレスが載ってる\n");
            printf("  pop rax        \n");          // スタックからpopしてraxに格納
            char *reg = "rax";
            if (node->ty->ty == INT)
                reg = "eax";
            else if(node->ty->ty == CHAR) {
                printf("  mov al, [rax] # デリファレンスのアドレスから値をロード\n");   // raxをアドレスとして値をロードしてraxに格納
                printf("  movzb rax, al\n");
                printf("  push rax       # デリファレンス後の値の結果をスタックに積む\n");         // スタックにraxをpush
                return;
            }
            printf("  mov %s, [rax] # デリファレンスのアドレスから値をロード\n", reg);   // raxをアドレスとして値をロードしてraxに格納
            printf("  push rax       # デリファレンス後の値の結果をスタックに積む\n");         // スタックにraxをpush
            return;
        }

        case ND_ADDR: {
            // 左辺値としてコンパイルすると変数のアドレスが取得できる
            gen_lval(node->lhs);
            return;
        }
        case ND_EQ:
        case ND_NE:
        case ND_LE:
        case '<':
        case '>': {
            printf("#gen_expr == != <= < >などの処理開始\n");
            printf("#評価の左辺をスタックに乗せる \n");
            gen_expr(node->lhs);                 // lhsの値がスタックにのる
            printf("#評価の右辺をスタックに乗せる \n");
            gen_expr(node->rhs);                 // rhsの値がスタックにのる

            printf("  pop rdi      # 右辺をrdiにpop\n");          // 左辺をrdiにpop
            printf("  pop rax      # 左辺をraxにpop\n");          // 右辺をraxにpop
            printf("  cmp rax, rdi # 左辺と右辺が同じかどうかを比較する\n");     // 2つのレジスタの値が同じかどうか比較する

            if (node->op == ND_EQ)
                printf("  sete al   # al(raxの下位8ビットを指す別名レジスタ)にcmpの結果(同じなら1/それ以外なら0)をセット\n");          // al(raxの下位8ビットを指す別名レジスタ)にcmpの結果(同じなら1/それ以外なら0)をセット
            if (node->op == ND_NE)
                printf("  setne al  # != \n");
            if (node->op == '<' || node->op == '>')
                printf("  setl al   # < \n");
            if (node->op == ND_LE)
                printf("  setle al  # <= \n");
            printf("  movzb rax, al # raxを0クリアしてからalの結果をraxに格納\n");    // raxを0クリアしてからalの結果をraxに格納
            printf("  push rax      # スタックに結果を積む\n");         // スタックに結果を積む
            return;
        }
        case ND_LOGOR:
            // 左辺と右辺の内いずれかが1なら1
            gen_expr(node->lhs);
            printf("  pop rdi\n");
            printf("  cmp rdi, 1 # 1と等しければje..\n");
            printf("  je .Ltrue%d   # 0なら.Lend%dに飛ぶ\n", jump_num, jump_num);      // lhsが0のとき（false) Lendに飛ぶ
            gen_expr(node->rhs);
            printf("  pop rdi\n");
            printf("  cmp rdi, 1 # 1と等しければ..\n");
            printf("  je .Ltrue%d   # 0なら.Lend%dに飛ぶ\n", jump_num, jump_num);      // lhsが0のとき（false) Lendに飛ぶ
            printf("  push 0\n"); // 両方0の時
            printf("  jmp .Lend%d\n", jump_num);
            printf(".Ltrue%d:\n", jump_num);
            printf("  push 1\n");
            printf(".Lend%d:\n", jump_num);
            jump_num++;
            return;
        case ND_LOGAND:
            // 左辺と右辺の内いずれかが1なら1
            gen_expr(node->lhs);
            printf("  pop rdi\n");
            printf("  cmp rdi, 0 # 0と等しければje..\n");
            printf("  je .Lfalse%d   # 0なら.Lend%dに飛ぶ\n", jump_num, jump_num);      // lhsが0のとき（false) Lendに飛ぶ
            gen_expr(node->rhs);
            printf("  pop rdi\n");
            printf("  cmp rdi, 0 # 0と等しければ..\n");
            printf("  je .Lfalse%d   # 0なら.Lend%dに飛ぶ\n", jump_num, jump_num);      // lhsが0のとき（false) Lendに飛ぶ
            printf("  push 1\n"); // 両方0の時
            printf("  jmp .Lend%d\n", jump_num);
            printf(".Lfalse%d:\n", jump_num);
            printf("  push 0\n");
            printf(".Lend%d:\n", jump_num);
            jump_num++;
            return;
        case '+':
            printf("#gen_expr +の評価開始\n");
            gen_expr(node->lhs);
            gen_expr(node->rhs);
            printf("  pop rdi       # \n");
            if (node->rhs->ty->ty == PTR) {
                printf("  pop rax #左辺の値を取り出して、stacksizeの大きさをかける\n");
                printf("  mov rsi, %d\n", size_of(node->rhs->ty->ptr_of));//node->lhs->stacksize);
                printf("  mul rsi\n");
                printf("  push rax\n");
            }
            if (node->lhs->ty->ty == PTR) {
                printf("  mov rax, rdi\n");
                printf("  mov rsi, %d\n", size_of(node->lhs->ty->ptr_of));//node->lhs->stacksize);
                printf("  mul rsi\n");
                printf("  mov rdi, rax\n");
            }
            printf("  pop rax\n");
            printf("  add rax, rdi\n");
            break;
        case '-':
            printf("#gen_expr -の評価開始\n");
            gen_expr(node->lhs);
            gen_expr(node->rhs);
            printf("  pop rdi\n");
            if (node->rhs->ty->ty == PTR) {
                printf("  pop rax #左辺の値を取り出して、stacksizeの大きさをかける\n");
                printf("  mov rsi, %d\n", size_of(node->rhs->ty->ptr_of)); //node->lhs->stacksize);
                printf("  mul rsi\n");
                printf("  push rax\n");
            }
            if (node->lhs->ty->ty == PTR) {
                printf("  mov rax, rdi\n");
                printf("  mov rsi, %d\n", size_of(node->lhs->ty->ptr_of));
                printf("  mul rsi\n");
                printf("  mov rdi, rax\n");
            }
            printf("  pop rax\n");
            printf("  sub rax, rdi\n");
            break;
        case '*':
            printf("#gen_expr *の評価開始\n");
            gen_binop(node->lhs, node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mul rdi\n");
            break;
        case '/':
            printf("#gen_expr /の評価開始\n");
            gen_binop(node->lhs, node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov rdx, 0\n");
            printf("  div rdi\n");
    }
    printf("  push rax   #gen_exprの結果をスタックに積む\n");
    return;
}

// 左辺値を計算する
void gen_lval(Node *node) {
    printf("#gen_lvalの評価開始\n");
    if (node->op == ND_DEREF)
        return gen_expr(node->lhs);

    if (node->op != ND_LVAR && node->op != ND_GVAR && node->op != ND_VARDEF)
        error("代入の左辺値が変数ではありません", 0);

    if (node->op == ND_LVAR || node->op == ND_VARDEF){
        printf("  mov rax, rbp # 関数のベースポインタをraxにコピー\n");         // ベースポインタをraxにコピー
        printf("  sub rax, %d   # raxを%sのoffset:%d分だけ押し下げたアドレスが%sの変数のアドレス。それをraxに保存)\n", node->offset, node->name, node->offset, node->name);  // raxをoffset文だけ押し下げ（nameの変数のアドレスをraxに保存)
        printf("  push rax      # 結果をスタックに積む(変数のアドレスがスタック格納されてる) \n");             // raxをスタックにプッシュ
        return;
    }

    assert(node->op == ND_GVAR);
    printf("  lea rax, %s\n", node->name);
    printf("  push rax\n");
    return;
}

// 関数のプロローグ
// args: 関数の引数
void gen_args(Vector *args) {
    int args_len = args->len;                   // argsのlengthを取得
    for(int i = 0; i < args_len; i++) {
        Node *node = args->data[i];
        gen_lval(node);       // 関数の引数定義はlvalとして定義
        printf("  pop rax        # 第%d引数の変数のアドレスがraxに格納\n", i);          // 変数のアドレスがraxに格納

        if (node->ty->ty == INT)
            printf("  mov [rax], %s # raxのレジスタのアドレスに呼び出し側で設定したレジスタの中身をストア\n", argreg32[i]);
        else if (node->ty->ty == CHAR)
            printf("  mov [rax], %s # raxのレジスタのアドレスに呼び出し側で設定したレジスタの中身をストア\n", argreg8[i]);
        else
            printf("  mov [rax], %s # raxのレジスタのアドレスに呼び出し側で設定したレジスタの中身をストア\n", argreg[i]);   // raxのレジスタのアドレスに呼び出し側で設定したレジスタの中身をストア
    }
}

void gen(Node *node);
void gen_stmt(Node *node) {
    if (node->op == ND_VARDEF) {
        printf("#gen_stmt ND_VARDEFの処理がここはいる\n");
        if (!node->init)
            return;
        gen_expr(node->init);
        // 右辺の結果がスタックに入る入る
        printf("  mov rax, rbp # 関数のベースポインタをraxにコピー\n");         // ベースポインタをraxにコピー
        printf("  sub rax, %d   # raxを%sのoffset:%d分だけ押し下げたアドレスが%sの変数のアドレス。それをraxに保存)\n", node->offset, node->name, node->offset, node->name);  // raxをoffset文だけ押し下げ（nameの変数のアドレスをraxに保存)
        printf("  push rax      # 結果をスタックに積む(変数のアドレスがスタック格納されてる) \n");             // raxをスタックにプッシュ
        printf("  pop rax       # 代入すべきアドレスがスタックされている\n");
        printf("  pop rdi       # 宣言時の右辺の値が入っている\n");

        char *reg = "rdi";
        if (node->ty->ty == INT)
            reg = "edi";
        else if(node->ty->ty == CHAR)
            reg = "dil";
        printf("  mov [rax], %s\n", reg);

        return;
    }

    // if(lhs) rhsをコンパイル
    if (node->op == ND_IF) {
        printf("#gen_stmt IFの処理\n");
        gen_expr(node->lhs);                             // lhsの結果をスタックにpush
        printf("  pop rax      # lhsの結果をraxにコピー\n");                      // lhsの結果をraxにコピー
        printf("  cmp rax, 0   # raxの結果と0を比較する(if文の中身がfalseのときは1)\n");                   // raxの結果と0を比較
        printf("  je .Lend%d   # 0なら.Lend%dに飛ぶ\n", jump_num, jump_num);      // lhsが0のとき（false) Lendに飛ぶ
        gen_stmt(node->rhs);                             // rhsの結果をスタックにpush
        printf(".Lend%d:\n", jump_num);          // 終わる
        printf("  push %d      # if文の結果がfalseの場合は0をスタックに積む\n", 0);                   // Lendのときは0をstackに積む
        jump_num++;
        return;
    }

    // for(lhs, lhs2, lhs3) rhsをコンパイル
    if (node->op == ND_FOR) {
        printf("#gen_stmt FORの処理\n");
        gen_expr(node->lhs);                     // lhsをまず実行してスタックに積む
        printf(".Lbegin%d:      # ループの開始\n", jump_num);   // ループの開始
        gen_expr(node->lhs2);                    // lhs2の実行結果をスタックに積む
        printf("  pop rax       # 二つ目の実行結果をraxに格納\n");              // lhs2の実行結果をraxに格納
        printf("  cmp rax, 0    # 0と等しい(二つ目がfalseになったら)終わる\n");           // lhsの実行結果が0と等しい。falseになったらおわる
        printf("  je .Lend%d\n", jump_num);
        printf("# for文の内部を処理\n");
        gen_stmt(node->rhs);                     // rhsを実行
        printf("# forの３つ目の領域の処理実行\n");
        gen_expr(node->lhs3);                    // lhs3の実行結果をスタックに積む
        printf("  jmp .Lbegin%d # ループの開始に飛ぶ\n", jump_num);// ループの開始に戻る
        printf(".Lend%d:        # for文終わり\n", jump_num);
        jump_num++;
        return;
    }

    // while(lhs) rhsをコンパイル
    if (node->op == ND_WHILE) {
        printf("#gen_stmt WHILEの処理\n");
        printf("  .Lbegin%d: # ループの開始\n", jump_num);      // ループの開始
        gen_expr(node->lhs);                         // lhsをコンパイルしてスタックにpush
        printf("  pop rax      # while評価の結果を格納(0 or 1) \n");                  // raxにstackを格納
        printf("  cmp rax, 0   # while評価が0ならば終わり\n");               // rhsの結果が0のとき(falseになったら) Lendに飛ぶ
        printf("  je .Lend%d\n", jump_num);
        printf("#WHILEの中身の処理\n");
        gen_stmt(node->rhs);                         // ループの中身をコンパイル
        printf("  jmp .Lbegin%d\n", jump_num);  // ループの開始時点に戻る
        printf(".Lend%d:\n", jump_num);
        jump_num++;
        return;
    }

    if (node->op == ND_RETURN) {
        printf("#gen_stmt ND_RETURNの処理\n");
        gen_expr(node->lhs);
        printf("  pop rax #returnしたい結果がスタックに入っているのでをraxにロード\n");          // genで生成された値をraxにpopして格納

        //関数のエピローグ
        printf("# return したのでエピローグ\n");
        printf("  mov rsp, rbp # ベースポインタをrspに格納\n");     // ベースポインタをrspにコピーして
        printf("  pop rbp      # スタックには呼び出し元のrbpが入ってるはずなのでrbpにロードする\n");          // スタックの値をrbpに持ってくる
        printf("  ret          # returnする\n");
        return;
    }

    if (node->op == ND_EXPR_STMT) {
        printf("#gen_stmt ND_EXPR_STMTの処理\n");
        gen_expr(node->lhs);
        return;
    }

    if (node->op == ND_COMP_STMT) {
        printf("#gen_stmt ND_COMP_STMTの処理\n");
        int stmt_len = node->stmts->len;
        for (int i = 0; i < stmt_len; i++) {
            printf("# %d番目のステートメントの処理\n", i);
            gen_stmt(node->stmts->data[i]);
        }
        return;
    }
}

void gen_main(Vector* v) {
    gen_initial();

    int len = v->len;
    for (int i = 0; i < len; i++) {
        Node *node = v->data[i];
        // 関数定義
        if (node->op == ND_FUNC) {
            printf("#ND_FUNCの処理\n");
            printf("%s:\n", node->name);
            // プロローグ
            // 使用した変数分の領域を確保する
            printf("  push rbp     # 呼び出し元のrbpをスタックにつんでおく(エピローグでpopしてrspをそこに戻す)\n");                     // ベースポインタをスタックにプッシュする
            printf("  mov rbp, rsp # rspが今の関数のベースポインタを指しているのでrbpにコピーしておく\n");                 // rspをrbpにコピーする
            printf("  sub rsp, %d  # 今の関数で使用する変数(%d個)の数だけrspを動かす\n", node->stacksize, node->stacksize);   // rspを使用した変数分動かす
            printf("#関数の引数の処理\n");
            gen_args(node->args);
            printf("#関数本体の処理\n");
            gen_stmt(node->body);
            // 抽象構文木を下りながらコード生成
            // 式の評価結果としてスタックに一つの値が残ってる
            // はずなので、スタックが溢れないようにポップしておく
            printf("  pop rax     # 関数の結果をraxにロード\n");
        } else if (node->op == ND_VARDEF) {
            continue;
        } else {
            fprintf(stderr, "node->op must be ND_FUNC but got: %d", node->op);
            exit(1);
        }
    }

    gen_epilog();
}
