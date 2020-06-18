#include "9cc.h"

char* argreg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char* argreg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char* argreg16[] = { "di", "si", "dx", "cx", "r8w", "r9w" };
char* argreg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};

static char *reg(int idx) {
    char *r[] = {"r10", "r11", "r12", "r13", "r14", "r15"};
    if (idx < 0 || sizeof(r) / sizeof(*r) <= idx) {
        fprintf(stderr, "register out of range: %d", idx);
        exit(1);
    }
    return r[idx];
}

int stack = 0;

void load(Type* ty, int idx) {
    char *r;
    if (ty->size == 1) {
        r = reg(idx);
        printf("  mov %sb, [rax]\n", r);
        if (ty->is_unsigned)
            printf("  movzx %s, %sb\n", r, r);
        else
            printf("  movsx %s, %sb\n", r, r);
    } else if (ty->size == 2) {
        r = reg(idx);
        printf("  mov %sw, [rax]\n", r);
        if (ty->is_unsigned)
            printf("  movzx %s, %sw\n", r, r);
        else
            printf("  movsx %s, %sw\n", r, r);
    } else if (ty->size == 4) {
        r = reg(idx);
        printf("  mov %sd, [rax]\n", r);
        printf("  movsx %s, %sd\n", r, r);
    } else {
        r = reg(idx);
        printf("  mov %s, [rax]\n", r);
    }
}

void store(Type* ty, int idx) {
    if (ty->size == 1)
        printf("  mov [rax], %sb\n", reg(idx));
    else if (ty->size == 2)
        printf("  mov [rax], %sw\n", reg(idx));
    else if (ty->size == 4)
        printf("  mov [rax], %sd\n", reg(idx));
    else
        printf("  mov [rax], %s\n", reg(idx));
}

void gen_initial() {
    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".data\n");
    for (int i = 0; i < globals->len; i++) {
        Var *var = globals->data[i];
        if (var->is_extern) {
            continue;
        } else {
            if (!var->is_static)
                printf(".globl %s\n", var->name);
            printf("%s:\n", var->name);
            if (!var->initializer) {
                printf("  .zero %d\n", var->ty->size);
                continue;
            }
            for (Initializer *init = var->initializer; init; init = init->next) {
                if (init->label)
                    printf("  .quad %s\n", init->label);
                else if(init->sz == 1)
                    printf("  .byte %ld\n", init->val);
                else
                    printf("  .%dbyte %ld\n", init->sz, init->val);
            }
        }
    }

    printf(".text\n");
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

static void divmod(Node *node, char *r64) {
    printf("#gen_expr /の評価開始\n");
    gen_binop(node->lhs, node->rhs);
    if ((node->lhs->ty->size) == 8) {
        printf("  pop r10\n");
        printf("  pop rax \n");
        if (node->lhs->ty->is_unsigned) {
            printf("  mov rdx, 0\n");
            printf("  div r10\n");
        } else {
            printf("  cqo\n");
            printf("  idiv r10\n");
        }
        printf("  push %s   #/の値をstackにつむ\n", r64);
    } else {
        printf("  pop r10\n");
        printf("  pop rax\n");
        if (node->lhs->ty->is_unsigned) {
            printf("  mov edx, 0\n");
            printf("  div r10d\n");
        } else {
            printf("  cdq\n");
            printf("  idiv r10d\n");
        }
        printf("  push %s   #/の値をstackにつむ\n", r64);
    }
}

void gen_expr(Node *node){
    int seq;
    switch(node->op) {
        case ND_NUM: {
            printf("#gen_expr ND_NUM \n");
            if (node->ty->ty == LONG) {
                printf("  movabs rax, %ld\n", node->val);
            } else {
                printf("  mov rax, %ld\n", node->val);
            }
            printf("  push rax\n");
            return;
        }
        // 変数に格納
        case '=': {
            // 左辺、右辺ともにSTRUCTの場合は、1バイトずつロードする
            if (node->lhs->ty->ty == STRUCT && node->rhs->ty->ty == STRUCT) {
                gen_lval(node->lhs);            // ここでスタックのトップに変数のアドレスが入っている
                gen_lval(node->rhs);            // 右辺値が評価されてスタックのトップに入っている

                printf("#gen_expr =の処理開始\n");
                printf("  pop r10        # 評価された右辺値をr10にロード\n");          // 評価された右辺値がr10にロード
                printf("  pop r11        # 左辺の変数のアドレスがr11に格納\n");          // 変数のアドレスがraxに格納
                for(int i = 0; i < node->lhs->ty->size; i++) {
                    printf("  mov al, [r10 + %d]\n", i);
                    printf("  mov [r11 + %d], al\n", i);
                }
                load(node->lhs->ty, 0);
                printf("  push r10\n");
                return;
            }
            gen_lval(node->lhs);            // ここでスタックのトップに変数のアドレスが入っている
            gen_expr(node->rhs);            // 右辺値が評価されてスタックのトップに入っている

            printf("#gen_expr =の処理開始\n");
            printf("  pop r10        # 評価された右辺値をr10にロード\n");          // 評価された右辺値がr10にロード

            char *reg = "r10";
            if (node->lhs->ty->ty == BOOL) {
                printf("  mov rax, 0\n");
                printf("  cmp rax, r10\n");
                printf("  setne al  # != \n");
                printf("  movzx r10, al # r10を0クリアしてからalの結果をr10に格納\n");    // raxを0クリアしてからalの結果をraxに格納
                printf("  pop rax        # 左辺の変数のアドレスがraxに格納\n");          // 変数のアドレスがraxに格納
                printf("  mov [rax], r10b\n");
                printf("  push r10\n");
                return;
            }
            printf("  pop rax        # 左辺の変数のアドレスがraxに格納\n");          // 変数のアドレスがraxに格納
            store(node->lhs->ty, 0);
            printf("  push r10       # 右辺値をスタックにプッシュする\n");         // r10の値をスタックにプッシュする
            return;
        }

        case ND_CALL: {
            printf("#gen_expr ND_CALL開始:\n");
            int args_len = node->args->len;
            for (int i = 0; i < args_len; i++) {
                printf("#先にND_NUMがここで処理されるのではない？\n");
                printf("#gen_expr ND_CALL(引数処理): %d番目の引数\n", i);
                gen_expr(node->args->data[i]);         // スタックに引数を順に積む
            }
            for (int i = args_len - 1; i >= 0; i--) {
                printf("  pop rax      # スタックされた引数の評価値をスタックからraxに格納\n");                     // 結果をraxに格納
                printf("  mov %s, rax # raxには引数が積まれているので、各レジスタに値を格納\n", argreg64[i]);        // raxから各レジスタに格納
            }
            seq = nlabel++;
            printf("  mov rax, rsp\n");
            printf("  and rax, 15\n");
            printf("  jnz .Lcall%d\n", seq);
            printf("  mov rax, 0\n");
            printf("  call %s       #関数呼び出し \n", node->name);         // 関数の呼び出し
            printf("  jmp .Lend%d\n", seq);
            printf(".Lcall%d:\n", seq);
            printf("  sub rsp, 8\n");
            printf("  mov rax, 0\n");
            printf("  call %s       #関数呼び出し \n", node->name);         // 関数の呼び出し
            printf("  add rsp, 8\n");
            printf(".Lend%d: # RSPのスタックフレームが16の倍数 \n", seq);
            if (node->ty->size == 1) {
                if (node->ty->is_unsigned)
                    printf("  movzx rax, al\n");
                else
                    printf("  movsx rax, al\n");
            } else if (node->ty->size == 2) {
                printf("  movsx rax, ax\n");
            } else if (node->ty->size == 4) {
                printf("  movsx rax, eax\n");
            }
            printf("  push rax      #関数の結果をスタックに積む \n");         // スタックに結果を積む
            return;
        }

        case ND_GVAR:
        case ND_LVAR: {
            printf("#gen_expr ND_LVARの処理開始 \n");
            gen_lval(node);
            printf("  pop rax        # 左辺値がコンパイルされた結果をスタックからraxにロード\n");          // スタックからpopしてraxに格納
            char *reg = "rax";
            if (node->ty->ty == BOOL) {
                printf("  mov al, [rax]\n");
                printf("  mov r10b, 0\n");
                printf("  cmp al, r10b # 左辺と右辺が同じかどうかを比較する\n");     // 2つのレジスタの値が同じかどうか比較する
                printf("  setne al  # != \n");
                printf("  movzx rax, al # raxを0クリアしてからalの結果をraxに格納\n");    // raxを0クリアしてからalの結果をraxに格納
                printf("  push rax\n");
                return;
            }
            load(node->ty, 0);
            printf("  push r10       # 結果をスタックに積む\n");         // スタックにraxをpush
            return;
        }

        case ND_DOT:
        case ND_DEREF: {
            printf("#gen_expr ND_DEREF/ND_DOTが右辺にきたときの処理開始\n");
            gen_lval(node);
            printf("#スタックに値を格納したいさきのアドレスが載ってる\n");
            printf("  pop rax        \n");          // スタックからpopしてraxに格納
            char *reg = "rax";
            if (node->ty->ty == BOOL) {
                printf("  mov al, [rax]\n");
                printf("  mov r10b, 0\n");
                printf("  cmp al, r10b # 左辺と右辺が同じかどうかを比較する\n");     // 2つのレジスタの値が同じかどうか比較する
                printf("  setne al  # != \n");
                printf("  movzx rax, al # raxを0クリアしてからalの結果をraxに格納\n");    // raxを0クリアしてからalの結果をraxに格納
                printf("  push rax\n");
                return;
            }
            load(node->ty, 0);
            printf("  push r10\n");         // スタックにraxをpush
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

            printf("  pop r10      # 右辺をr10にpop\n");          // 左辺をr10にpop
            printf("  pop rax      # 左辺をraxにpop\n");          // 右辺をraxにpop
            printf("  cmp rax, r10 # 左辺と右辺が同じかどうかを比較する\n");     // 2つのレジスタの値が同じかどうか比較する

            if (node->op == ND_EQ)
                printf("  sete al   # al(raxの下位8ビットを指す別名レジスタ)にcmpの結果(同じなら1/それ以外なら0)をセット\n");          // al(raxの下位8ビットを指す別名レジスタ)にcmpの結果(同じなら1/それ以外なら0)をセット
            if (node->op == ND_NE)
                printf("  setne al  # != \n");
            if (node->op == '<' || node->op == '>') {
                if (node->lhs->ty->is_unsigned)
                    printf("  setb al   # < \n");
                else
                    printf("  setl al   # < \n");
            }
            if (node->op == ND_LE) {
                if (node->lhs->ty->is_unsigned)
                    printf("  setbe al  # <= \n");
                else
                    printf("  setle al  # <= \n");
            }
            printf("  movzx rax, al # raxを0クリアしてからalの結果をraxに格納\n");    // raxを0クリアしてからalの結果をraxに格納
            printf("  push rax      # スタックに結果を積む\n");         // スタックに結果を積む
            return;
        }
        case ND_LOGOR:
            // 左辺と右辺の内いずれかが1なら1
            gen_expr(node->lhs);
            seq = nlabel++;
            printf("  pop r10\n");
            printf("  cmp r10, 1 # 1と等しければje..\n");
            printf("  je .Ltrue%d   # 0なら.Lend%dに飛ぶ\n", seq, seq);      // lhsが0のとき（false) Lendに飛ぶ
            gen_expr(node->rhs);
            printf(" pop r10\n");
            printf("  cmp r10, 1 # 1と等しければ..\n");
            printf("  je .Ltrue%d   # 0なら.Lend%dに飛ぶ\n", seq, seq);      // lhsが0のとき（false) Lendに飛ぶ
            printf("  push 0\n"); // 両方0の時
            printf("  jmp .Lend%d\n", seq);
            printf(".Ltrue%d:\n", seq);
            printf("  push 1\n");
            printf(".Lend%d:\n", seq);
            return;
        case ND_LOGAND:
            // 左辺と右辺の内いずれかが1なら1
            gen_expr(node->lhs);
            printf("  pop r10\n");
            printf("  cmp r10, 0 # 0と等しければje..\n");
            seq = nlabel++;
            printf("  je .Lfalse%d   # 0なら.Lend%dに飛ぶ\n", seq, seq);      // lhsが0のとき（false) Lendに飛ぶ
            gen_expr(node->rhs);
            printf("  pop r10\n");
            printf("  cmp r10, 0 # 0と等しければ..\n");
            printf("  je .Lfalse%d   # 0なら.Lend%dに飛ぶ\n", seq, seq);      // lhsが0のとき（false) Lendに飛ぶ
            printf("  push 1\n"); // 両方0の時
            printf("  jmp .Lend%d\n", seq);
            printf(".Lfalse%d:\n", seq);
            printf("  push 0\n");
            printf(".Lend%d:\n", seq);
            return;
        case '&':
            gen_binop(node->lhs, node->rhs);
            printf("  pop r10 \n");
            printf("  pop rax \n");
            printf("  and rax, r10 # & を計算 raxに結果が格納されている\n");
            printf("  push rax \n");
            return;
        case '^':
            gen_binop(node->lhs, node->rhs);
            printf("  pop r10 \n");
            printf("  pop rax \n");
            printf("  xor rax, r10 # ^ を計算 raxに結果が格納されている\n");
            printf("  push rax \n");
            return;
        case '|':
            gen_binop(node->lhs, node->rhs);
            printf("  pop r10 \n");
            printf("  pop rax \n");
            printf("  or rax, r10 # | を計算 raxに結果が格納されている\n");
            printf("  push rax \n");
            return;
        case ND_LSHIFT:
            gen_binop(node->lhs, node->rhs);
            printf("  pop rcx \n");
            printf("  pop rax \n");
            printf("  shl rax, cl\n");
            printf("  push rax\n");
            return;
        case ND_RSHIFT:
            gen_binop(node->lhs, node->rhs);
            printf("  pop rcx \n");
            printf("  pop rax \n");
            if (node->lhs->ty->is_unsigned)
                printf("  shr rax, cl\n");
            else
                printf("  sar rax, cl\n");
            printf("  push rax\n");
            return;
        case '+':
            printf("#gen_expr +の評価開始\n");
            gen_expr(node->lhs);
            gen_expr(node->rhs);
            printf("  pop r10       # \n");
            if (node->rhs->ty->ty == PTR) {
                printf("  pop rax #左辺の値を取り出して、stacksizeの大きさをかける\n");
                printf("  mov r11, %d\n", node->rhs->ty->ptr_to->size);//node->lhs->stacksize);
                printf("  mul r11\n");
                printf("  push rax\n");
            }
            if (node->lhs->ty->ty == PTR) {
                printf("  mov rax, r10\n");
                printf("  mov r11, %d\n", node->lhs->ty->ptr_to->size);//node->lhs->stacksize);
                printf("  mul r11\n");
                printf("  mov r10, rax\n");
            }
            printf("  pop rax\n");
            printf("  add rax, r10\n");
            printf("  push rax   #+の値をstackにつむ\n");
            return;
        case '-':
            printf("#gen_expr -の評価開始\n");
            gen_expr(node->lhs);
            gen_expr(node->rhs);
            printf("  pop r10\n");
            if (node->rhs->ty->ty == PTR) {
                printf("  pop rax #左辺の値を取り出して、stacksizeの大きさをかける\n");
                printf("  mov r11, %d\n", node->rhs->ty->ptr_to->size); //node->lhs->stacksize);
                printf("  mul r11\n");
                printf("  push rax\n");
            }
            if (node->lhs->ty->ty == PTR) {
                printf("  mov rax, r10\n");
                printf("  mov r11, %d\n", node->lhs->ty->ptr_to->size);
                printf("  mul r11\n");
                printf("  mov r10, rax\n");
            }
            printf("  pop rax\n");
            printf("  sub rax, r10\n");
            printf("  push rax   #-の値をstackにつむ\n");
            return;
        case ND_PREINC:
            printf("#gen_expr 前置++ の評価開始\n");
            gen_lval(node->lhs);
            if (node->lhs->ty->ty == PTR) {
                printf("  mov rax, 1\n");
                printf("  mov r11, %d\n", node->lhs->ty->ptr_to->size);
                printf("  mul r11\n");
                printf("  mov r10, rax\n");
            } else {
                printf("  mov r10, 1\n");
            }
            printf("  pop rax\n");
            load(node->lhs->ty, 1);
            printf("  add r11, r10\n");
            store(node->lhs->ty, 1);
            printf("  push r11\n");
            return;
        case ND_POSTINC:
            printf("# gen_expr 後置++の評価開始\n");
            gen_lval(node->lhs);
            if (node->lhs->ty->ty == PTR) {
                printf("  mov rax, 1\n");
                printf("  mov r11, %d\n", node->lhs->ty->ptr_to->size);
                printf("  mul r11\n");
                printf("  mov r10, rax\n");
            } else {
                printf("  mov r10, 1\n");
            }
            printf("  pop rax\n");
            load(node->lhs->ty, 1);
            printf("  push r11\n");
            printf("  add r11, r10\n");
            store(node->lhs->ty, 1);
            return;
        case ND_PREDEC:
            printf("#gen_expr 前置-- の評価開始\n");
            gen_lval(node->lhs);
            if (node->lhs->ty->ty == PTR) {
                printf("  mov rax, 1\n");
                printf("  mov r11, %d\n", node->lhs->ty->ptr_to->size);
                printf("  mul r11\n");
                printf("  mov r10, rax\n");
            } else {
                printf("  mov r10, 1\n");
            }
            printf("  pop rax\n");
            load(node->lhs->ty, 1);
            printf("  sub r11, r10\n");
            store(node->lhs->ty, 1);
            printf("  push r11\n");
            return;
        case ND_POSTDEC:
            printf("#gen_expr 前置-- の評価開始\n");
            gen_lval(node->lhs);
            if (node->lhs->ty->ty == PTR) {
                printf("  mov rax, 1\n");
                printf("  mov r11, %d\n", node->lhs->ty->ptr_to->size);
                printf("  mul r11\n");
                printf("  mov r10, rax\n");
            } else {
                printf("  mov r10, 1\n");
            }
            printf("  pop rax\n");
            load(node->lhs->ty, 1);
            printf("  push r11\n");
            printf("  sub r11, r10\n");
            store(node->lhs->ty, 1);
            return;
        case ND_NEG:
            printf("#gen_expr - の評価開始\n");
            gen_expr(node->lhs);
            printf("  pop rax\n");
            printf("  neg rax\n");
            printf("  push rax\n");
            return;
        case '~':
            printf("#gen_expr ~ の評価開始\n");
            gen_expr(node->lhs);
            printf("  pop rax\n");
            printf("  not rax\n");
            printf("  push rax\n");
            return;
        case '*':
            printf("#gen_expr *の評価開始\n");
            gen_binop(node->lhs, node->rhs);
            printf("  pop r10\n");
            printf("  pop rax\n");
            printf("  mul r10\n");
            printf("  push rax   #*の値をstackにつむ\n");
            return;
        case '/':
            divmod(node, "rax");
            return;
        case '%':
            divmod(node, "rdx");
            return;
        case '!':
            printf("#gen_expr !の評価開始\n");
            gen_expr(node->lhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  sete al   # al(raxの下位8ビットを指す別名レジスタ)にcmpの結果(同じなら1/それ以外なら0)をセット\n");          // al(raxの下位8ビットを指す別名レジスタ)にcmpの結果(同じなら1/それ以外なら0)をセット
            printf("  movzb eax, al\n");
            printf("  push rax# !した値をつむ\n");
            return;
        case ND_CAST: {
            if (node->ty->ty == VOID) {
                fprintf(stderr, "cast to void\n");
                return;
            }
            gen_expr(node->lhs);
            printf("  pop rax\n");
            if (node->ty->ty == BOOL) {
                printf("  mov r10, 0\n");
                printf("  cmp rax, r10 # 左辺と右辺が同じかどうかを比較する\n");     // 2つのレジスタの値が同じかどうか比較する
                printf("  setne al  # != \n");
                printf("  movzx rax, al # raxを0クリアしてからalの結果をraxに格納\n");    // raxを0クリアしてからalの結果をraxに格納
            } else if (node->ty->size == 1) {
                printf("  movsx rax, al # raxを0クリアしてからalの結果をraxに格納\n");
            } else if (node->ty->size == 2) {
                printf("  movsx rax, ax\n");
            } else if (node->ty->size == 4) {
                printf("  movsx rax, eax\n");
            }
            printf("  push rax      # スタックに結果を積む\n");         // スタックに結果を積む
            return;
        }
        case ND_STMT_EXPR:
            gen_stmt(node->lhs);
            printf("  sub rsp, 8#stmtをコンパイルして最後に値が入ってるはず\n");
            return;
        case '?':
            seq = nlabel++;
            gen_expr(node->cond);
            printf("  pop rax      # condの結果をraxにコピー\n");                      // lhsの結果をraxにコピー
            printf("  cmp rax, 0   # cond結果と0を比較する(cond式の中身がfalseのときは1)\n");                   // raxの結果と0を比較
            printf("  je .Lelse%d   # 0なら.Lelse%dに飛ぶ\n", seq, seq);      // lhsが0のとき（false) Lendに飛ぶ
            gen_expr(node->if_body);                             // rhsの結果をスタックにpush
            printf("jmp .Lend%d\n", seq);        // endにとぶ
            printf(".Lelse%d:\n", seq);          // else条件
            gen_expr(node->else_body);
            printf(".Lend%d:\n", seq);          // 終わる
            return;
        case ',':
            printf("# ',' gen_expr\n");
            gen_expr(node->lhs);
            printf("  pop rax #一つ目, の前\n");
            gen_expr(node->rhs);
            return;
    }
}

// 左辺値を計算する
void gen_lval(Node *node) {
    printf("#gen_lvalの評価開始\n");
    if (node->op == ND_DEREF)
        return gen_expr(node->lhs);
    if (node->op == ND_DOT) {
        gen_lval(node->lhs);
        printf("  pop rax\n");
        printf("  add rax, %d # rax %sのmemberのoffset:%d分だけ押し下げたアドレスが%sのmemberの変数のアドレス\n", node->offset, node->name, node->offset, node->name);
        printf("  push rax\n");
        return;
    }

    if (node->op != ND_LVAR && node->op != ND_GVAR && node->op != ND_VARDEF) {
        fprintf(stderr, "node->op: %d\n", node->op);
        fprintf(stderr, "代入の左辺値が変数ではありません");
        exit(1);
    }

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

        if (node->ty->size == 4) {
            printf("  mov [rax], %s # raxのレジスタのアドレスに呼び出し側で設定したレジスタの中身をストア\n", argreg32[i]);
        } else if (node->ty->size == 8) {
            printf("  mov [rax], %s # raxのレジスタのアドレスに呼び出し側で設定したレジスタの中身をストア\n", argreg64[i]);
        } else if (node->ty->size == 2) {
            printf("  mov [rax], %s # raxのレジスタのアドレスに呼び出し側で設定したレジスタの中身をストア\n", argreg16[i]);
        } else if (node->ty->size == 1) {
            printf("  mov [rax], %s # raxのレジスタのアドレスに呼び出し側で設定したレジスタの中身をストア\n", argreg8[i]);
        } else {
            printf("  mov [rax], %s # raxのレジスタのアドレスに呼び出し側で設定したレジスタの中身をストア\n", argreg64[i]);   // raxのレジスタのアドレスに呼び出し側で設定したレジスタの中身をストア
        }
    }
}

void gen(Node *node);
void gen_stmt(Node *node) {
    if (node->op == ND_VARDEF) {
        printf("#gen_stmt ND_VARDEFの処理がここはいる\n");
        if (!node->init)
            return;
        gen_stmt(node->init);
        return;
    }

    // if(lhs) rhsをコンパイル
    if (node->op == ND_IF) {
        printf("#gen_stmt IFの処理\n");
        int seq = nlabel++;
        gen_expr(node->cond);                             // lhsの結果をスタックにpush
        // if_node->else_bodyがあるとき
        if (node->else_body) {
            printf("  pop rax      # condの結果をraxにコピー\n");                      // lhsの結果をraxにコピー
            printf("  cmp rax, 0   # cond結果と0を比較する(if文の中身がfalseのときは1)\n");                   // raxの結果と0を比較
            printf("  je .Lelse%d   # 0なら.Lelse%dに飛ぶ\n", seq, seq);      // lhsが0のとき（false) Lendに飛ぶ
            gen_stmt(node->if_body);                             // rhsの結果をスタックにpush
            printf("jmp .Lend%d\n", seq);        // endにとぶ
            printf(".Lelse%d:\n", seq);          // else条件
            gen_stmt(node->else_body);
            printf(".Lend%d:\n", seq);          // 終わる
        } else {
            printf("  pop rax      # lhsの結果をraxにコピー\n");                      // lhsの結果をraxにコピー
            printf("  cmp rax, 0   # raxの結果と0を比較する(if文の中身がfalseのときは1)\n");                   // raxの結果と0を比較
            printf("  je .Lend%d   # 0なら.Lend%dに飛ぶ\n", seq, seq);      // lhsが0のとき（false) Lendに飛ぶ
            gen_stmt(node->if_body);                             // rhsの結果をスタックにpush
            printf(".Lend%d:\n", seq);          // 終わる
        }
        return;
    }

    // for(lhs, lhs2, lhs3) rhsをコンパイル
    if (node->op == ND_FOR) {
        printf("#gen_stmt FORの処理\n");
        gen_stmt(node->lhs);                     // lhsをまず実行してスタックに積む
        int seq = nlabel++;
        printf(".Lbegin%d:      # ループの開始\n", seq);   // ループの開始
        if (node->lhs2) {
            gen_expr(node->lhs2);                    // lhs2の実行結果をスタックに積む
            printf("  pop rax\n");
            printf("  cmp rax, 0    # 0と等しい(二つ目がfalseになったら)終わる\n");           // lhsの実行結果が0と等しい。falseになったらおわる
            printf("  je .Lend%d\n", seq);
        }
        printf("# for文の内部を処理\n");
        gen_stmt(node->rhs);                     // rhsを実行
        printf(".Lend%d: # continue\n", node->continue_label);
        if (node->lhs3) {
            printf("# forの３つ目の領域の処理実行\n");
            gen_expr(node->lhs3);                    // lhs3の実行結果をスタックに積む
            printf("  pop rax # forの三つ目の結果は捨てる\n");
        }
        printf("  jmp .Lbegin%d # ループの開始に飛ぶ\n", seq);// ループの開始に戻る

        printf(".Lend%d:        # for文終わり\n", seq);
        printf(".Lend%d:        # break \n", node->break_label);
        return;
    }

    if (node->op == ND_DO_WHILE) {
        int seq = nlabel++;
        printf(".Lbegin%d: #ループの開始地点\n", seq);
        printf(".Lend%d: #continue の開始地点\n", node->continue_label);
        gen_stmt(node->body);
        gen_expr(node->cond);
        printf("  pop rax # do whileの評価結果(0 or 1) \n");
        printf("  cmp rax, 0 #\n");
        printf("  jne .Lbegin%d\n", seq);
        printf(".Lend%d:\n", node->break_label);
        return;
    }

    // while(lhs) rhsをコンパイル
    if (node->op == ND_WHILE) {
        printf("#gen_stmt WHILEの処理\n");
        int seq = nlabel++;
        printf(".Lbegin%d: # ループの開始\n", seq);      // ループの開始
        printf(".Lend%d: # continueの開始\n", node->continue_label);
        gen_expr(node->lhs);                         // lhsをコンパイルしてスタックにpush
        printf("  pop rax      # while評価の結果を格納(0 or 1) \n");                  // raxにstackを格納
        printf("  cmp rax, 0   # while評価が0ならば終わり\n");               // rhsの結果が0のとき(falseになったら) Lendに飛ぶ
        printf("  je .Lend%d\n", seq);
        printf("#WHILEの中身の処理\n");
        gen_stmt(node->rhs);                         // ループの中身をコンパイル
        printf("  jmp .Lbegin%d\n", seq);  // ループの開始時点に戻る
        printf(".Lend%d:\n", seq);
        printf(".Lend%d:\n", node->break_label);
        return;
    }

    if (node->op == ND_SWITCH) {
        printf("#gen_stmt switchの処理\n");
        gen_expr(node->cond);
        printf("  pop rax # switchのconditionの内容をraxにロード\n");
        Node *default_ = NULL; // if exists
        for (int i = 0; i < node->cases->len; i++) {
            Node *case_ = node->cases->data[i];
            if (case_->op == ND_CASE) {
                printf("  mov r10, %ld # case[%d]のconst値をr10にロード\n", case_->val, i);
                printf("  cmp rax, r10\n");
                printf("  je .Lcase%d\n", case_->case_label);
            } else if (case_->op == ND_DEFAULT) {
                default_ = case_;
            } else {
                fprintf(stderr, "case is not default or case\n");
                exit(1);
            }
        }
        // 存在する時は上記で引っかからなかったものをdefaultにとばす
        if (default_)
            printf("  jmp .Lcase%d\n", default_->case_label);

        printf("  jmp .Lend%d\n", node->break_label);
        gen_stmt(node->body);
        printf(".Lend%d:\n", node->break_label);
        return;
    }

    if (node->op == ND_CASE || node->op == ND_DEFAULT) {
        printf(".Lcase%d:\n", node->case_label);
        gen_stmt(node->body);
        return;
    }

    if (node->op == ND_RETURN) {
        printf("#en_stmt ND_RETURNの処理\n");
        if (node->lhs) {
            gen_expr(node->lhs);
            printf("  pop rax #returnしたい結果がスタックに入っているのでをraxにロード\n");          // genで生成された値をraxにpopして格納
        }

        //関数のエピローグ
        printf("# return したのでエピローグ\n");
        printf("  mov rsp, rbp # ベースポインタをrspに格納\n");     // ベースポインタをrspにコピーして
        printf("  pop rbp      # スタックには呼び出し元のrbpが入ってるはずなのでrbpにロードする\n");          // スタックの値をrbpに持ってくる
        printf("  ret          # returnする\n");
        return;
    }

    if (node->op == ND_BREAK) {
        printf("#gen_stmt ND_BREAKの処理\n");
        printf("  jmp .Lend%d # breakしたので、ループを抜ける\n", node->target->break_label);
        return;
    }

    if (node->op == ND_CONTINUE) {
        printf("#gen_stmt ND_CONTINUEの処理\n");
        printf("  jmp .Lend%d # contiueしたので、ループの開始に戻る\n", node->target->continue_label);
        return;
    }

    if (node->op == ND_EXPR_STMT) {
        printf("#gen_stmt ND_EXPR_STMTの処理\n");
        gen_expr(node->lhs);
        printf("  pop rax # ND_EXPR_STMTなので、expressionからpopしておく\n");
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
            if(!node->is_static)
                printf(".globl %s\n", node->name);
            printf("%s:\n", node->name);
            // プロローグ
            // 使用した変数分の領域を確保する
            printf("  push rbp     # 呼び出し元のrbpをスタックにつんでおく(エピローグでpopしてrspをそこに戻す)\n");                     // ベースポインタをスタックにプッシュする
            printf("  mov rbp, rsp # rspが今の関数のベースポインタを指しているのでrbpにコピーしておく\n");                 // rspをrbpにコピーする
            printf("  sub rsp, %d  # 今の関数で使用する変数(%d個)の数だけalignしてrspを動かす\n", roundup(node->stacksize, 16), node->args->len);   // rspを使用した変数分動かす
            printf("#関数の引数の処理\n");
            gen_args(node->args);
            printf("#関数本体の処理\n");
            gen_stmt(node->body);
            // 抽象構文木を下りながらコード生成
            // 式の評価結果としてスタックに一つの値が残ってる
            // はずなので、スタックが溢れないようにポップしておく
            gen_epilog();
        } else if (node->op == ND_VARDEF) {
            continue;
        } else if (node->op == ND_DECL) {
            continue;
        } else {
            fprintf(stderr, "node->op must be ND_FUNC but got: %d", node->op);
            exit(1);
        }
    }
}
