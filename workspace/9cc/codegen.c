#include "9cc.h"

char* argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char* argreg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char* argreg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};

static char *escape(char *s, int len) {
    static char escaped[256] = {
        ['\b'] = 'b', ['\f'] = 'f', ['\n'] = 'n', ['\r'] = 'r',
        ['\t'] = 't', ['\\'] = '\\', ['\''] = '\'', ['"'] = '"',
    };
    char *buf = malloc(len * 4);
    char *p = buf;
    for (int i = 0; i < len; i++) {
        uint8_t c = s[i];
        char esc = escaped[c];
        if (esc) {
            *p++ = '\\';
            *p++ = esc;
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

int stack = 0;

void push(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    stack++;
    printf("#current: %d\n", stack);
}

void pop(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    printf(fmt, ap);
    stack--;
    printf("#current: %d\n", stack);
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
            printf("%s:\n", var->name);
            printf("  .ascii \"%s\"\n", escape(var->data, var->len));
        }
    }

    printf(".global main\n");
}

void gen_epilog() {
    // エピローグ
    printf("#gen_epilog\n");
    printf("  mov rsp, rbp # ベースポインタをrspにコピーして \n");     // ベースポインタをrspにコピーして
    pop("  pop rbp      # スタックの値をrbpに持ってくる\n");        // スタックの値をrbpに持ってくる
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
    int seq;
    switch(node->op) {
        case ND_NUM: {
            printf("#gen_expr ND_NUM \n");
            push("  push %d        # スタックに数字を積む\n", node->val);
            return;
        }
        // 変数に格納
        case '=': {
            // 左辺が変数であるはず
            gen_lval(node->lhs);            // ここでスタックのトップに変数のアドレスが入っている
            gen_expr(node->rhs);            // 右辺値が評価されてスタックのトップに入っている

            printf("#gen_expr =の処理開始\n");
            pop("  pop r10        # 評価された右辺値をr10にロード\n");          // 評価された右辺値がr10にロード
            pop("  pop rax        # 左辺の変数のアドレスがraxに格納\n");          // 変数のアドレスがraxに格納

            char *reg = "r10";
            if (node->lhs->ty->ty == INT)
                reg = "r10d";
            else if(node->lhs->ty->ty == CHAR)
                reg = "r10b";

            printf("  mov [rax], %s # raxのレジスタのアドレスにr10の値をストアする(この場合左辺のアドレスに右辺の評価値を書き込む) \n", reg);   // raxのレジスタのアドレスにr10の値をストアする
            push("  push r10       # 右辺値をスタックにプッシュする\n");         // r10の値をスタックにプッシュする
            return;
        }

        case ND_CALL: {
            printf("#gen_expr ND_CALL開始:\n");
            int args_len = node->args->len;
            for (int i = 0; i < args_len; i++) {
                printf("#先にND_NUMがここで処理されるのではない？\n");
                printf("#gen_expr ND_CALL(引数処理): %d番目の引数\n", i);
                gen_expr((Node *)  node->args->data[i]);         // スタックに引数を順に積む
            }
            for (int i = args_len - 1; i >= 0; i--) {
                pop("  pop rax      # スタックされた引数の評価値をスタックからraxに格納\n");                     // 結果をraxに格納
                printf("  mov %s, rax # raxには引数が積まれているので、各レジスタに値を格納\n", argreg[i]);        // raxから各レジスタに格納
            }
            seq = jump_num++;
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
            push("  push rax      #関数の結果をスタックに積む \n");         // スタックに結果を積む
            return;
        }

        case ND_GVAR:
        case ND_LVAR: {
            printf("#gen_expr ND_IDENTの処理開始\n");
            gen_lval(node);
            pop("  pop rax        # 左辺値がコンパイルされた結果をスタックからraxにロード\n");          // スタックからpopしてraxに格納
            char *reg = "rax";
            if (node->ty->ty == INT)
                reg = "eax";
            else if(node->ty->ty == CHAR) {
                push("  push rax\n");
                return;
            }
            printf("  mov %s, [rax] # raxをアドレスとして値をロードしてraxに格納(この場合左辺値のアドレスに格納された値がraxに入る)\n", reg);   // raxをアドレスとして値をロードしてraxに格納
            push("  push rax       # 結果をスタックに積む\n");         // スタックにraxをpush
            return;
        }

        case ND_DOT:
        case ND_DEREF: {
            printf("#gen_expr ND_DEREF/ND_DOTが右辺にきたときの処理開始\n");
            gen_lval(node);
            printf("#スタックに値を格納したいさきのアドレスが載ってる\n");
            pop("  pop rax        \n");          // スタックからpopしてraxに格納
            char *reg = "rax";
            if (node->ty->ty == INT)
                reg = "eax";
            else if(node->ty->ty == CHAR) {
                printf("  mov al, [rax] # デリファレンスのアドレスから値をロード\n");   // raxをアドレスとして値をロードしてraxに格納
                printf("  movzb rax, al\n");
                push("  push rax       # デリファレンス後の値の結果をスタックに積む\n");         // スタックにraxをpush
                return;
            }
            printf("  mov %s, [rax] # デリファレンスのアドレスから値をロード\n", reg);   // raxをアドレスとして値をロードしてraxに格納
            push("  push rax       # デリファレンス後の値の結果をスタックに積む\n");         // スタックにraxをpush
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

            pop("  pop r10      # 右辺をr10にpop\n");          // 左辺をr10にpop
            pop("  pop rax      # 左辺をraxにpop\n");          // 右辺をraxにpop
            printf("  cmp rax, r10 # 左辺と右辺が同じかどうかを比較する\n");     // 2つのレジスタの値が同じかどうか比較する

            if (node->op == ND_EQ)
                printf("  sete al   # al(raxの下位8ビットを指す別名レジスタ)にcmpの結果(同じなら1/それ以外なら0)をセット\n");          // al(raxの下位8ビットを指す別名レジスタ)にcmpの結果(同じなら1/それ以外なら0)をセット
            if (node->op == ND_NE)
                printf("  setne al  # != \n");
            if (node->op == '<' || node->op == '>')
                printf("  setl al   # < \n");
            if (node->op == ND_LE)
                printf("  setle al  # <= \n");
            printf("  movzb rax, al # raxを0クリアしてからalの結果をraxに格納\n");    // raxを0クリアしてからalの結果をraxに格納
            push("  push rax      # スタックに結果を積む\n");         // スタックに結果を積む
            return;
        }
        case ND_LOGOR:
            // 左辺と右辺の内いずれかが1なら1
            gen_expr(node->lhs);
            seq = jump_num++;
            pop("  pop r10\n");
            printf("  cmp r10, 1 # 1と等しければje..\n");
            printf("  je .Ltrue%d   # 0なら.Lend%dに飛ぶ\n", seq, seq);      // lhsが0のとき（false) Lendに飛ぶ
            gen_expr(node->rhs);
            pop(" pop r10\n");
            printf("  cmp r10, 1 # 1と等しければ..\n");
            printf("  je .Ltrue%d   # 0なら.Lend%dに飛ぶ\n", seq, seq);      // lhsが0のとき（false) Lendに飛ぶ
            push("  push 0\n"); // 両方0の時
            printf("  jmp .Lend%d\n", seq);
            printf(".Ltrue%d:\n", seq);
            push("  push 1\n");
            printf(".Lend%d:\n", seq);
            return;
        case ND_LOGAND:
            // 左辺と右辺の内いずれかが1なら1
            gen_expr(node->lhs);
            pop("  pop r10\n");
            printf("  cmp r10, 0 # 0と等しければje..\n");
            seq = jump_num++;
            printf("  je .Lfalse%d   # 0なら.Lend%dに飛ぶ\n", seq, seq);      // lhsが0のとき（false) Lendに飛ぶ
            gen_expr(node->rhs);
            pop("  pop r10\n");
            printf("  cmp r10, 0 # 0と等しければ..\n");
            printf("  je .Lfalse%d   # 0なら.Lend%dに飛ぶ\n", seq, seq);      // lhsが0のとき（false) Lendに飛ぶ
            push("  push 1\n"); // 両方0の時
            printf("  jmp .Lend%d\n", seq);
            printf(".Lfalse%d:\n", seq);
            push("  push 0\n");
            printf(".Lend%d:\n", seq);
            return;
        case '&':
            gen_binop(node->lhs, node->rhs);
            pop("  pop r10 \n");
            pop("  pop rax \n");
            printf("  and rax, r10 # & を計算 raxに結果が格納されている\n");
            push("  push rax \n");
            return;
        case '^':
            gen_binop(node->lhs, node->rhs);
            pop("  pop r10 \n");
            pop("  pop rax \n");
            printf("  xor rax, r10 # ^ を計算 raxに結果が格納されている\n");
            push("  push rax \n");
            return;
        case '|':
            gen_binop(node->lhs, node->rhs);
            pop("  pop r10 \n");
            pop("  pop rax \n");
            printf("  or rax, r10 # | を計算 raxに結果が格納されている\n");
            push("  push rax \n");
            return;
        case ND_LSHIFT:
            gen_binop(node->lhs, node->rhs);
            pop("  pop rcx \n");
            pop("  pop rax \n");
            printf("  shl rax, cl\n");
            push("  push rax\n");
            return;
        case ND_RSHIFT:
            gen_binop(node->lhs, node->rhs);
            pop("  pop rcx \n");
            pop("  pop rax \n");
            printf("  shr rax, cl\n");
            push("  push rax\n");
            return;
        case '+':
            printf("#gen_expr +の評価開始\n");
            gen_expr(node->lhs);
            gen_expr(node->rhs);
            pop("  pop r10       # \n");
            if (node->rhs->ty->ty == PTR) {
                pop("  pop rax #左辺の値を取り出して、stacksizeの大きさをかける\n");
                printf("  mov r11, %d\n", node->rhs->ty->ptr_to->size);//node->lhs->stacksize);
                printf("  mul r11\n");
                push("  push rax\n");
            }
            if (node->lhs->ty->ty == PTR) {
                printf("  mov rax, r10\n");
                printf("  mov r11, %d\n", node->lhs->ty->ptr_to->size);//node->lhs->stacksize);
                printf("  mul r11\n");
                printf("  mov r10, rax\n");
            }
            pop("  pop rax\n");
            printf("  add rax, r10\n");
            push("  push rax   #+の値をstackにつむ\n");
            return;
        case '-':
            printf("#gen_expr -の評価開始\n");
            gen_expr(node->lhs);
            gen_expr(node->rhs);
            pop("  pop r10\n");
            if (node->rhs->ty->ty == PTR) {
                pop("  pop rax #左辺の値を取り出して、stacksizeの大きさをかける\n");
                printf("  mov r11, %d\n", node->rhs->ty->ptr_to->size); //node->lhs->stacksize);
                printf("  mul r11\n");
                push("  push rax\n");
            }
            if (node->lhs->ty->ty == PTR) {
                printf("  mov rax, r10\n");
                printf("  mov r11, %d\n", node->lhs->ty->ptr_to->size);
                printf("  mul r11\n");
                printf("  mov r10, rax\n");
            }
            pop("  pop rax\n");
            printf("  sub rax, r10\n");
            push("  push rax   #-の値をstackにつむ\n");
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
            pop("  pop rax\n");
            printf("  mov r11, [rax]\n");
            printf("  add r11, r10\n");
            printf("  mov [rax], r11\n");
            push("  push r11\n");
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
            pop("  pop rax\n");
            printf("  mov r11, [rax]\n");
            printf("  sub r11, r10\n");
            printf("  mov [rax], r11\n");
            push("  push r11\n");
            return;
        case ND_NEG:
            printf("#gen_expr - の評価開始\n");
            gen_expr(node->lhs);
            pop("  pop rax\n");
            printf("  mov r10, -1\n");
            push("  mul r10\n");
            push("  push rax\n");
            return;
        case '*':
            printf("#gen_expr *の評価開始\n");
            gen_binop(node->lhs, node->rhs);
            pop("  pop r10\n");
            pop("  pop rax\n");
            printf("  mul r10\n");
            push("  push rax   #*の値をstackにつむ\n");
            return;
        case '/':
            printf("#gen_expr /の評価開始\n");
            gen_binop(node->lhs, node->rhs);
            pop("  pop r10\n");
            pop("  pop rax\n");
            printf("  cqo\n");
            printf("  idiv r10\n");
            push("  push rax   #/の値をstackにつむ\n");
            return;
        case '%':
            gen_binop(node->lhs, node->rhs);
            pop("  pop r10\n");
            pop("  pop rax\n");
            printf("  cqo\n");
            printf("  idiv r10\n");
            push("  push rdx # %の値をstackにつむ\n");
            return;
        case '!':
            printf("#gen_expr !の評価開始\n");
            gen_expr(node->lhs);
            pop("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  sete al   # al(raxの下位8ビットを指す別名レジスタ)にcmpの結果(同じなら1/それ以外なら0)をセット\n");          // al(raxの下位8ビットを指す別名レジスタ)にcmpの結果(同じなら1/それ以外なら0)をセット
            push("  push rax\n # !した値をつむ");
            return;
        case ND_STMT_EXPR:
            gen_stmt(node->lhs);
            push("  sub rsp, 8#stmtをコンパイルして最後に値が入ってるはず\n");
            return;
        case '?':
            seq = jump_num++;
            gen_expr(node->cond);
            pop("  pop rax      # condの結果をraxにコピー\n");                      // lhsの結果をraxにコピー
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
            pop("  pop rax #一つ目, の前\n");
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
        printf("#gen_lvalとしてND_DOTを処理する\n");
        gen_lval(node->lhs);
        printf("#structの先頭のアドレスがraxに入ってる?\n");
        pop("  pop rax\n");
        printf("  add rax, %d # rax %sのmemberのoffset:%d分だけ押し下げたアドレスが%sのmemberの変数のアドレス\n", node->offset, node->name, node->offset, node->name);
        push("  push rax\n");
        return;
    }

    if (node->op != ND_LVAR && node->op != ND_GVAR && node->op != ND_VARDEF)
        error("代入の左辺値が変数ではありません", 0);

    if (node->op == ND_LVAR || node->op == ND_VARDEF){
        printf("  mov rax, rbp # 関数のベースポインタをraxにコピー\n");         // ベースポインタをraxにコピー
        printf("  sub rax, %d   # raxを%sのoffset:%d分だけ押し下げたアドレスが%sの変数のアドレス。それをraxに保存)\n", node->offset, node->name, node->offset, node->name);  // raxをoffset文だけ押し下げ（nameの変数のアドレスをraxに保存)
        push("  push rax      # 結果をスタックに積む(変数のアドレスがスタック格納されてる) \n");             // raxをスタックにプッシュ
        return;
    }

    assert(node->op == ND_GVAR);
    printf("  lea rax, %s\n", node->name);
    push("  push rax\n");
    return;
}

// 関数のプロローグ
// args: 関数の引数
void gen_args(Vector *args) {
    int args_len = args->len;                   // argsのlengthを取得
    for(int i = 0; i < args_len; i++) {
        Node *node = args->data[i];
        gen_lval(node);       // 関数の引数定義はlvalとして定義
        pop("  pop rax        # 第%d引数の変数のアドレスがraxに格納\n", i);          // 変数のアドレスがraxに格納

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
        push("  push rax      # 結果をスタックに積む(変数のアドレスがスタック格納されてる) \n");             // raxをスタックにプッシュ
        pop("  pop rax       # 代入すべきアドレスがスタックされている\n");
        pop("  pop r10       # 宣言時の右辺の値が入っている\n");

        char *reg = "r10";
        if (node->ty->ty == INT)
            reg = "r10d";
        else if(node->ty->ty == CHAR)
            reg = "r10b";
        printf("  mov [rax], %s\n", reg);
        return;
    }

    // if(lhs) rhsをコンパイル
    if (node->op == ND_IF) {
        printf("#gen_stmt IFの処理\n");
        int seq = jump_num++;
        gen_expr(node->cond);                             // lhsの結果をスタックにpush
        // if_node->else_bodyがあるとき
        if (node->else_body) {
            pop("  pop rax      # condの結果をraxにコピー\n");                      // lhsの結果をraxにコピー
            printf("  cmp rax, 0   # cond結果と0を比較する(if文の中身がfalseのときは1)\n");                   // raxの結果と0を比較
            printf("  je .Lelse%d   # 0なら.Lelse%dに飛ぶ\n", seq, seq);      // lhsが0のとき（false) Lendに飛ぶ
            gen_stmt(node->if_body);                             // rhsの結果をスタックにpush
            printf("jmp .Lend%d\n", seq);        // endにとぶ
            printf(".Lelse%d:\n", seq);          // else条件
            gen_stmt(node->else_body);
            printf(".Lend%d:\n", seq);          // 終わる
        } else {
            pop("  pop rax      # lhsの結果をraxにコピー\n");                      // lhsの結果をraxにコピー
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
        gen_expr(node->lhs);                     // lhsをまず実行してスタックに積む
        pop("  pop rax # forの第一文の条件式はこのあと使わないので捨てる\n");
        int seq = jump_num++;
        printf(".Lbegin%d:      # ループの開始\n", seq);   // ループの開始
        gen_expr(node->lhs2);                    // lhs2の実行結果をスタックに積む
        pop("  pop rax       # 二つ目の実行結果をraxに格納\n");              // lhs2の実行結果をraxに格納
        printf("  cmp rax, 0    # 0と等しい(二つ目がfalseになったら)終わる\n");           // lhsの実行結果が0と等しい。falseになったらおわる
        printf("  je .Lend%d\n", seq);
        printf("# for文の内部を処理\n");
        gen_stmt(node->rhs);                     // rhsを実行
        printf("# forの３つ目の領域の処理実行\n");
        gen_expr(node->lhs3);                    // lhs3の実行結果をスタックに積む
        pop("  pop rax # forの三つ目の結果は捨てる\n");
        printf("  jmp .Lbegin%d # ループの開始に飛ぶ\n", seq);// ループの開始に戻る
        printf(".Lend%d:        # for文終わり\n", seq);
        return;
    }

    // while(lhs) rhsをコンパイル
    if (node->op == ND_WHILE) {
        printf("#gen_stmt WHILEの処理\n");
        int seq = jump_num++;
        printf("  .Lbegin%d: # ループの開始\n", seq);      // ループの開始
        gen_expr(node->lhs);                         // lhsをコンパイルしてスタックにpush
        pop("  pop rax      # while評価の結果を格納(0 or 1) \n");                  // raxにstackを格納
        printf("  cmp rax, 0   # while評価が0ならば終わり\n");               // rhsの結果が0のとき(falseになったら) Lendに飛ぶ
        printf("  je .Lend%d\n", seq);
        printf("#WHILEの中身の処理\n");
        gen_stmt(node->rhs);                         // ループの中身をコンパイル
        printf("  jmp .Lbegin%d\n", seq);  // ループの開始時点に戻る
        printf(".Lend%d:\n", seq);
        return;
    }

    if (node->op == ND_RETURN) {
        printf("#gen_stmt ND_RETURNの処理\n");
        gen_expr(node->lhs);
        pop("  pop rax #returnしたい結果がスタックに入っているのでをraxにロード\n");          // genで生成された値をraxにpopして格納

        //関数のエピローグ
        printf("# return したのでエピローグ\n");
        printf("  mov rsp, rbp # ベースポインタをrspに格納\n");     // ベースポインタをrspにコピーして
        pop("  pop rbp      # スタックには呼び出し元のrbpが入ってるはずなのでrbpにロードする\n");          // スタックの値をrbpに持ってくる
        printf("  ret          # returnする\n");
        return;
    }

    if (node->op == ND_EXPR_STMT) {
        printf("#gen_stmt ND_EXPR_STMTの処理\n");
        gen_expr(node->lhs);
        pop("  pop rax # ND_EXPR_STMTなので、expressionからpopしておく\n");
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
            push("  push rbp     # 呼び出し元のrbpをスタックにつんでおく(エピローグでpopしてrspをそこに戻す)\n");                     // ベースポインタをスタックにプッシュする
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
        } else {
            fprintf(stderr, "node->op must be ND_FUNC but got: %d", node->op);
            exit(1);
        }
    }
}
